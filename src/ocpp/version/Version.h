#ifndef OCPP_VERSION_H
#define OCPP_VERSION_H

#include <string>

// 返回 OCPP 软件版本号
// 格式：OCPP<protocol>-g<git>
//
// 示例：OCPP1.6g8c91fa2
std::string getOcppSoftwareVersion();

#endif // OCPP_VERSION_H
