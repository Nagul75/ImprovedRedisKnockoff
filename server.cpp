#include "Socket.h"
#include "Epoll.h"
#include "Connection.h"

#include <map>
#include <iostream>
#include <memory>

int main()
{
    try
    {
        // Setup listening socket
        NW::Socket listening_socket{};
        listening_socket.set_reuse_addr();
        listening_socket.bind_to(8000);
        listening_socket.listen_for();
        listening_socket.set_non_blocking();

        NW::Epoll epoll{};
        epoll.add(listening_socket, &listening_socket, EPOLLIN);
        std::cout << "Server listening on port 8000... \n";

        // managing connections via data structures
        std::vector<struct epoll_event> events{16};
        std::map<int, std::unique_ptr<NW::Connection>> connections{};

        // event loop
        while (true)
        {
            const int n_ready{epoll.wait(events)};

            for (int i{}; i < n_ready; ++i)
            {
                if (events[i].data.ptr == &listening_socket)    // handle new connection
                {
                    NW::Socket client_socket{listening_socket.accept_connection()};
                    client_socket.set_non_blocking();
                    int client_fd{client_socket.get_fd()};

                    auto conn{std::make_unique<NW::Connection>(std::move(client_socket))};
                    epoll.add(conn->get_socket(), conn.get(), EPOLLIN);
                    connections[client_fd] = std::move(conn);
                }
                else    // handles existing connections
                {
                    auto conn{static_cast<NW::Connection*>(events[i].data.ptr)};
                    if (events[i].events & EPOLLIN)
                    {
                        conn->handle_read();
                    }
                    else if (events[i].events & EPOLLOUT)
                    {
                        conn->handle_write();
                    }

                    // update state after handling events
                    if (conn->get_should_close() || (events[i].events & EPOLLERR))
                    {
                        epoll.remove(conn->get_socket());
                    }
                    else if (conn->has_data_to_write())
                    {
                        epoll.modify(conn->get_socket(), conn, EPOLLIN | EPOLLOUT);
                    }
                    else
                    {
                        epoll.modify(conn->get_socket(), conn, EPOLLIN);
                    }
                }
            }
        }
    }catch (std::runtime_error& e)
    {
        std::cerr << "Fatal server error: " << e.what() << '\n';
    }
}