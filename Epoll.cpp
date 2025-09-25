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
