//
// Created by regal on 9/22/25.
//

#include "Socket.h"
#include <iostream>

int main()
{
    try
    {
        Socket client_socket{};
        std::cout << "Connecting to server ... \n";
        client_socket.connect_to("127.0.0.1", 1234);
        std::cout << "Connected successfully! \n";

        const std::string message{"Heya, this is client! ^^ How are ya doing?!"};
        client_socket.write_data(message);
        std::cout << "Message sent \n";

        std::vector<char> buffer(1024);
        ssize_t bytes_read{client_socket.read_data(buffer)};

        std::cout << "Server says: " << std::string(buffer.data(), bytes_read) << '\n';

    }catch (const std::runtime_error& e)
    {
        std::cerr << "Client error: "  << e.what() << '\n';
    }
}
