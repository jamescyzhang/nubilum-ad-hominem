#include <cstdio>
#include <iostream>

#include <comm/server.hpp>

int main(int argc, char const *argv[])
{
    nubilum_ad_hominem::server *server = new nubilum_ad_hominem::server("669");
    server->run();
}
