//
// Created by regal on 9/24/25.
//

#include <gtest/gtest.h>
#include "Connection.h"
#include "sys/socket.h"
#include <fcntl.h>

class ConnectionTest : public ::testing::Test
{
protected:
    void SetUp() override {
        int fds[2];
        if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == -1) {
            FAIL() << "Failed to create socketpair";
        }
        client_fd = fds[0];
        server_fd = fds[1];
        int flags = fcntl(server_fd, F_GETFL, 0);
        fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);
        // The connection object will manage the server end of the pair
        connection = std::make_unique<NW::Connection>(NW::Socket(server_fd));
    }

    void TearDown() override {
        close(client_fd);
        // server_fd is closed by the Connection's destructor
    }

    int client_fd{};
    int server_fd{};
    std::unique_ptr<NW::Connection> connection;
};

TEST_F(ConnectionTest, handlesSimpleRequests)
{
    const std::string message{"Hello"};
    uint32_t len{static_cast<uint32_t>(message.length())};

    std::vector<char> req(4 + len);
    memcpy(req.data(), &len, 4);
    memcpy(req.data() + 4, message.data(), len);

    write(client_fd, req.data(), req.size());

    connection->handle_read();

    ASSERT_FALSE(connection->get_should_close());
    ASSERT_TRUE(connection->has_data_to_write());

    connection->handle_write();

    std::vector<char> response_buf(1024);
    read(client_fd, response_buf.data(), 4);
    uint32_t response_len = 0;
    memcpy(&response_len, response_buf.data(), 4);

    read(client_fd, response_buf.data(), response_len);
    std::string server_response(response_buf.data(), response_len);
    ASSERT_EQ(server_response, "Hello! Hello! This is the server ^^! <3");
}

TEST_F(ConnectionTest, HandlesClientDisconnect) {
    // 1. Close the client end of the socket
    close(client_fd);
    client_fd = -1; // Prevent TearDown from closing it again

    // 2. Trigger the read handler
    connection->handle_read();

    // 3. Assert that the connection is marked for closing
    ASSERT_TRUE(connection->get_should_close());
}