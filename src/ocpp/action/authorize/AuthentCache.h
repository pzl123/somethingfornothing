#ifndef AUTHENTCACHE_H
#define AUTHENTCACHE_H

#include "ocpp/action/authorize/AuthorizeDef.h"
#include "ocpp/common/Time.h"
#include "ocpp/Connector/Connectors.h"
#include "ocpp/tool/database/DataBase.h"
#include "ocpp/type/OcppStatus.h"

namespace ocpp1_6
{
    namespace auth
    {
        class AuthentCache
        {
        public:
            AuthentCache(Database &database);

            ~AuthentCache();

            bool initDatabaseTable();

            bool updateAuthCache(AuthorizationRecord authorizezation);

            bool checkAuthCache(std::string idTag);

            void clearAuthCache();

        private:
            Database &m_database; /* 数据库对象 */
            std::unique_ptr<Database::Query> m_find_query;
            std::unique_ptr<Database::Query> m_delete_query;
            std::unique_ptr<Database::Query> m_insert_query;
            std::unique_ptr<Database::Query> m_update_query;
        };
    } // namespace auth
} // namespace ocpp1_6
#endif // AUTHENTCACHE_H