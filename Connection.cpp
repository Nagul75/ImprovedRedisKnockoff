//
// Created by regal on 9/24/25.
//

#include "Connection.h"

#include <cassert>
#include <cstring>

NW::Connection::Connection(Socket&& socket)
    :m_socket(std::move(socket))
{
}

void NW::Connection::handle_read()
{
    try
    {
        std::vector<char> temp_buf(4096);

        while (true)
        {
            const ssize_t bytes_read{m_socket.read_full(temp_buf)};
            if (bytes_read > 0)
            {
                // Append newly read data to Connection's read buffer
                m_read_buffer.insert(m_read_buffer.end(), temp_buf.begin(), temp_buf.begin() + bytes_read);
            }
            else if (bytes_read == 0)
            {
                break;  // Non-blocking signal:  successfully read everything available NOW
            }
            else // bytes_read < 0, indicates EOF
            {
                m_should_close = true;
                break;
            }
        }
        process_request();
    } catch (const std::runtime_error& e)
    {
        std::cerr << "Connection::handle_read() error: " << m_socket.get_fd() << ": " << e.what() << '\n';
        m_should_close = true;
    }
}

void NW::Connection::handle_write()
{
    assert(!m_write_buffer.empty());

    try
    {
        while (!m_write_buffer.empty())
        {
            const ssize_t bytes_written{m_socket.write_all(m_write_buffer)};
            if (bytes_written > 0)
            {
                // if data written, remove it from front of the buffer.
                m_write_buffer.erase(m_write_buffer.begin(), m_write_buffer.begin() + bytes_written);
            }
            else if (bytes_written == 0)
            {
                break; // Non-blocking signal: Kernel's send buffer is full and must wait till epoll signal.
            }
            else    // bytes_written < 0, fatal error
            {
                m_should_close = true;
                break;
            }
        }
    } catch (const std::runtime_error& e)
    {
        std::cerr << "Connection::handle_write() error: " << m_socket.get_fd() << ": " << e.what() << '\n';
        m_should_close = true;
    }
}

void NW::Connection::process_request()
{
    while (true)
    {
        if (m_read_buffer.size() < 4)   // request needs at least a 4-byte header
        {
            break;  // not enough data for the header.
        }

        uint32_t len{};
        std::memcpy(&len, m_read_buffer.data(), sizeof(len));

        if (len > K_MAX_MSG)
        {
            std::cerr << "Message too long on fd " << m_socket.get_fd() << '\n';
            m_should_close = true;
            break;
        }

        // check if full message body has arrived.
        if (m_read_buffer.size() < (4 + len))
        {
            break;
        }

        // Now we have a complete request.
        std::cout << "Client (fd=" << m_socket.get_fd() << ") says: "
                  << std::string(m_read_buffer.begin() + 4, m_read_buffer.begin() + len + 4) << '\n';

        // Create and buffer the response
        const std::string response{"Hello! Hello! This is the server ^^! <3"};
        uint32_t response_len{static_cast<uint32_t>(response.length())};

        // Append header to write buffer
        std::vector<char> header(sizeof(response_len));
        memcpy(header.data(), &response_len, sizeof(response_len));
        m_write_buffer.insert(m_write_buffer.end(), header.begin(), header.end());

        // Append body to write buffer
        m_write_buffer.insert(m_write_buffer.end(), response.begin(), response.end());

        // Finally remove the request from read buffer.
        m_read_buffer.erase(m_read_buffer.begin(), m_read_buffer.begin() + 4 + len);
    }
}
