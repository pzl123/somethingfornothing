#include "Version.h"
#include <algorithm>
#include <cctype>

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

std::string getOcppSoftwareVersion()
{
    std::string v = std::string("OCPP")
                  + STR(OCPP_PROTOCOL_VERSION)
                  + "G"
                  + STR(OCPP_GIT_HASH);

    std::transform(v.begin(), v.end(), v.begin(),
                   [](unsigned char c) { return std::toupper(c); });

    return v;
}