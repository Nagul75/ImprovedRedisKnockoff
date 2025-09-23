//
// Created by regal on 9/22/25.
//

#include "Socket.h"
#include <iostream>

bool handle_one_request(const Socket& client_socket, const std::string& message)
{
    try
    {
        // WRITE OPERATION
        const uint32_t message_length{static_cast<uint32_t>(message.length())};
        client_socket.write_all(&message_length, sizeof(message_length)); // write the length of the message first (header)
        client_socket.write_all(message.c_str(), message.length()); // write the actual message now
        std::cout << "Message sent \n";

        // READ OPERATION
        // read the first 4 header bytes
        uint32_t len;
        client_socket.read_full(&len, sizeof(len));
        // read the message based on the length first read
        std::vector<char> buffer(len);
        client_socket.read_full(buffer.data(), len);
        std::cout << "Server says: " << std::string(buffer.begin(), buffer.end()) << '\n';

        return true;
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << "Connection error: " << e.what() << '\n';
        return false;
    }
}

int main()
{
    try
    {
        Socket client_socket{};
        std::cout << "Connecting to server ... \n";

        client_socket.connect_to("127.0.0.1", 1234);

        std::cout << "Connected successfully! \n";

        if (!handle_one_request(client_socket, "First message"))
        {
            std::cout << "Exchange unsuccessful! \n";
            return 1;
        }
        std::cout << "Exchange successful! \n";
        if (!handle_one_request(client_socket, "Second message"))
        {
            std::cout << "Exchange unsuccessful! \n";
            return 1;
        }
        std::cout << "Exchange successful! \n";
        if (!handle_one_request(client_socket, "Second message"))
        {
            std::cout << "Exchange unsuccessful! \n";
            return 1;
        }
        std::cout << "Exchange successful! \n";
        return 0;
    }
    catch (const std::runtime_error& e)
    {
        // This will now catch connection errors too
        std::cerr << "Client error: " << e.what() << '\n';
        return 1;
    }
}
