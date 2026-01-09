#ifndef CONFIG_DEFAULTS_H
#define CONFIG_DEFAULTS_H

#include "ocpp_json_minimal.h"
#include <string>
#include <unordered_map>

#include "chargePointConfigurationKeynames.h"
#include "standardConfigurationKeyNames.h"

namespace ocpp1_6
{
    namespace config
    {
        /**
         * @brief 校验并补全配置（对外统一接口）
         *
         * 此函数会将传入的配置与默认配置进行对比，执行两个操作：
         *
         * 1. **校验（Validation）**
         *    - 检查是否缺失配置项
         *    - 检查类型是否与默认配置一致
         *    - 仅记录日志，不会中断流程
         *
         * 2. **补全（Merge / Auto-fix）**
         *    - 对于缺失的字段，自动补上默认值
         *    - 对于类型不匹配的字段，用默认值替换
         *    - 遵循默认配置结构进行递归补全
         *
         * @param[in,out] config 需要被校验和补全的配置文档（rapidjson::Document）
         *
         * @return true  - 合法且补全成功（或已修正）
         * @return false - 传入 JSON 非对象，或默认配置解析失败（致命错误）
         *
         * @note 本接口是业务层唯一需要调用的配置检查接口。
         *       内部会自动执行：校验 + 补全，无需外部额外处理。
         *
         * @note 补全后的配置将直接写入 config（原地修改）。
         */
        bool validateAndMergeConfig(rapidjson::Document &config);

        /**
         * @brief 根据 keyPath 在 JSON 文档中查找目标字段，并与给定值的类型进行比较。
         *
         * 此函数从 rapidjson::Document 的根节点开始，根据 keyPath 指定的路径逐层查找字段。
         * 若成功找到对应字段，则比较其 rapidjson::Type 类型是否与 target 的类型一致。
         *
         * @param keyPath  使用点号分隔的路径字符串，如 a.b.c。
         *                 - 每一段表示一个对象中的 key；
         *                 - 当前版本不支持数组下标（如 arr[1]），若需要可扩展；
         *                 - 若路径中任意一层不存在，此函数直接返回 false。
         *
         * @param doc      要查询的 rapidjson::Document 根节点。
         *
         * @param target   用于比较类型的 rapidjson::Value。
         *                 - 比较的是 rapidjson::Type（例如 kObjectType、kArrayType 等）；
         *                 - 若 target 是 parse 失败得到的 null 类型，类型比较亦会按 null 处理。
         *
         * @return true    当 keyPath 对应的字段存在，并且类型与 target 类型一致。
         * @return false   路径不存在，或存在但类型不同。
         *
         * @note 此函数只比较 rapidjson::Type，不比较具体值内容。
         * @note 若 keyPath 为空，函数返回 true
         */
        bool CompareJsonTypeByPath(const std::string &keyPath, const rapidjson::Document &doc, const rapidjson::Value &target);

