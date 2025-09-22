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

class Socket
{
public:
    Socket();
    explicit Socket(int);
    ~Socket();

    // Disable copying
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    // Enable moving
    Socket(Socket&&) noexcept;
    Socket& operator=(Socket&& other) noexcept;

    void set_reuse_addr() const;
    void bind_to(uint16_t) const;
    void listen_for(int backlog = SOMAXCONN) const;
    [[nodiscard]] Socket accept_connection() const;
    void connect_to(const std::string& ip_address, uint16_t port) const;

    ssize_t read_data(std::vector<char>& buffer) const;
    void write_data(const std::string& message) const;

private:
    int m_fd{-1};
};


#endif //IMPROVEDREDISKNOCKOFF_SOCKET_H