#include "tcp_client.hpp"

#include <iostream>
#include <vector>

#include <cstdarg>

#include <unistd.h>

#include <net/utils.hpp>

namespace net
{
    tcp_client::tcp_client(const log_fn_callback logger, const settings_flag settings) :
            node(logger, settings), m_status(DISCONNECTED), m_p_result_addrinfo(nullptr), m_socket(INVALID_SOCKET)
    {
    }

    bool tcp_client::init_connect(const std::string str_server, const std::string str_port)
    {
        if (m_status == CONNECTED)
        {
            disconnect();
            if (m_settings_flags & ENABLE_LOG)
                m_logger(str_format(
                        "[tcp_client][warning] Opening a new connection; the last connection was automatically closed."));
        }

        memset(&m_hints_addr_info, 0, sizeof(m_hints_addr_info));
        m_hints_addr_info.ai_family = AF_INET;
        m_hints_addr_info.ai_socktype = SOCK_STREAM;

        if (getaddrinfo(str_server.c_str(), str_port.c_str(), &m_hints_addr_info,
                        &m_p_result_addrinfo) != 0)
        {
            if (m_settings_flags & ENABLE_LOG)
                m_logger(str_format("[tcp_client][error] getaddrinfo failed: %s", strerror(errno)));
            if (m_p_result_addrinfo != nullptr)
            {
                freeaddrinfo(m_p_result_addrinfo);
                m_p_result_addrinfo = nullptr;
            }
            return false;
        }

        struct addrinfo *p_res;
        for (p_res = m_p_result_addrinfo; p_res != nullptr; p_res = p_res->ai_next)
        {
            m_socket = socket(p_res->ai_family, p_res->ai_socktype, p_res->ai_protocol);
            if (m_socket < 0)
                continue;

            if (connect(m_socket, p_res->ai_addr, p_res->ai_addrlen) >= 0)
            {
                m_status = CONNECTED;
                if (m_p_result_addrinfo != nullptr)
                {
                    freeaddrinfo(m_p_result_addrinfo);
                    m_p_result_addrinfo = nullptr;
                }
                return true;
            }
            close(m_socket);
        }
        if (m_p_result_addrinfo != nullptr)
        {
            freeaddrinfo(m_p_result_addrinfo);
            m_p_result_addrinfo = nullptr;
        }
        if (m_settings_flags & ENABLE_LOG)
            m_logger(str_format("[tcp_client][error] no such host"));
        return false;
    }

    bool tcp_client::send(const char *data_ptr, const size_t size) const
    {
        if (m_status != CONNECTED)
        {
            if (m_settings_flags & ENABLE_LOG)
                m_logger(str_format("[tcp_client][error] send failed : not connected to a server"));
            return false;
        }

        if (write(m_socket, data_ptr, size) < 0)
        {
            if (m_settings_flags & ENABLE_LOG)
                m_logger(str_format("[tcp_client][error] writing to socket: %s", strerror(errno)));
            return false;
        }

        return true;
    }

    bool tcp_client::send(const std::string &data) const
    {
        return send(data.c_str(), data.length());
    }

    bool tcp_client::send(const std::vector<char> &data) const
    {
        return send(data.data(), data.size());
    }

    int tcp_client::receive(char *data_ptr, const size_t size) const
    {
        if (m_status != CONNECTED)
        {
            if (m_settings_flags & ENABLE_LOG)
                m_logger(str_format("[tcp_client][error] recv failed : not connected to a server"));
            return -1;
        }

        int i_bytes_rcvd = read(m_socket, data_ptr, size);
        if (i_bytes_rcvd < 0)
        {
            if (m_settings_flags & ENABLE_LOG)
                m_logger(str_format("[tcp_client][error] reading from socket: %s", strerror(errno)));
        }
        data_ptr[i_bytes_rcvd] = 0;
        return i_bytes_rcvd;
    }

    bool tcp_client::disconnect()
    {
        if (m_status != CONNECTED)
            return true;
        m_status = DISCONNECTED;
        close(m_socket);
        m_socket = INVALID_SOCKET;
        return true;
    }

    tcp_client::~tcp_client()
    {
        if (m_status == CONNECTED)
            disconnect();
    }
}
