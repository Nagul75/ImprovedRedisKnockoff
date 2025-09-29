//
// Created by regal on 9/22/25.
//

#ifndef IMPROVEDREDISKNOCKOFF_SOCKET_H
#define IMPROVEDREDISKNOCKOFF_SOCKET_H

#include <cstdint>
#include <iostream>
#include <vector>

// C headers for socket programming
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
namespace NW
{
    class Socket
    {
    public:
        static constexpr size_t K_MAX_MSG = 4096;

        Socket();
        explicit Socket(int);
        ~Socket();

        // Disable copying
        Socket(const Socket&) = delete;
        Socket& operator=(const Socket&) = delete;

        // Enable moving
        Socket(Socket&&) noexcept;
        Socket& operator=(Socket&& other) noexcept;

        [[nodiscard]] int get_fd() const {return m_fd;}

        void set_non_blocking();
        void set_reuse_addr();
        void bind_to(uint16_t);
        void listen_for(int backlog = SOMAXCONN);
        [[nodiscard]] Socket accept_connection();
        void connect_to(const std::string& ip_address, uint16_t port);

        ssize_t read_data(std::vector<char>& buf);
        ssize_t write_data(const std::vector<char>& buf);

    private:
        int m_fd{-1};
    };
}


#endif //IMPROVEDREDISKNOCKOFF_SOCKET_H