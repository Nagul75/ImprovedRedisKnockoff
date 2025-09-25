//
// Created by regal on 9/24/25.
//

#ifndef REDISKNOCKOFF_EPOLL_H
#define REDISKNOCKOFF_EPOLL_H

#include <sys/epoll.h>
#include <unistd.h>
#include <vector>
#include "Socket.h"

namespace NW
{
    class Epoll
    {
    public:
        Epoll();
        ~Epoll();

        // Unique resource, so not copyable
        Epoll(const Epoll&) = delete;
        Epoll& operator=(const Epoll&) = delete;

        void add(const NW::Socket& socket, void* user_data, const uint32_t events);
        void modify(NW::Socket& socket, void* user_data, uint32_t events);
        void remove(NW::Socket& socket);
        int wait(std::vector<struct epoll_event>& events, int timeout_ms = -1);

    private:
        int m_epoll_fd{};
    };
}


#endif //REDISKNOCKOFF_EPOLL_H