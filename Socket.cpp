//
// Created by regal on 9/22/25.
//

#include "Socket.h"

#include <cassert>
#include <cstring>      // For strerror
#include <cerrno>       // For errno

Socket::Socket()
{
    m_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_fd < 0)
    {
        throw std::runtime_error("Socket() failed: " + std::string(strerror(errno)));
    }
}

Socket::Socket(const int fd)
    :m_fd(fd)
{
}

Socket::~Socket()
{
    if (m_fd > 0)
        close(m_fd);
}

Socket::Socket(Socket&& other) noexcept
    :m_fd(other.m_fd)
{
    other.m_fd = -1;
}

Socket& Socket::operator=(Socket&& other) noexcept
{
    if (this != &other)
    {
        if (m_fd > 0)
        {
            close(m_fd);
        }
        m_fd = other.m_fd;
        other.m_fd = -1;
    }
    return *this;
}

void Socket::set_reuse_addr()
{
    const int val{1};
    if (setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0)
    {
        throw std::runtime_error("setsockopt(SO_REUSEADDR) failed: " + std::string(strerror(errno)));
    }
}

void Socket::bind_to(const uint16_t port)
{
    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(m_fd, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)) < 0)
    {
        throw std::runtime_error("bind() failed: " + std::string(strerror(errno)));
    }
}

void Socket::listen_for(const int backlog)
{
    if (listen(m_fd, backlog) < 0)
    {
        throw std::runtime_error("listen() failed: " + std::string(strerror(errno)));
    }
}

[[nodiscard]] Socket Socket::accept_connection()
{
    const int connfd{accept(m_fd, nullptr, nullptr)};
    if (connfd < 0)
    {
        throw std::runtime_error("accept() failed: " + std::string(strerror(errno)));
    }
    return Socket(connfd);
}

void Socket::connect_to(const std::string& ip_address,const uint16_t port)
{
    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    // convert ip address from string to binary
    if (inet_pton(AF_INET, ip_address.c_str(), &addr.sin_addr) <= 0)
    {
        throw std::runtime_error("Invalid address / address not supported \n");
    }

    if (connect(m_fd, reinterpret_cast<const struct sockaddr *>(&addr), sizeof(addr)) < 0)
    {
        throw std::runtime_error("connect() failed: " + std::string(strerror(errno)));
    }
}

//buf is an out parameter
ssize_t Socket::read_full(std::vector<char>& buf)
{
    const ssize_t n{read(m_fd, buf.data(), buf.size())};
    if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 0; // Return 0 to indicate no data was read, but connection is fine.
        }
        throw std::runtime_error("read() error: " + std::string(strerror(errno)));
    }
    return n;
}

ssize_t Socket::write_all(const std::vector<char>& buf)
{
    const ssize_t n{write(m_fd, buf.data(), buf.size())};
    if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 0; // Return 0 to indicate no data was read, but connection is fine.
        }
        throw std::runtime_error("read() error: " + std::string(strerror(errno)));
    }
    return n;
}