        /**
         * @brief 只读配置项表（键 → 是否仅可读）
         *
         * 说明：
         *   true  = 配置项不可被修改（ReadOnly）
         *   false = 配置项可被修改
         */
        inline const std::unordered_map<std::string, bool> CONFIG_READONLY_KEYS = {{"AllowOfflineTxForUnknownId", false},
                                                                                   {"AuthorizationCacheEnabled", false},
                                                                                   {"AuthorizeRemoteTxRequests", true},
                                                                                   {"ClockAlignedDataInterval", true},
                                                                                   {"ConnectorPhaseRotation", true},
                                                                                   {"ConnectorPhaseRotationMaxLength", true},
                                                                                   {"GetConfigurationMaxKeys", false},
                                                                                   {"HeartbeatInterval", false},
                                                                                   {"LocalAuthorizeOffline", false},
                                                                                   {"LocalPreAuthorize", false},
                                                                                   {"MeterValuesAlignedData", true},
                                                                                   {"MeterValuesAlignedDataMaxLength", true},
                                                                                   {"MeterValueSampleInterval", false},
                                                                                   {"MeterValuesSampledData", false},
                                                                                   {"MeterValuesSampledDataMaxLength", true},
                                                                                   {"NumberOfConnectors", true},
                                                                                   {"ResetRetries", false},
                                                                                   {"StopTransactionOnEVSideDisconnect", true},
                                                                                   {"StopTransactionOnInvalidId", true},
                                                                                   {"StopTxnAlignedData", true},
                                                                                   {"StopTxnAlignedDataMaxLength", true},
                                                                                   {"StopTxnSampledData", true},
                                                                                   {"StopTxnSampledDataMaxLength", true},
                                                                                   {"SupportedFeatureProfiles", true},
                                                                                   {"SupportedFeatureProfilesMaxLength", true},
                                                                                   {"TransactionMessageAttempts", false},
                                                                                   {"TransactionMessageRetryInterval", false},
                                                                                   {"UnlockConnectorOnEVSideDisconnect", true},
                                                                                   {"WebSocketPingInterval", false},
                                                                                   {"LocalAuthListEnabled", false},
                                                                                   {"LocalAuthListMaxLength", true},
                                                                                   {"SendLocalListMaxLength", true},
                                                                                   {"ReserveConnectorZeroSupported", true},
                                                                                   {"ConnectionTimeOut", false},
                                                                                   {"SupportedFileTransferProtocols", true},
                                                                                   {"CpoName", true},
                                                                                   {"CertificateSignedMaxChainSize", true},
                                                                                   {"CertificateStoreMaxLength", true},
                                                                                   {"SecurityProfile", false},
                                                                                   {"AuthorizationKey", false},
                                                                                   {"AdditionalRootCertificateCheck", true}};

