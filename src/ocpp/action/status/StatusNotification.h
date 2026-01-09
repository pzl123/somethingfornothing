#ifndef STATUSNOTIFICATION_H
#define STATUSNOTIFICATION_H

#include <string>

#include "ocpp_json_minimal.h"

#include "utils/utils.h"

#include "ocpp/action/OcppActionCommon.h"
#include "ocpp/client/OCPPClient.h"
#include "ocpp/common/EnumStrMappings.h"
#include "ocpp/common/Time.h"
#include "ocpp/type/ChargePointErrorCode.h"

using namespace rapidjson;

namespace ocpp1_6
{
    namespace state
    {
        class StatusNotification
        {
        private:
            std::string m_strCode;
            std::string m_strErrMsg;
            client::IOCPPClient &m_client; // 引用一个实现了 IOCPPClient 的具体对象

            struct StatusNotificationOther
            {
                std::string vendorId;
                std::string vendorErrorCode;
                std::string info;
            };

            /**
             * @brief 从 JSON 字符串解析 StatusNotification 可选字段
             * @param otherJsonStr 输入 JSON 字符串
             * @return StatusNotificationOther 解析后的结构体
             */
            static StatusNotificationOther parseStatusNotificationOther(const std::string &otherJsonStr)
            {

                StatusNotificationOther out;

                if (otherJsonStr.empty())
                {
                    return out;
                }

                Document doc;
                doc.Parse(otherJsonStr.c_str());
                if (doc.HasParseError() || !doc.IsObject())
                {
                    d_log("StatusNotification other JSON parse failed.");
                    return out;
                }

                if (doc.HasMember("vendorId") && doc["vendorId"].IsString())
                {
                    out.vendorId = doc["vendorId"].GetString();
                }

                if (doc.HasMember("vendorErrorCode") && doc["vendorErrorCode"].IsString())
                {
                    out.vendorErrorCode = doc["vendorErrorCode"].GetString();

                    // 最多 49 字符
                    if (out.vendorErrorCode.size() > 49)
                    {
                        out.vendorErrorCode = out.vendorErrorCode.substr(0, 49);
                    }
                }

                if (doc.HasMember("info") && doc["info"].IsString())
                {
                    out.info = doc["info"].GetString();
                }

                return out;
            }

            bool buildStatusNotificationMessage(rapidjson::Document &doc,
                                                unsigned int connectorId,
                                                const ChargePointErrorCode &errorCode,
                                                ChargePointStatus status,
                                                const StatusNotificationOther &other);

        public:
            StatusNotification(client::IOCPPClient &client);
            ~StatusNotification();

            /**
             * @brief 发送 StatusNotification 消息
             * @param connector_id 连接器ID
             * @param errorCode 错误码
             * @param status 状态
             * @param other json格式：StatusNotification的可选信息
             * @return 是否发送成功
             * @note
             * other json格式：除开这三个字段，其他字段不解析
             * {
             *   "info":"",
             *   "vendorId":"",
             *   "vendorErrorCode":""
             * }
             */
            bool sendCall(unsigned int connector_id, const ChargePointErrorCode &errorCode, ChargePointStatus status, const std::string &other);

            /**
             * @brief 处理 StatusNotification 响应
             * @param doc 响应的 JSON 文档
             * @return true:CallResult, false:CallError
             */
            bool handleResponse(const rapidjson::Document &doc);
            /**
             * @brief 当失败的时候获取错误信息
             * @param code 错误码
             * @param errMsg 错误信息
             */
            void getErrorMsg(std::string &code, std::string &errMsg);
        };

    } // namespace state
} // namespace ocpp1_6

#endif
