#ifndef JSON_VALIDATOR_H
#define JSON_VALIDATOR_H

#include "ocpp_json_minimal.h"
#include <string>

class JsonValidator
{
  public:
    /**
     * @brief 构造函数，传入 schema 所在目录。
     */
    explicit JsonValidator(const std::string& schemaDirectory);

    /**
     * @brief 校验请求报文（xxx.json）
     * @param action 例如 "BootNotification"
     */
    bool validateRequest(const std::string& action, const rapidjson::Document& reqMsg);

    /**
     * @brief 校验响应报文（xxxResponse.json）
     * @param action 例如 "BootNotification"
     */
    bool validateResponse(const std::string& action, const rapidjson::Document& rspMsg);

  private:
    std::string m_schemaDirectory;

    /// 内部统一校验入口
    bool validateJson(const std::string& action, const rapidjson::Document& doc, const std::string& typeSuffix);

    /// 从文件加载并解析 schema
    rapidjson::Document loadSchemaFromFile(const std::string& path);
};

#endif // JSON_VALIDATOR_H
