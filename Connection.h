//
// Created by regal on 9/24/25.
//

#ifndef REDISKNOCKOFF_CONNECTION_H
#define REDISKNOCKOFF_CONNECTION_H

#include "Socket.h"

namespace NW
{
    class Connection
    {
    public:
        explicit Connection(Socket&& socket);

        void handle_read();     // Event loop will call this whenever epoll reports that client's socket is ready for reading
        void handle_write();    // Event loop will call this whenever epoll reports that socket is ready for writing.

        [[nodiscard]] Socket& get_socket() {return m_socket;}
        [[nodiscard]] bool get_should_close() const {return m_should_close;}
        [[nodiscard]] bool has_data_to_write() const {return !m_write_buffer.empty();}
    private:
        static constexpr size_t K_MAX_MSG{4096};

        // Internal helper to parse messages from the read buffer.
        void process_request();     // Protocol parser: acts on complete messages.

        Socket m_socket{};
        std::vector<char> m_read_buffer{};      // Assembles incoming data chunks
        std::vector<char> m_write_buffer{};     // Queues outgoing data.
        bool m_should_close{false};             // Flag to mark connection for removal
    };
}

#endif //REDISKNOCKOFF_CONNECTION_H