//
// Created by regal on 9/24/25.
//

#include "Epoll.h"
#include <cstring>
#include <cerrno>

NW::Epoll::Epoll()
{
    m_epoll_fd = epoll_create1(0);
    if (m_epoll_fd < 0)
    {
        throw std::runtime_error("Epoll() - epoll_create1() failed: " + std::string(strerror(errno)));
    }
}

NW::Epoll::~Epoll() {
    // The file descriptor is automatically closed when the object is destroyed.
    if (m_epoll_fd >= 0) {
        close(m_epoll_fd);
    }
}

void NW::Epoll::add(const NW::Socket& socket, void* user_data, const uint32_t events) {
    struct epoll_event ev = {};
    ev.events = events;
    ev.data.ptr = user_data; // Associate our custom data pointer with the fd

    if (epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, socket.get_fd(), &ev) != 0) {
        throw std::runtime_error("Epoll::add() - epoll_ctl(ADD) failed: " + std::string(strerror(errno)));
    }
}

void NW::Epoll::modify(const NW::Socket& socket, void* user_data, const uint32_t events)
{
    struct epoll_event ev{};
    ev.events = events;
    ev.data.ptr = user_data;

    if (epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, socket.get_fd(), &ev) != 0)
    {
        throw std::runtime_error("Epoll::modify() - epoll_ctl(MOD) failed: " + std::string(strerror(errno)));
    }
}

void NW::Epoll::remove(const NW::Socket& socket)
{
    if (epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, socket.get_fd(), nullptr) != 0) {
        throw std::runtime_error("Epoll::remove - epoll_ctl(DEL) failed: " + std::string(strerror(errno)));
    }
}

int NW::Epoll::wait(std::vector<struct epoll_event>& events, const int timeout_ms)
{
    const int n_ready{epoll_wait(m_epoll_fd, events.data(), static_cast<int>(events.size()), timeout_ms)};

    if (n_ready < 0) {
        // An interrupted system call is not a fatal error, the loop can just continue.
        if (errno == EINTR) {
            return 0;
        }
        // All other errors are fatal.
        throw std::runtime_error("Epoll::wait() - epoll_wait() failed: " + std::string(strerror(errno)));
    }

    return n_ready;
}


