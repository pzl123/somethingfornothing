#include "AuthentLocalList.h"

#include "ocpp/log/ocpp_log.h"

namespace ocpp1_6
{
    namespace auth
    {
        AuthorizeLocalList::AuthorizeLocalList(client::IOCPPClient &client, Database &database, chargepoint::Connectors &connectors)
            : m_client(client), m_database(database), m_connectors(connectors), m_local_list_version(0)
        {
        }

        AuthorizeLocalList::~AuthorizeLocalList() {}

        void AuthorizeLocalList::registHandler()
        {
            m_client.registerCallHandler(SEND_LOCAL_LIST, [this](rapidjson::Document &req)
                                         { sendListCallHandler(req); });
            m_client.registerCallHandler(GET_LOCAL_LIST_VERSION, [this](rapidjson::Document &req)
                                         { getVersionCallHandler(req); });
        }

        void AuthorizeLocalList::getVersionCallHandler(rapidjson::Document &req)
        {
            if (m_connectors.registrationStatus() != RegistrationStatus::Accepted)
            {
                log_error("Registration not accepted");
                return;
            }
            std::string uniqueId = req[1].GetString();

            if (RegistrationStatus::Rejected == m_connectors.registrationStatus())
            {
                sendCallError(uniqueId, "GenericError", "Charge point is not connected");
                return;
            }

            rapidjson::Document ocppCfg;
            bool localAuthListEnabled = false;
            if (config::ConfigManager::getInstance().getConfig("OCPP1_6", ocppCfg) && ocppCfg.HasMember("LocalAuthListEnabled") &&
                ocppCfg["LocalAuthListEnabled"].IsBool())
            {

                localAuthListEnabled = ocppCfg["LocalAuthListEnabled"].GetBool();
            }
            else
            {
                localAuthListEnabled = false;
            }

            if (localAuthListEnabled)
            {
                sendCallResult(uniqueId, m_local_list_version);
            }
            else
            {
                sendCallError(uniqueId, "GenericError", "Charge point is not connected");
            }
        }

        void AuthorizeLocalList::sendListCallHandler(rapidjson::Document &req)
        {
            if (m_connectors.registrationStatus() != RegistrationStatus::Accepted)
            {
                log_error("Registration not accepted");
                return;
            }
            std::string uniqueId = req[1].GetString();
            const rapidjson::Value &payload = req[3];

            if (RegistrationStatus::Rejected == m_connectors.registrationStatus())
            {
                sendCallError(uniqueId, "GenericError", "Charge point is not connected");
                return;
            }

            bool localAuthListEnabled = false;
            rapidjson::Document config;
            if (config::ConfigManager::getInstance().getConfig("OCPP1_6", config))
            {
                if (config.HasMember("LocalAuthListEnabled") && config["LocalAuthListEnabled"].IsBool())
                    localAuthListEnabled = config["LocalAuthListEnabled"].GetBool();
            }

            if (!localAuthListEnabled)
            {
                log_warn("AuthorizeLocalList: LocalAuthList is not enabled");
                sendCallError(uniqueId, "NotImplemented", "LocalAuthListEnabled is not enabled");
                return;
            }

            int64_t listVersion = payload["listVersion"].IsInt() ? payload["listVersion"].GetInt64() : -1;
            if (listVersion < 0)
            {
                sendCallError(uniqueId, "GenericError", "listVersion < 0 is invalid");
                return;
            }

            // 判断 Full / Differential
            bool isFull = false;
            if (payload["updateType"].IsString())
            {
                std::string updateType = payload["updateType"].GetString();
                if (updateType == "Differential")
                {
                    isFull = false;
                }
                else if (updateType == "Full")
                {
                    isFull = true;
                }
                else
                {
                    log_error("Invalid updateType: %s", updateType.c_str());
                    sendCallError(uniqueId, "InvalidParams", "Invalid updateType");
                    return;
                }
            }
            log_debug("AuthorizeLocalList: Update type is %s", isFull ? "Full" : "Differential");

            if (!isFull) // Differential版本必须递增
            {
                if (listVersion <= m_local_list_version)
                {
                    sendCallResult(uniqueId, UpdateStatus::VersionMismatch);
                    return;
                }
            }

            log_debug("AuthorizeLocalList: Processing listVersion=%u, current local_list_version=%u", listVersion, m_local_list_version);

            rapidjson::Document emptyLocalList;
            emptyLocalList.SetArray();
            const rapidjson::Value *localAuthorizationListPtr = nullptr;

            if (payload.HasMember("localAuthorizationList"))
            {
                if (payload["localAuthorizationList"].IsArray())
                {
                    localAuthorizationListPtr = &payload["localAuthorizationList"];
                }
                else
                {
                    log_error("AuthorizeLocalList: localAuthorizationList is not an array");
                    sendCallError(uniqueId, "FormatViolation", "localAuthorizationList must be an array");
                    return;
                }
            }

            const rapidjson::Value &localAuthorizationList = localAuthorizationListPtr ? *localAuthorizationListPtr : emptyLocalList;
            log_debug("AuthorizeLocalList: Processing %zu authorization entries (empty list allowed)", localAuthorizationList.Size());

            // 执行更新
            bool result = isFull ? performFullUpdate(localAuthorizationList) : performPartialUpdate(localAuthorizationList);

            // 更新配置
            if (result)
            {
                rapidjson::Document versionDoc;
                versionDoc.SetInt64(listVersion);
                if (!config::ConfigManager::getInstance().setConfig("ChargePoint.LocalListVersion", versionDoc))
                {
                    sendCallResult(uniqueId, UpdateStatus::Failed);
                    return;
                }
                m_local_list_version = listVersion;
                log_debug("Updated local_list_version=%u", m_local_list_version);
                sendCallResult(uniqueId, UpdateStatus::Accepted);
            }
            else
            {
                sendCallResult(uniqueId, UpdateStatus::Failed);
            }
        }

