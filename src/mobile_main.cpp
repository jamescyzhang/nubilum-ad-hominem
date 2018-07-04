#include <iostream>
#include <mobile/mobile.hpp>

int main(int argc, char const *argv[])
{
    mobile *client = new mobile("127.0.0.1", "669");
    client->run();
}