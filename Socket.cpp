//
// Created by regal on 9/22/25.
//

#include "Socket.h"

#include <cassert>
#include <cstring>      // For strerror
#include <cerrno>       // For errno
#include <fcntl.h>

NW::Socket::Socket()
{
    m_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_fd < 0)
    {
        throw std::runtime_error("Socket() failed: " + std::string(strerror(errno)));
    }
}

NW::Socket::Socket(const int fd)
    :m_fd(fd)
{
}

NW::Socket::~Socket()
{
    if (m_fd > 0)
        close(m_fd);
}

NW::Socket::Socket(NW::Socket&& other) noexcept
    :m_fd(other.m_fd)
{
    other.m_fd = -1;
}

NW::Socket& NW::Socket::operator=(Socket&& other) noexcept
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

void NW::Socket::set_reuse_addr()
{
    const int val{1};
    if (setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0)
    {
        throw std::runtime_error("setsockopt(SO_REUSEADDR) failed: " + std::string(strerror(errno)));
    }
}

void NW::Socket::bind_to(const uint16_t port)
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

void NW::Socket::listen_for(const int backlog)
{
    if (listen(m_fd, backlog) < 0)
    {
        throw std::runtime_error("listen() failed: " + std::string(strerror(errno)));
    }
}

[[nodiscard]] NW::Socket NW::Socket::accept_connection()
{
    const int connfd{accept(m_fd, nullptr, nullptr)};
    if (connfd < 0)
    {
        throw std::runtime_error("accept() failed: " + std::string(strerror(errno)));
    }
    return Socket(connfd);
}

void NW::Socket::connect_to(const std::string& ip_address,const uint16_t port)
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
ssize_t NW::Socket::read_full(std::vector<char>& buf)
{
    const ssize_t n{read(m_fd, buf.data(), buf.size())};
    if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 0; // Return 0 to indicate no data was read, but connection is fine.
        }
        throw std::runtime_error("read() error: " + std::string(strerror(errno)));
    }
    if (n == 0) {
        return -1; // Return -1 ONLY for "client disconnected" (EOF)
    }
    return n;
}

ssize_t NW::Socket::write_all(const std::vector<char>& buf)
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

void NW::Socket::set_non_blocking()
{
    int flags{fcntl(m_fd, F_GETFL, 0)};
    if (flags == -1)
    {
        throw std::runtime_error("Socket::set_non_blocking() - fcntl(F_GETFL) error: " + std::string(strerror(errno)));
    }

    flags |= O_NONBLOCK;

    if (fcntl(m_fd, F_SETFL, flags) == -1) {
        throw std::runtime_error("Socket::set_non_blocking() - fcntl(F_SETFL) failed: " + std::string(strerror(errno)));
    }
}











