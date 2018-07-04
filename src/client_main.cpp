#include <iostream>
#include <comm/client.hpp>

int main(int argc, char const *argv[])
{
    nubilum_ad_hominem::client *client = new nubilum_ad_hominem::client("127.0.0.1", "669");
    client->run();
}