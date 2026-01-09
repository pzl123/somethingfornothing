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

        type = static_cast<MessageType>(arr[0].GetInt());
        if (type < MessageType::Call || MessageType::CallError < type)
        {
            d_log("Invalid message type: Must be 2 (Call), 3 (CallResult), or 4 (CallError).");
            return false;
        }

        // -----------------------------
        if (type == MessageType::Call)
        {
            if (document.Size() != 4)
            {
                e_log("Call message size=%u, expected 4", document.Size());
                return false;
            }

            if (!document[1].IsString())
            {
                e_log("Call[1] uniqueId must be string");
                return false;
            }
            if (!document[2].IsString())
            {
                e_log("Call[2] action must be string");
                return false;
            }
            if (!document[3].IsObject())
            {
                e_log("Call[3] payload must be JSON object");
                return false;
            }

            return true;
        }

        // -----------------------------
        // Check CallResult message
        // -----------------------------
        if (type == MessageType::CallResult)
        {
            if (document.Size() != 3)
            {
                e_log("CallResult size=%u, expected 3", document.Size());
                return false;
            }

            if (!document[1].IsString())
            {
                e_log("CallResult[1] uniqueId must be string");
                return false;
            }
            if (!document[2].IsObject())
            {
                e_log("CallResult[2] payload must be JSON object");
                return false;
            }

            return true;
        }

        // -----------------------------
        // Check CallError message
        // -----------------------------
        if (type == MessageType::CallError)
        {
            if (document.Size() != 5)
            {
                e_log("CallError size=%u, expected 5", document.Size());
                return false;
            }

            if (!document[1].IsString())
            {
                e_log("CallError[1] uniqueId must be string");
                return false;
            }
            if (!document[2].IsString())
            {
                e_log("CallError[2] errorCode must be string");
                return false;
            }
            if (!document[3].IsString())
            {
                e_log("CallError[3] errorDescription must be string");
                return false;
            }
            if (!document[4].IsObject())
            {
                e_log("CallError[4] errorDetails must be JSON object");
                return false;
            }

            return true;
        }

        // Should never reach
        e_log("Unknown messageType=%d", type);
        return false;
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

