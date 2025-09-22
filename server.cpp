//
// Created by regal on 9/22/25.
//

#include "Socket.h"

void handleClient(Socket client_socket)
{
    std::cout << "Handling new client connection. \n";
    std::vector<char> buffer(1024);
    ssize_t bytes_read{client_socket.read_data(buffer)};
    if (bytes_read > 0)
    {
        std::cout << "Client says: " << std::string(buffer.data(), bytes_read) << '\n';

        const std::string response {"Response from server hello!!!!!! :)"};
        client_socket.write_data(response);
    }
    std::cout << "Closing client connection. \n";
}

int main()
{
    try
    {
        Socket listening_socket{};
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