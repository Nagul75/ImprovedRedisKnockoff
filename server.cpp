//
// Created by regal on 9/22/25.
//

#include "Socket.h"

bool handle_one_request(const Socket& client_socket)
{
    try
    {
        // read the first 4 header bytes
        uint32_t len;
        client_socket.read_full(&len, sizeof(len));

        if (len > Socket::K_MAX_MSG)
        {
            std::cerr << "Message is too long! \n";
            return false; // terminate connection
        }
        std::vector<char>body_buf(len);
        client_socket.read_full(body_buf.data(), len);

        std::cout << "Client says: " << std::string(body_buf.begin(), body_buf.end()) << '\n';

        // response
        const std::string response{"Moshi Moshi!!! This is the server ^^! How are you?!"};
        const uint32_t reply_length{static_cast<uint32_t>(response.length())};

        client_socket.write_all(&reply_length, sizeof(reply_length));
        client_socket.write_all(response.c_str(), response.length());

        return true;
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << "Connection error: " << e.what() << '\n';
        return false;
    }

}

void handleClient(Socket client_socket)
{
    std::cout << "Handling new client connection ... \n";
    while (handle_one_request(client_socket))
    {
        // continue serving requests
    }

    std::cout << "Closing client! \n";
}


int main()
{
    try
    {
        const Socket listening_socket{};
        listening_socket.set_reuse_addr();
        listening_socket.bind_to(1234);
        listening_socket.listen_for();

        std::cout << "Server Listening on port 1234 ..." << '\n';

        while (true)
        {
            try
            {
                Socket client_socket{listening_socket.accept_connection()};
                handleClient(std::move(client_socket));
            } catch (const std::runtime_error& e)
            {
                std::cerr << "Error accepting connection: " <<  e.what() << '\n';
            }
        }
    }catch (const std::runtime_error& e)
    {
        std::cerr << "Fatal server error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}