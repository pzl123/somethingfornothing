#include "Common.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <unordered_set>
#include <uuid/uuid.h>

#include "utils/utils.h"

#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "ocpp/config/standardConfigurationKeyNames.h"
#include "ocpp/config/chargePointConfigurationKeynames.h"

namespace ocpp1_6
{
    bool validateOCPPMessage(const std::string &jsonMessage, MessageType &type, rapidjson::Document &document)
    {
        // Step 1: 解析 JSON 字符串
        rapidjson::Document doc;
        doc.Parse(jsonMessage.c_str());
        if (doc.HasParseError())
        {
            d_log("Invalid JSON format: %s (offset %zu)",rapidjson::GetParseError_En(doc.GetParseError()), doc.GetErrorOffset());
            return false;
        }

        // Step 2: 必须是数组
        if (!doc.IsArray())
        {
            d_log("Invalid JSON format: Not an array.");
            return false;
        }

        rapidjson::Value &arr = doc;
        if (arr.Size() < 3)
        {
            d_log("Invalid JSON format: Array too short (min 3 elements).");
            return false;
        }

        // Step 3: 第一个元素必须是整数（消息类型）
        if (!arr[0].IsInt())
        {
            d_log("Invalid message type: First element is not an integer.");
            return false;
        }

        int msgType = arr[0].GetInt();
        if (msgType < 2 || msgType > 4)
        {
            d_log("Invalid message type: Must be 2 (Call), 3 (CallResult), or 4 (CallError).");
            return false;
        }

        type = static_cast<MessageType>(msgType);

        // Step 4: 根据消息类型校验结构
        switch (type)
        {
        case MessageType::Call:
        {
            if (arr.Size() != 4)
            {
                d_log("Invalid Call message: Must have exactly 4 elements.");
                return false;
            }
            if (!arr[1].IsString() || !arr[2].IsString() || !arr[3].IsObject())
            {
                d_log("Invalid Call message: Expected [2, uniqueId(string), action(string), payload(object)]");
                return false;
            }
            break;
        }

        case MessageType::CallResult:
        {
            if (arr.Size() != 3)
            {
                d_log("Invalid CallResult message: Must have exactly 3 elements.");
                return false;
            }
            if (!arr[1].IsString() || !arr[2].IsObject())
            {
                d_log("Invalid CallResult message: Expected [3, uniqueId(string), payload(object)]");
                return false;
            }
            break;
        }

        case MessageType::CallError:
        {
            if (arr.Size() != 5)
            {
                d_log("Invalid CallError message: Must have exactly 5 elements.");
                return false;
            }
            if (!arr[1].IsString() || !arr[2].IsString() ||
                !arr[3].IsString() || !arr[4].IsObject())
            {
                d_log("Invalid CallError message: Expected [4, uniqueId(string), errorCode(string), errorDescription(string), errorDetails(object)]");
                return false;
            }
            break;
        }

        default:
            d_log("Unexpected message type (should not happen).");
            return false;
        }

        // Step 5: 将解析结果移动到输出参数 document
        // 注意：RapidJSON Document 不支持直接赋值，需 Swap 或 Parse 再传引用
        // 这里我们假设调用方传入的 document 是空的，可以直接 swap
        if (document.GetType() != rapidjson::kNullType)
        {
            document.SetNull(); // 清空目标 document
        }
        document.Swap(doc); // 高效转移所有权

        return true;
    }

    bool ocppisInt(const std::string& strVal)
    {
        return std::all_of(strVal.begin(), strVal.end(), ::isdigit);
    }

    bool ocppisUint(const std::string &strVal)
    {
        size_t pos;
        try
        {
            unsigned long num = std::stoul(strVal, &pos);
            if (pos != strVal.size())
            {
                return false; // 存在非法后缀字符
            }
            if (num > UINT_MAX)
            {
                return false; // 超出 uint 范围
            }
            return true;
        }
        catch (const std::invalid_argument &)
        {
            e_log("error invalid_argument");
            return false; // 非数字字符串
        }
        catch (const std::out_of_range &)
        {
            e_log("error out_of_range");
            return false; // 数值过大
        }
    }

