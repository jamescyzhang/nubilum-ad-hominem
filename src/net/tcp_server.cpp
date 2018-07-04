#include "tcp_server.hpp"

#include <iostream>
#include <vector>

#include <cstdarg>

#include <arpa/inet.h>
#include <unistd.h>

#include <net/utils.hpp>

namespace net
{
    tcp_server::tcp_server(const log_fn_callback logger, const std::string &str_port,
                           const settings_flag settings) noexcept(false) :
            node(logger, settings), m_listen_socket(INVALID_SOCKET), m_str_port(str_port)
    {
        bzero((char *) &m_serv_addr, sizeof(m_serv_addr));

        int i_port = atoi(str_port.c_str());

        m_serv_addr.sin_family = AF_INET;
        m_serv_addr.sin_addr.s_addr = INADDR_ANY;
        m_serv_addr.sin_port = htons(i_port);
    }

    bool tcp_server::start_listen(node::socket_fd &client_socket)
    {
        client_socket = INVALID_SOCKET;
        if (m_listen_socket == INVALID_SOCKET)
        {
            m_listen_socket = socket(AF_INET, SOCK_STREAM, 0);
            if (m_listen_socket < 0)
            {
                if (m_settings_flags & ENABLE_LOG)
                    m_logger(str_format("[tcp_server][error] opening socket: %s", strerror(errno)));
                m_listen_socket = INVALID_SOCKET;
                return false;
            }

            int opt = 1;
            if( setsockopt(m_listen_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
                           sizeof(opt)) < 0 )
            {
                if (m_settings_flags & ENABLE_LOG)
                    m_logger(str_format("[tcp_server][error] setsockopt: %s", strerror(errno)));
                return false;
            }

            if (bind(m_listen_socket, reinterpret_cast<struct sockaddr *>(&m_serv_addr), sizeof(m_serv_addr)) < 0)
            {
                if (m_settings_flags & ENABLE_LOG)
                    m_logger(str_format("[tcp_server][error] bind failed: %s", strerror(errno)));
                return false;
            }
        }

        if (listen(m_listen_socket, SOMAXCONN) < 0)
        {
            if (m_settings_flags & ENABLE_LOG)
                m_logger(str_format("[tcp_server][error] listen failed: %s", strerror(errno)));
            return false;
        }

        struct sockaddr_in client_addr;
        socklen_t u_client_len = sizeof(client_addr);
        client_socket = accept(m_listen_socket, reinterpret_cast<struct sockaddr *>(&client_addr), &u_client_len);
        if (client_socket < 0)
        {
            if (m_settings_flags & ENABLE_LOG)
                m_logger(str_format("[tcp_server][error] accept failed: %s", strerror(errno)));
            return false;
        }

        if (m_settings_flags & ENABLE_LOG)
            m_logger(str_format("[tcp_server][info] Incoming connection from '%s' port '%d'",
                                inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port)));

        return true;
    }

    int tcp_server::receive(socket_fd &client_socket, char *data_ptr, const size_t size) const
    {
        int i_bytes_rcvd = static_cast<int>(read(client_socket, data_ptr, size));
        if (i_bytes_rcvd < 0)
        {
            if (m_settings_flags & ENABLE_LOG)
                m_logger(str_format("[tcp_server][error] reading from socket: %s", strerror(errno)));
        }
        data_ptr[i_bytes_rcvd] = 0;
        return i_bytes_rcvd;
    }

    bool tcp_server::send(const socket_fd client_socket, const char *data_ptr, const size_t size) const
    {
        int i_res = static_cast<int>(write(client_socket, data_ptr, size));
        if (i_res < 0)
        {
            if (m_settings_flags & ENABLE_LOG)
                m_logger(str_format("[tcp_server][error] writing to socket: %s", strerror(errno)));
            return false;
        }
        return true;
    }

    bool tcp_server::send(const socket_fd client_socket, const std::string &data) const
    {
        return send(client_socket, data.c_str(), data.length());
    }

    bool tcp_server::send(const socket_fd client_socket, const std::vector<char> &data) const
    {
        return send(client_socket, data.data(), data.size());
    }

    bool tcp_server::disconnect(const socket_fd client_socket) const
    {
        close(client_socket);
        return true;
    }

    tcp_server::~tcp_server()
    {
        close(m_listen_socket);
    }
}