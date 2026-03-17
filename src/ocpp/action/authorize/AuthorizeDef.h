#ifndef AUTHORIZE_DEF_H
#define AUTHORIZE_DEF_H

#include "ocpp/type/OcppStatus.h"
#include <string>

namespace ocpp1_6
{
    namespace auth
    {
        struct AuthorizationRecord
        {
            std::string idTag;                                         // idTag
            std::string parentIdTag;                                   // parentIdTag，可空
            uint64_t expiryDate = 0;                                   // 过期时间，Unix timestamp
            AuthorizationStatus status = AuthorizationStatus::Invalid; // 鉴权状态

            AuthorizationRecord() = default;

            AuthorizationRecord(const std::string &idTag, const std::string &parentIdTag, uint64_t expiryDate, AuthorizationStatus status)
                : idTag(idTag), parentIdTag(parentIdTag), expiryDate(expiryDate), status(status)
            {
            }
        };

        struct AuthorizationConfig
        {
            bool allowOfflineTxForUnknownId;
            bool authorizationCacheEnabled;
            bool localAuthorizeOffline;
            bool localPreAuthorize;
            bool localAuthListEnabled;

            AuthorizationConfig()
                : allowOfflineTxForUnknownId(false),
                  authorizationCacheEnabled(false),
                  localAuthorizeOffline(false),
                  localPreAuthorize(false),
                  localAuthListEnabled(false)
            {
            }
        };

    } // namespace auth
} // namespace ocpp1_6
#endif