#include <gtest/gtest.h>
#include "Socket.h"
#include "Connection.h"
#include "Epoll.h"
#include <thread>
#include <future>
#include <map>
#include <memory>
#include <atomic>

TEST(EpollServerTest, HandlesClientEcho)
{
    std::atomic<bool> server_should_stop = false;

    // --- Server Thread Logic ---
    std::thread server_thread([&server_should_stop]() {
        try {
            NW::Socket listening_socket{};
            listening_socket.set_non_blocking();
            listening_socket.set_reuse_addr();
            listening_socket.bind_to(8000);
            listening_socket.listen_for();

            NW::Epoll epoll;
            epoll.add(listening_socket, &listening_socket, EPOLLIN);

            std::vector<struct epoll_event> events(16);
            std::map<int, std::unique_ptr<NW::Connection>> connections;

            while (!server_should_stop) {
                // Use a short timeout so the loop can check server_should_stop
                int n_ready = epoll.wait(events, 100);

                for (int i = 0; i < n_ready; ++i) {
                    if (events[i].data.ptr == &listening_socket) {
                        NW::Socket client_socket = listening_socket.accept_connection();
                        client_socket.set_non_blocking();
                        int client_fd = client_socket.get_fd();
                        auto conn = std::make_unique<NW::Connection>(std::move(client_socket));

                        // New connections are ONLY interested in reading
                        epoll.add(conn->get_socket(), conn.get(), EPOLLIN);
                        connections[client_fd] = std::move(conn);
                    } else {
                        auto conn = static_cast<NW::Connection*>(events[i].data.ptr);

                        if (events[i].events & EPOLLIN) {
                            conn->handle_read();
                        }
                        // Use 'else if' because a socket might be closed after reading
                        else if (events[i].events & EPOLLOUT) {
                            conn->handle_write();
                        }

                        // --- Correct State Management ---
                        if (conn->get_should_close()) {
                            epoll.remove(conn->get_socket());
                            connections.erase(conn->get_socket().get_fd());
                        } else if (conn->has_data_to_write()) {
                            // We have data to write, so we need EPOLLOUT
                            epoll.modify(conn->get_socket(), conn, EPOLLIN | EPOLLOUT);
                        } else {
                            // Write buffer is empty, we only care about EPOLLIN
                            epoll.modify(conn->get_socket(), conn, EPOLLIN);
                        }
                    }
                }
            }
        } catch (const std::runtime_error& e) {
            FAIL() << "Server thread threw an exception: " << e.what();
        }
    });

    // --- Client (Main Thread) Logic ---
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Give server time to start
    NW::Socket client_socket{};
    ASSERT_NO_THROW(client_socket.connect_to("127.0.0.1", 8000));

    // Send message to server
    std::string msg_body = "echo_this";
    uint32_t len = msg_body.length();
    std::vector<char> request(4 + len);
    memcpy(request.data(), &len, 4);
    memcpy(request.data() + 4, msg_body.c_str(), len);
    write(client_socket.get_fd(), request.data(), request.size());

    // Read echo response from server
    uint32_t response_len;
    read(client_socket.get_fd(), &response_len, 4); // Read header

    std::vector<char> response_body_buf(response_len);
    read(client_socket.get_fd(), response_body_buf.data(), response_len); // Read body
    std::string received_echo(response_body_buf.begin(), response_body_buf.end());

    // --- Assertion ---
    EXPECT_EQ(received_echo, "Hello! Hello! This is the server ^^! <3");

    // --- Cleanup ---
    server_should_stop = true; // Signal the server thread to exit its loop
    server_thread.join();
}