#ifndef AUTHENT_LOCAL_LIST_H
#define AUTHENT_LOCAL_LIST_H

#include "ocpp/action/authorize/AuthorizeDef.h"
#include "ocpp/client/OCPPClient.h"
#include "ocpp/common/EnumStrMappings.h"
#include "ocpp/common/Time.h"
#include "ocpp/config/ConfigManager.h"
#include "ocpp/Connector/Connectors.h"
#include "ocpp_json_minimal.h"
#include "ocpp/tool/database/DataBase.h"
#include "ocpp/type/OcppStatus.h"

namespace ocpp1_6
{
  namespace auth
  {
    /**
     * @brief AuthorizeLocalList类处理与本地列表鉴权相关的操作，包括发送本地列表鉴权请求和接收响应。
     */
    class AuthorizeLocalList
    {
    public:
      AuthorizeLocalList(client::IOCPPClient &client, Database &database, chargepoint::Connectors &connectors);

      ~AuthorizeLocalList();

      bool initDatabaseTable();

      void registHandler();

      void sendListCallHandler(rapidjson::Document &req);

      void getVersionCallHandler(rapidjson::Document &req);

      void sendCallError(const std::string &uniqueId, const std::string &errorCode, const std::string &errorDescription);

      void sendCallResult(const std::string &uniqueId, const UpdateStatus status);

      void sendCallResult(const std::string &uniqueId, const int32_t listVersion);

      bool checkLocalAuth(const std::string idTag);

    private:
      client::IOCPPClient &m_client;
      Database &m_database;
      chargepoint::Connectors &m_connectors;

      int64_t m_local_list_version;

      std::unique_ptr<Database::Query> m_find_query;
      std::unique_ptr<Database::Query> m_delete_query;
      std::unique_ptr<Database::Query> m_insert_query;
      std::unique_ptr<Database::Query> m_update_query;

      bool performFullUpdate(const rapidjson::Value &authorization_datas);

      bool performPartialUpdate(const rapidjson::Value &authorization_datas);
    };

  } // namespace auth
} // namespace ocpp1_6

#endif // AUTHENT_LOCAL_LIST_H