        inline const std::unordered_map<std::string, bool> CONFIG_NEED_RESTART = {
            {"WebSocketPingInterval", true},
            {"SecurityProfile", true},
            {"ChargePointIdentifier", true},
            {"ChargePoint.ChargePointSerialNumber", true},
            {"ChargePoint.ClientCertificateRequestEcCurve", true},
            {"ChargePoint.tSerialNumber", true},
            {"ChargePoint.tSerialNumber", true},
            {"ChargePoint.tSerialNumber", true},
            {"ChargePoint.tSerialNumber", true},
            {"ChargePoint.tSerialNumber", true},
            {"ChargePoint.tSerialNumber", true},
        };
        /**
         * @brief 默认配置（JSON 字符串形式）。
         *
         * 程序启动时使用：
         *   - 校验用户配置
         *   - 自动补全用户配置
         *   - 若删除关键字段，则根据此默认值恢复
         */
        inline const char *DEFAULT_CONFIG_JSON = R"(
    {
        "ChargePoint": {
            "CallRequestTimeout": 30,
            "ChargePointIdentifier": "DeYeL430",
            "ChargePointSerialNumber": "SN123456",
            "ClientCertificateRequestEcCurve": "P-256",
            "ClientCertificateRequestHashType": "SHA256",
            "ClientCertificateRequestKeyType": "RSA",
            "ClientCertificateRequestRsaKeyLength": 2048,
            "ClientCertificateRequestSubjectCountry": "CN",
            "ClientCertificateRequestSubjectEmail": "it@deye.com.cn",
            "ClientCertificateRequestSubjectLocation": "Ningbo",
            "ClientCertificateRequestSubjectOrganizationUnit": "OrganizationUnit",
            "ClientCertificateRequestSubjectState": "State",
            "ConnectionTimeout": 3000,
            "ConnectionUrl": "ws://172.30.1.88:8080/steve/websocket/CentralSystemService",
            "DatabasePath": "/usr/local/app/ocpp/chargepoint.db",
            "FirmwareVersion": "v1.0",
            "FirmwareSavePath": "/tmp",
            "SecurityLogSavePath":"/usr/local/app/ocpp",
            "Iccid": "111",
            "Imsi": "111",
            "JsonSchemasPath": "/usr/local/app/ocpp/schema/json",
            "MeterSerialNumber": "MeterSN123",
            "MeterType": "MainMeterType",
            "Model": "YourModel",
            "Vendor": "DeYe",
            "Protocol": "ocpp1.6",
            "OcppSoftwareVersion": "OCPP1.6_g00000",
            "RetryInterval": 2000,
            "TransactionMessageAttempts": 3,
            "LocalListVersion": 0,
            "SecurityProfile": 1,
            "Tlsv12CipherList": "ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384",
            "Tlsv13CipherList": "TLS_AES_128_GCM_SHA256:TLS_AES_256_GCM_SHA384",
            "TlsServerCertificateCa": "/usr/local/app/ocpp/open-ocpp_ca.crt",
            "TlsClientCertificate": "/usr/local/app/ocpp/open-ocpp_charge-point.crt",
            "TlsClientCertificatePrivateKey": "/usr/local/app/ocpp/open-ocpp_charge-point.key",
            "TlsClientCertificatePrivateKeyPassphrase": "",
            "InternalCertificateManagementEnabled": false,
            "TlsAllowSelfSignedCertificates": true,
            "TlsAllowExpiredCertificates": false,
            "TlsAcceptNonTrustedCertificates": false,
            "TlsSkipServerNameCheck": false,
            "SecurityEventNotificationEnabled": true,
            "SecurityLogMaxEntriesCount": 2000,
            "AuthentCacheMaxEntriesCount": 1000
        },
        "OCPP1_6": {
            "AllowOfflineTxForUnknownId": false,
            "AuthorizationCacheEnabled": false,
            "AuthorizeRemoteTxRequests": true,
            "ClockAlignedDataInterval": 3600,
            "ConnectorPhaseRotation": "NotApplicable",
            "GetConfigurationMaxKeys": 150,
            "HeartbeatInterval": 15,
            "LocalAuthListEnabled": false,
            "LocalAuthListMaxLength": 500,
            "LocalAuthorizeOffline": false,
            "LocalPreAuthorize": false,
            "MeterValueSampleInterval": 60,
            "MeterValuesAlignedData": "Energy.Active.Import.Register",
            "MeterValuesAlignedDataMaxLength": 64,
            "MeterValuesSampledData": "Energy.Active.Import.Register",
            "MeterValuesSampledDataMaxLength": 64,
            "NumberOfConnectors": 8,
            "ReserveConnectorZeroSupported": false,
            "ResetRetries": 2,
            "SendLocalListMaxLength": 0,
            "StopTransactionOnEVSideDisconnect": true,
            "StopTransactionOnInvalidId": false,
            "StopTxnAlignedData": "Energy.Active.Import.Register",
            "StopTxnAlignedDataMaxLength": 64,
            "StopTxnSampledData": "Energy.Active.Import.Register",
            "StopTxnSampledDataMaxLength": 64,
            "SupportedFeatureProfiles": "Core,FirmwareManagement,LocalAuthListManagement,Reservation,RemoteTrigger",
            "SupportedFeatureProfilesMaxLength": 128,
            "TransactionMessageAttempts": 0,
            "TransactionMessageRetryInterval": 60,
            "WebSocketPingInterval": 30,
            "ConnectionTimeOut": 180,
            "AdditionalRootCertificateCheck": false,
            "AuthorizationKey": "123123",
            "CertificateSignedMaxChainSize": 10000,
            "CertificateStoreMaxLength": 50,
            "CpoName": "DeYe OCPP1_6",
            "SupportedFileTransferProtocols": "FTP,FTPS,HTTP,HTTPS",
            "UnlockConnectorOnEVSideDisconnect":true
        }
    }
)";
    } // namespace config
} // namespace ocpp1_6

#endif // CONFIG_DEFAULTS_H
