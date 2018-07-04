#ifndef NET_TCP_CLIENT_HPP
#define NET_TCP_CLIENT_HPP

#include <netdb.h>

#include <net/node.hpp>

namespace net
{
    class tcp_client : public net::node
    {
    public:
        explicit tcp_client(log_fn_callback logger,
                            settings_flag settings = ALL_FLAGS);

        ~tcp_client() override;

        tcp_client(const tcp_client &) = delete;

        tcp_client &operator=(const tcp_client &) = delete;

        bool init_connect(std::string str_server, std::string str_port);

        int receive(char *data_ptr, size_t size) const;

        bool send(const char *data_ptr, size_t size) const;

        bool send(const std::string &data) const;

        bool send(const std::vector<char> &data) const;

        bool disconnect();

    protected:
        enum connection_status
        {
            CONNECTED,
            DISCONNECTED
        };

        connection_status m_status;
        socket_fd m_socket;
        struct addrinfo *m_p_result_addrinfo;
        struct addrinfo m_hints_addr_info;
    };
}


#endif //NET_TCP_CLIENT_HPP