        void AuthorizeLocalList::sendCallError(const std::string &uniqueId, const std::string &errorCode, const std::string &errorDescription)
        {
            rapidjson::Document errorDetails;
            errorDetails.SetObject();
            bool result = m_client.sendCallError(uniqueId, errorCode, errorDescription, errorDetails);
            log_info("AuthorizeLocalList: sendCallError: %s", result ? "success" : "fail");
        }

        void AuthorizeLocalList::sendCallResult(const std::string &uniqueId, UpdateStatus status)
        {
            rapidjson::Document payload;
            payload.SetObject();
            auto &alloc = payload.GetAllocator();

            rapidjson::Value statusVal;
            const std::string statusStr = updateStatusMap.toString(status, "Accepted");
            statusVal.SetString(statusStr.c_str(), static_cast<rapidjson::SizeType>(statusStr.length()), alloc);

            payload.AddMember("status", statusVal, alloc);

            bool result = m_client.sendCallResult(uniqueId, SEND_LOCAL_LIST, payload);
            log_info("AuthorizeLocalList: sendCallResult: %s", result ? "success" : "fail");
        }

        void AuthorizeLocalList::sendCallResult(const std::string &uniqueId, int32_t listVersion)
        {
            rapidjson::Document payload;
            payload.SetObject();
            auto &alloc = payload.GetAllocator();

            payload.AddMember("listVersion", listVersion, alloc);

            bool result = m_client.sendCallResult(uniqueId, GET_LOCAL_LIST_VERSION, payload);
            log_info("AuthorizeLocalList: sendCallResult: %s", result ? "success" : "fail");
        }

        bool AuthorizeLocalList::checkLocalAuth(const std::string idTag)
        {
            bool result = false;
            if (idTag.empty())
            {
                log_debug("AuthorizeLocalList: Empty idTag provided");
                result = false;
                return result;
            }

            log_debug("AuthorizeLocalList: Checking authorization for idTag=%s", idTag.c_str());

            if (m_find_query != nullptr)
            {
                m_find_query->reset();
                m_find_query->bind(1, idTag.c_str());
                if (m_find_query->exec())
                {
                    result = m_find_query->hasRows();
                    if (result)
                    {
                        bool expiredIsValid = !m_find_query->isNull(3);
                        std::time_t expired = m_find_query->getInt64(3);
                        if (expiredIsValid)
                        {
                            std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                            result = expired > now;
                            log_debug("AuthorizeLocalList: Tag %s expiry check - expired: %ld, now: %ld, valid: %s",
                                      idTag.c_str(),
                                      expired,
                                      now,
                                      result ? "yes" : "no");
                        }
                        else
                        {
                            result = false;
                            log_debug("AuthorizeLocalList: Tag %s has no valid expiry date", idTag.c_str());
                        }
                        if (result)
                        {
                            AuthorizationStatus status = static_cast<AuthorizationStatus>(m_find_query->getUInt32(4));
                            result = status == AuthorizationStatus::Accepted;
                            log_debug("AuthorizeLocalList: Tag %s status check - status: %d, accepted: %s",
                                      idTag.c_str(),
                                      static_cast<int>(status),
                                      result ? "yes" : "no");
                        }
                    }
                    else
                    {
                        log_debug("AuthorizeLocalList: Tag %s not found in local list", idTag.c_str());
                    }
                }
                else
                {
                    log_error("AuthorizeLocalList: Database query failed for tag %s: %s", idTag.c_str(), m_find_query->lastError().c_str());
                }
            }
            m_find_query->reset();
            log_debug("AuthorizeLocalList: Authorization result for tag %s: %s", idTag.c_str(), result ? "ACCEPTED" : "REJECTED");
            return result;
        }

