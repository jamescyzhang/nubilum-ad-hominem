#ifndef COMM_CLIENT_HPP
#define COMM_CLIENT_HPP

#include <string>
#include <net/tcp_client.hpp>

namespace nubilum_ad_hominem
{
    class client
    {
    public:
        client();

        client(std::string str_addr, std::string str_port);

        bool init_connect(std::string str_addr, std::string str_port);

        int comm_thread();

        virtual int run();

    protected:
        net::tcp_client *m_client;
    };
}

#endif //COMM_CLIENT_HPP
