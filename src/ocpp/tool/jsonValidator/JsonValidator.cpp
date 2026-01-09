#include "JsonValidator.h"
#include "utils/utils.h"

JsonValidator::JsonValidator(const std::string& schemaDirectory) : m_schemaDirectory(schemaDirectory)
{
    i_log("JsonValidator initialized. schemaDir = %s", schemaDirectory.c_str());
}

bool JsonValidator::validateRequest(const std::string& action, const rapidjson::Document& reqMsg)
{
    d_log("Validate request: action=%s", action.c_str());
    if (validateJson(action, reqMsg, ".json"))
    {
        return true;
    }
    else
    {
        rapidjson::StringBuffer                    buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        reqMsg.Accept(writer);
        e_log("Invalid request: action=%s \n reqMsg=%s", action.c_str(), buffer.GetString());
        return false;
    }
}

bool JsonValidator::validateResponse(const std::string& action, const rapidjson::Document& rspMsg)
{
    d_log("validate response: action=%s", action.c_str());
    if (validateJson(action, rspMsg, "Response.json"))
    {
        return true;
    }
    else
    {
        rapidjson::StringBuffer                    buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        rspMsg.Accept(writer);
        e_log("Invalid response: action=%s \n rspMsg=%s", action.c_str(), buffer.GetString());
        return false;
    }
}

/**
 * @brief 核心校验逻辑
 */
bool JsonValidator::validateJson(const std::string& action, const rapidjson::Document& doc, const std::string& typeSuffix)
{
    std::string schemaPath = m_schemaDirectory + "/" + action + typeSuffix;

    rapidjson::Document schemaDoc = loadSchemaFromFile(schemaPath);
    if (schemaDoc.IsNull())
    {
        e_log("Schema load failed: %s", schemaPath.c_str());
        return false;
    }

    rapidjson::SchemaDocument  schema(schemaDoc);
    rapidjson::SchemaValidator validator(schema);

    if (!doc.Accept(validator))
    {
        // 打印 schema 错误
        rapidjson::StringBuffer sb;

        validator.GetInvalidSchemaPointer().StringifyUriFragment(sb);
        e_log("JSON schema error: invalid schema pointer: %s", sb.GetString());
        e_log("Schema keyword: %s", validator.GetInvalidSchemaKeyword());

        sb.Clear();
        validator.GetInvalidDocumentPointer().StringifyUriFragment(sb);
        e_log("Invalid document pointer: %s", sb.GetString());

        return false;
    }

    return true;
}

/**
 * @brief 加载 schema JSON 文件
 */
rapidjson::Document JsonValidator::loadSchemaFromFile(const std::string& path)
{
    // 打开文件
    FILE* fp = fopen(path.c_str(), "rb");
    if (!fp)
    {
        e_log("Failed to open schema file: %s", path.c_str());
        return rapidjson::Document();
    }

    // 读取整个文件内容
    fseek(fp, 0, SEEK_END);
    long length = ftell(fp);
    rewind(fp);

    if (length <= 0)
    {
        e_log("Schema file is empty: %s", path.c_str());
        fclose(fp);
        return rapidjson::Document();
    }

    std::string json;
    json.resize(static_cast<size_t>(length));

    size_t readSize = fread(&json[0], 1, json.size(), fp);
    fclose(fp);

    if (readSize != json.size())
    {
        e_log("Read schema file failed [%s]", path.c_str());
        return rapidjson::Document();
    }

    rapidjson::Document doc;
    doc.Parse(json.c_str());

    if (doc.HasParseError())
    {
        e_log("Schema parse error [%s]: error=%d, offset=%zu", path.c_str(), doc.GetParseError(), doc.GetErrorOffset());
        return rapidjson::Document();
    }

    return doc;
}