        bool AuthorizeLocalList::initDatabaseTable()
        {
            // Create database table
            auto createQuery = m_database.query("CREATE TABLE IF NOT EXISTS AuthentLocalList ("
                                                "  [id] INTEGER PRIMARY KEY AUTOINCREMENT,"
                                                "  [tag] VARCHAR(20) UNIQUE,"
                                                "  [parent] VARCHAR(20),"
                                                "  [expiry] INTEGER,"
                                                "  [status] INTEGER"
                                                ");");

            if (!createQuery || !createQuery->exec())
            {
                log_error("Failed to create AuthentLocalList table: %s", createQuery ? createQuery->lastError().c_str() : "createQuery is null");
                return false;
            }

            // Prepare parameterized queries
            m_find_query = m_database.query("SELECT * FROM AuthentLocalList WHERE tag=?;");
            m_delete_query = m_database.query("DELETE FROM AuthentLocalList WHERE tag=?;");
            m_insert_query = m_database.query("INSERT INTO AuthentLocalList VALUES (NULL, ?, ?, ?, ?);");
            m_update_query = m_database.query("UPDATE AuthentLocalList SET [parent]=?, [expiry]=?, [status]=? WHERE tag=?;");

            // Validate all prepared statements
            if (!m_find_query)
            {
                log_error("Failed to prepare 'find' query");
                return false;
            }
            if (!m_delete_query)
            {
                log_error("Failed to prepare 'delete' query");
                return false;
            }
            if (!m_insert_query)
            {
                log_error("Failed to prepare 'insert' query");
                return false;
            }
            if (!m_update_query)
            {
                log_error("Failed to prepare 'update' query");
                return false;
            }

            // Load local list version from config
            rapidjson::Document jLocalListVersion;
            config::ConfigManager &configManager = config::ConfigManager::getInstance();

            if (configManager.getConfig("ChargePoint.LocalListVersion", jLocalListVersion) && jLocalListVersion.IsUint())
            {
                m_local_list_version = jLocalListVersion.GetUint();
            }
            else
            {
                m_local_list_version = 0; // default
            }

            return true;
        }