     bool ocppisBool(const std::string& strVal)
    {
        return strVal == "true" || strVal == "false";
    }

    bool ocppstringToBool(const std::string& strVal)
    {
        return strVal == "true";
    }

    bool keyValueIsInt(const std::string &key)
    {
        static const std::unordered_set<std::string> intKeys =
        {
            ocpp1_6::config::ClockAlignedDataInterval,
            ocpp1_6::config::GetConfigurationMaxKeys,
            ocpp1_6::config::HeartbeatInterval,
            ocpp1_6::config::LocalAuthListMaxLength,
            ocpp1_6::config::MeterValueSampleInterval,
            ocpp1_6::config::MeterValuesAlignedDataMaxLength,
            ocpp1_6::config::MeterValuesSampledDataMaxLength,
            ocpp1_6::config::NumberOfConnectors,
            ocpp1_6::config::ResetRetries,
            ocpp1_6::config::SendLocalListMaxLength,
            ocpp1_6::config::StopTxnAlignedDataMaxLength,
            ocpp1_6::config::StopTxnSampledDataMaxLength,
            ocpp1_6::config::SupportedFeatureProfilesMaxLength,
            ocpp1_6::config::TransactionMessageAttempts,
            ocpp1_6::config::TransactionMessageRetryInterval,
            ocpp1_6::config::WebSocketPingInterval,
            ocpp1_6::config::ConnectionTimeOut
        };
        return intKeys.find(key) != intKeys.end();
    }

     bool keyValueIsBool(const std::string& key)
    {
        static const std::unordered_set<std::string> boolKeys =
        {
            ocpp1_6::config::AllowOfflineTxForUnknownId,
            ocpp1_6::config::AuthorizationCacheEnabled,
            ocpp1_6::config::AuthorizeRemoteTxRequests,
            ocpp1_6::config::LocalAuthListEnabled,
            ocpp1_6::config::LocalAuthorizeOffline,
            ocpp1_6::config::LocalPreAuthorize,
            ocpp1_6::config::ReserveConnectorZeroSupported,
            ocpp1_6::config::StopTransactionOnEVSideDisconnect,
            ocpp1_6::config::StopTransactionOnInvalidId
        };
        return boolKeys.find(key) != boolKeys.end();
    }

    bool keyValueIsString(const std::string &key)
    {
        static const std::unordered_set<std::string> stringKeys =
        {
            ocpp1_6::config::ConnectorPhaseRotation,
            ocpp1_6::config::MeterValuesAlignedData,
            ocpp1_6::config::MeterValuesSampledData,
            ocpp1_6::config::StopTxnAlignedData,
            ocpp1_6::config::StopTxnSampledData,
            ocpp1_6::config::SupportedFeatureProfiles
        };

        return stringKeys.find(key) != stringKeys.end();
    }

    bool jsonSerialize(const std::string& jsonStr, rapidjson::Document &json)
    {
        json.Parse(jsonStr.c_str());
        if (true == json.HasParseError())
        {
            e_log("error parse json:%s", jsonStr);
            return false;
        }
        return true;
    }

    bool jsonDeserialize(const rapidjson::Document &json, std::string &str)
    {
        if ((false == json.IsObject()) && (false == json.IsArray()))
        {
            e_log("json not object or array");
            return false;
        }

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        json.Accept(writer);
        str = buffer.GetString();
    }

    std::string generateMessageId()
    {
        uuid_t id;
        uuid_generate(id);

        char str[37]; // UUID 36位 + '\0'
        uuid_unparse_lower(id, str);
        return std::string(str);
    }

    uint32_t generateOrderId()
    {
        uuid_t id;
        uuid_generate(id);

        uint32_t orderId = 0;
        for (int i = 0; i < 4; ++i)
        {
            orderId = (orderId << 8) | id[i];
        }
        return orderId;
    }
}

