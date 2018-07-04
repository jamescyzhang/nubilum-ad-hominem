#ifndef NUBILUM_AD_HOMINEM_MOBILE_HPP
#define NUBILUM_AD_HOMINEM_MOBILE_HPP


#include <comm/client.hpp>
#include <string>

class mobile : nubilum_ad_hominem::client
{
public:
    mobile();

    mobile(std::string str_addr, std::string str_port);

    virtual int run();
};


#endif //NUBILUM_AD_HOMINEM_MOBILE_HPP