        bool AuthorizeLocalList::performFullUpdate(const rapidjson::Value &authorization_datas)
        {
            bool ret = true;
            config::ConfigManager &configManager = config::ConfigManager::getInstance();

            // 读取最大允许长度
            uint32_t LocalAuthListMaxLength = 500; // default

            rapidjson::Document cfg;
            if (configManager.getConfig("OCPP1_6.LocalAuthListMaxLength", cfg) && cfg.IsUint())
            {
                LocalAuthListMaxLength = cfg.GetUint();
            }

            if (!authorization_datas.IsArray())
            {
                log_error("AuthorizeLocalList: authorization_datas is not an array");
                return false;
            }

            if (authorization_datas.Size() > LocalAuthListMaxLength)
            {
                log_warn("AuthorizeLocalList: Authorization data size %u exceeds maximum %u", authorization_datas.Size(), LocalAuthListMaxLength);
                ret = false;
            }

            // 预解析所有数据
            std::vector<AuthorizationRecord> records;

            if (ret)
            {
                log_debug("AuthorizeLocalList: Processing %u entries for full update", authorization_datas.Size());

                for (auto &data : authorization_datas.GetArray())
                {
                    if (!data.HasMember("idTag") || !data["idTag"].IsString())
                        continue;

                    AuthorizationRecord record;
                    record.idTag = data["idTag"].GetString();

                    if (data.HasMember("idTagInfo") && data["idTagInfo"].IsObject())
                    {
                        const auto &idTagInfo = data["idTagInfo"];

                        if (idTagInfo.HasMember("status") && idTagInfo["status"].IsString())
                        {
                            std::string statusStr = idTagInfo["status"].GetString();
                            record.status = authorizationStatusMap.toEnum(statusStr, AuthorizationStatus::Invalid);
                        }

                        if (idTagInfo.HasMember("parentIdTag") && idTagInfo["parentIdTag"].IsString())
                        {
                            record.parentIdTag = idTagInfo["parentIdTag"].GetString();
                        }

                        if (idTagInfo.HasMember("expiryDate") && idTagInfo["expiryDate"].IsString())
                        {
                            Time::DateTime expiry;
                            expiry.assign(idTagInfo["expiryDate"].GetString());
                            record.expiryDate = expiry.timestamp();
                            log_debug("expiry: %s, status: %d", idTagInfo["expiryDate"].GetString(), record.status);
                        }
                    }

                    records.emplace_back(record);
                }
            }

            // 清空数据库
            if (ret)
            {
                auto query = m_database.query("DELETE FROM AuthentLocalList;");
                if (query)
                {
                    ret = query->exec();
                    if (!ret)
                        log_error("Could not clear authent local list table: %s", query->lastError().c_str());
                    else
                        log_debug("AuthorizeLocalList: Cleared existing local authorization list");
                }
            }

            // 插入数据库
            if (ret && m_insert_query)
            {
                log_debug("AuthorizeLocalList: Inserting %zu authorization entries", records.size());

                for (const auto &record : records)
                {
                    m_insert_query->reset();
                    m_insert_query->bind(1, record.idTag.c_str());
                    m_insert_query->bind(2, record.parentIdTag.c_str());
                    m_insert_query->bind(3, static_cast<uint64_t>(record.expiryDate));
                    m_insert_query->bind(4, static_cast<uint32_t>(record.status));

                    if (!m_insert_query->exec())
                    {
                        log_error("Could not insert idTag [%s]: %s", record.idTag.c_str(), m_insert_query->lastError().c_str());
                        ret = false;
                        break;
                    }
                    else
                    {
                        log_debug("IdTag [%s] inserted", record.idTag.c_str());
                    }
                }
            }

            if (m_insert_query)
                m_insert_query->reset();

            log_debug("AuthorizeLocalList: Full update %s", ret ? "completed successfully" : "failed");
            return ret;
        }

        bool AuthorizeLocalList::performPartialUpdate(const rapidjson::Value &authorization_datas)
        {
            bool ret = true;

            if (!authorization_datas.IsArray())
            {
                log_error("AuthorizeLocalList: authorization_datas is not an array");
                return false;
            }

            // 解析为记录结构
            std::vector<AuthorizationRecord> records;

            log_debug("AuthorizeLocalList: Processing %u entries for partial update", authorization_datas.Size());

            for (auto &data : authorization_datas.GetArray())
            {
                if (!data.HasMember("idTag") || !data["idTag"].IsString())
                    continue;

                AuthorizationRecord record;
                record.idTag = data["idTag"].GetString();

                if (data.HasMember("idTagInfo") && data["idTagInfo"].IsObject())
                {
                    const auto &info = data["idTagInfo"];

                    if (!info.HasMember("status") || !info["status"].IsString())
                        continue;

                    std::string statusStr = info["status"].GetString();
                    record.status = authorizationStatusMap.toEnum(statusStr, AuthorizationStatus::Invalid);

                    if (info.HasMember("parentIdTag") && info["parentIdTag"].IsString())
                        record.parentIdTag = info["parentIdTag"].GetString();

                    if (info.HasMember("expiryDate") && info["expiryDate"].IsString())
                    {
                        Time::DateTime expiry;
                        expiry.assign(info["expiryDate"].GetString());
                        record.expiryDate = expiry.timestamp();

                        log_debug("expiry: %s, status: %d", info["expiryDate"].GetString(), record.status);
                    }
                }

                records.emplace_back(record);
            }

            // 更新数据库
            log_debug("AuthorizeLocalList: Updating %zu authorization entries", records.size());

            for (const auto &record : records)
            {
                m_update_query->reset();
                m_update_query->bind(1, record.idTag.c_str());
                m_update_query->bind(2, record.parentIdTag.c_str());
                m_update_query->bind(3, static_cast<uint64_t>(record.expiryDate));
                m_update_query->bind(4, static_cast<uint32_t>(record.status));

                if (!m_update_query->exec())
                {
                    log_error("[AuthentLocalList] update error: %s", m_update_query->lastError().c_str());
                    ret = false;
                    break;
                }
                else
                {
                    log_debug("[AuthentLocalList] IdTag [%s] updated", record.idTag.c_str());
                }
            }

            if (m_update_query)
                m_update_query->reset();

            log_debug("AuthorizeLocalList: Partial update %s", ret ? "completed successfully" : "failed");
            return ret;
        }

    } // namespace auth
} // namespace ocpp1_6