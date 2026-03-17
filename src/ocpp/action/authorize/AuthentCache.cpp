#include "AuthentCache.h"
#include "ocpp/config/ConfigManager.h"

#include "ocpp/log/ocpp_log.h"

namespace ocpp1_6
{
    namespace auth
    {
        AuthentCache::AuthentCache(Database &database) : m_database(database) {}
        AuthentCache::~AuthentCache()
        {
            clearAuthCache();
        }

        bool AuthentCache::initDatabaseTable()
        {
            // 创建主表
            auto createTableQuery = m_database.query("CREATE TABLE IF NOT EXISTS AuthentCache ("
                                                     "  tag VARCHAR(20) PRIMARY KEY,"
                                                     "  parent VARCHAR(20),"
                                                     "  expiry INTEGER,"
                                                     "  status INTEGER NOT NULL"
                                                     ");");

            if (!createTableQuery || !createTableQuery->exec())
            {
                log_error("Failed to create AuthentCache table: %s", createTableQuery ? createTableQuery->lastError().c_str() : "query is null");
                return false;
            }

            // 创建自动删除触发器（使用 rowid 保证按插入顺序淘汰）
            const uint32_t cacheMaxEntriesCount = 300;
            std::stringstream triggerQuery;
            triggerQuery << "CREATE TRIGGER IF NOT EXISTS delete_oldest_AuthentCache "
                            "AFTER INSERT ON AuthentCache "
                            "WHEN (SELECT COUNT(*) FROM AuthentCache) > "
                         << cacheMaxEntriesCount
                         << " BEGIN "
                            "DELETE FROM AuthentCache "
                            "WHERE rowid = (SELECT MIN(rowid) FROM AuthentCache); "
                            "END;";

            auto createTriggerQuery = m_database.query(triggerQuery.str());
            if (!createTriggerQuery || !createTriggerQuery->exec())
            {
                log_error("Failed to create AuthentCache trigger: %s",
                          createTriggerQuery ? createTriggerQuery->lastError().c_str() : "query is null");
                return false;
            }

            // 预编译查询语句
            m_find_query = m_database.query("SELECT tag, parent, expiry, status FROM AuthentCache WHERE tag=?;");
            m_delete_query = m_database.query("DELETE FROM AuthentCache WHERE tag=?;");
            m_insert_query = m_database.query("INSERT INTO AuthentCache(tag, parent, expiry, status) VALUES (?, ?, ?, ?);");
            m_update_query = m_database.query("UPDATE AuthentCache SET parent=?, expiry=?, status=? WHERE tag=?;");

            // 验证所有预编译查询
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

            return true;
        }

        bool AuthentCache::updateAuthCache(AuthorizationRecord authorizezation)
        {
            if (!m_find_query || !m_insert_query || !m_update_query)
            {
                log_error("find, insert or update query not initialized");
                return false;
            }
            m_find_query->reset();
            m_insert_query->reset();
            m_update_query->reset();

            // 查找 tag 是否存在
            m_find_query->bind(1, authorizezation.idTag);
            if (!m_find_query->exec())
            {
                log_error("Failed to execute find query: %s", m_find_query->lastError().c_str());
                return false;
            }

            // 状态修正
            // OCPP 1.6 §3.5.1: ConcurrentTx is a successful authorization.
            // For caching purposes, treat it as Accepted to simplify validation.
            AuthorizationStatus status = authorizezation.status;
            if (status == AuthorizationStatus::ConcurrentTx)
            {
                status = AuthorizationStatus::Accepted;
            }

            if (m_find_query->hasRows())
            {
                // 存在则更新
                log_info("update idTag[%s]", authorizezation.idTag.c_str());

                m_update_query->bind(1, authorizezation.parentIdTag);
                m_update_query->bind(2, static_cast<std::time_t>(authorizezation.expiryDate));
                m_update_query->bind(3, static_cast<int64_t>(status));
                m_update_query->bind(4, authorizezation.idTag);

                if (!m_update_query->exec())
                {
                    log_error("Could not update idTag [%s]: %s", authorizezation.idTag.c_str(), m_update_query->lastError().c_str());
                    return false;
                }
            }
            else
            {
                // 不存在则插入
                log_info("insert idTag[%s]", authorizezation.idTag.c_str());

                m_insert_query->bind(1, authorizezation.idTag);
                m_insert_query->bind(2, authorizezation.parentIdTag);
                m_insert_query->bind(3, static_cast<std::time_t>(authorizezation.expiryDate));
                m_insert_query->bind(4, static_cast<int64_t>(status));

                if (!m_insert_query->exec())
                {
                    log_error("Failed to insert authorization cache: %s", m_insert_query->lastError().c_str());
                    return false;
                }
            }

            return true;
        }

        bool AuthentCache::checkAuthCache(std::string idTag)
        {
            if (!m_find_query || !m_delete_query)
            {
                log_error("Find or delete query not initialized");
                return false;
            }
            m_find_query->reset();
            m_delete_query->reset();

            m_find_query->bind(1, idTag);
            if (!m_find_query->exec())
            {
                log_error("Failed to execute find query: %s", m_find_query->lastError().c_str());
                return false;
            }

            if (m_find_query->hasRows())
            {
                std::string tag = m_find_query->getString(0);
                std::string parent = m_find_query->getString(1);
                std::time_t expiry = static_cast<time_t>(m_find_query->getInt64(2));
                int64_t status = static_cast<int64_t>(m_find_query->getInt64(3));

                log_info("checkAuthCache: tag: %s, parent: %s, expiryDate: %lld, status: %lld",
                         tag.c_str(),
                         parent.c_str(),
                         static_cast<long long>(expiry),
                         static_cast<long long>(status));

                Time::DateTime expiryDate(expiry);

                log_info("checkAuthCache: tag: %s, parent: %s, expiryDate: %s, status: %u",
                         tag.c_str(),
                         parent.c_str(),
                         expiryDate.str().c_str(),
                         status);

                /* 还没过期 */
                if (expiryDate > Time::DateTime::now() && status == static_cast<uint32_t>(AuthorizationStatus::Accepted))
                {
                    log_info("Authorization cache valid for IDTag: %s", idTag.c_str());
                    m_find_query->reset();
                    return true;
                }
                else
                {
                    // 删除无效或过期的缓存

                    m_delete_query->bind(1, idTag);
                    if (!m_delete_query->exec())
                    {
                        log_error("Failed to delete expired authorization cache: %s", m_delete_query->lastError().c_str());
                    }
                }
            }

            m_find_query->reset(); // 确保 reset 即使没找到数据
            return false;
        }

        void AuthentCache::clearAuthCache()
        {
            auto query = m_database.query("DELETE FROM AuthentCache WHERE TRUE;");
            if (query.get())
            {
                query->exec();
            }
        }

    } // namespace auth
} // namespace ocpp1_6