#include "StatusNotification.h"
#include "ocpp/common/Common.h"

using namespace rapidjson;
namespace ocpp1_6
{
    namespace state
    {
        StatusNotification::StatusNotification(client::IOCPPClient &client) : m_client(client) {}
        StatusNotification::~StatusNotification() {}

        bool StatusNotification::sendCall(unsigned int connectorId,
                                          const ChargePointErrorCode &errorCode,
                                          ChargePointStatus status,
                                          const std::string &otherJson)
        {
            StatusNotificationOther otherInfo = parseStatusNotificationOther(otherJson);

            rapidjson::Document req;
            if (!buildStatusNotificationMessage(req, connectorId, errorCode, status, otherInfo))
            {
                e_log("build StatusNotification message failed");
                return false;
            }

            rapidjson::Document rsp;
            if (!m_client.sendCall(STATUS_NOTIFICATION, req, rsp))
            {
                e_log("send StatusNotification message failed");
                return false;
            }

            return handleResponse(rsp);
        }

        bool StatusNotification::handleResponse(const rapidjson::Document &doc)
        {
            m_strCode.clear();
            m_strErrMsg.clear();

            ocpp1_6::MessageType messageType = static_cast<ocpp1_6::MessageType>(doc[0].GetInt());

            if (ocpp1_6::MessageType::CallResult == messageType)
            {
                return true;
            }

            if (ocpp1_6::MessageType::CallError == messageType)
            {

                if (doc[2].IsString())
                    m_strCode = doc[2].GetString();
                else
                    m_strCode = "UnknownError";

                if (doc[3].IsString())
                    m_strErrMsg = doc[3].GetString();
                else
                    m_strErrMsg = "Unknown error description";

                return false;
            }

            m_strCode = "InvalidMsgType";
            m_strErrMsg = "Unexpected message type in StatusNotification response";

            return false;
        }

        void StatusNotification::getErrorMsg(std::string &code, std::string &errMsg)
        {
            code = m_strCode;
            errMsg = m_strErrMsg;
        }

        bool StatusNotification::buildStatusNotificationMessage(rapidjson::Document &doc,
                                                                unsigned int connectorId,
                                                                const ChargePointErrorCode &errorCode,
                                                                ChargePointStatus status,
                                                                const StatusNotificationOther &other)
        {
            doc.SetObject();
            auto &alloc = doc.GetAllocator();

            // vendor 信息
            if (!other.vendorId.empty())
            {
                doc.AddMember("vendorId", rapidjson::Value(other.vendorId.c_str(), alloc), alloc);
            }

            if (!other.vendorErrorCode.empty())
            {
                std::string truncated = other.vendorErrorCode.substr(0, 49);
                doc.AddMember("vendorErrorCode", rapidjson::Value(truncated.c_str(), alloc), alloc);
            }

            // 标准字段
            doc.AddMember("connectorId", connectorId, alloc);
            doc.AddMember("errorCode", rapidjson::Value(chargePointErrorCodeMap.toString(errorCode, "NoError").c_str(), alloc), alloc);
            doc.AddMember("status", rapidjson::Value(chargePointStatusMap.toString(status, "Available").c_str(), alloc), alloc);


            // 时间
            std::string ts = Time::DateTime::now().str();
            doc.AddMember("timestamp", rapidjson::Value(ts.c_str(), alloc), alloc);

            return true;
        }

    } // namespace state
} // namespace ocpp1_6
