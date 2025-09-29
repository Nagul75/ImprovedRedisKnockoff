#include "Socket.h"
#include <cstring>

void send_request(NW::Socket& socket, const std::string& text)
{
    auto len{static_cast<uint32_t>(text.length())};
    if (len > NW::Socket::K_MAX_MSG)
    {
        throw std::runtime_error("Message is too long! \n");
    }

    socket.write_all(&len, sizeof(len));
    socket.write_all(text.data(), text.size());
}

std::string read_response(NW::Socket& socket)
{
    uint32_t len{};
    socket.read_all(&len, sizeof(len));
    if (len > NW::Socket::K_MAX_MSG)
    {
        throw std::runtime_error("Response is too long! \n");
    }
    std::vector<char> response_buf(len);
    socket.read_all(response_buf.data(), len);

    return std::string(response_buf.begin(), response_buf.end());
}

int main()
{
    try
    {
        NW::Socket client{};
        std::cout << "Connecting to port 8000 ... \n";
        client.connect_to("127.0.0.1", 8000);
        std::cout << "Connected! \n";
        std::vector<std::string> query_list = {
            "hello",
            "world",
            "this is a test"
        };
        std::cout << "Sending " << query_list.size() << " messages...\n";
        for (const std::string& query : query_list) {
            send_request(client, query);
        }

        std::cout << "Reading responses...\n";
        for (size_t i = 0; i < query_list.size(); ++i) {
            std::string response = read_response(client);
            std::cout << "  Server replied: \"" << response << "\"\n";
        }

    }
    catch (std::runtime_error& e)
    {
        std::cout << "Client error: " << e.what() << '\n';
        return 1;
    }
}