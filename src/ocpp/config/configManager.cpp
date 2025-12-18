#include <iostream>
#include <unistd.h>
#include <mutex>
#include <sstream>

#include "configManager.h"
#include "standardConfigurationKeyNames.h"
#include "chargePointConfigurationKeynames.h"

#include "utils/utils.h"
#include "utils/file_utils.h"

#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/error/en.h"
#include "rapidjson/document.h"

#define DEFAULT_PATH "../config_profile/app/ocpp/"
#define MAX_BUF_LEN_KB 65536

namespace ocpp1_6
{
    ConfigManager::ConfigManager(std::string path) : m_defaultCfgPath(path), m_currentCfgPath("")
    {
        m_defaultCfgPath = m_defaultCfgPath + "ocpp_default_config.json";
        d_log("m_defaultCfgPath path: %s", m_defaultCfgPath.c_str());
        if (0 > access(m_defaultCfgPath.c_str(), F_OK))
        {
            d_log("ocpp_default_config not exit, create it");
            (void)createDefaultConfig(m_defaultCfgPath);
        }
        m_restartOcppKeys =
        {
            "ChargePoint.ChargePointIdentifier",
            "ChargePoint.ChargePointSerialNumber",
            "ChargePoint.MeterSerialNumber",
            "ChargePoint.MeterType",
            "ChargePoint.Protocol",
            "ChargePoint.SecurityProfile",
            "ChargePoint.JsonSchemasPath",
            "ChargePoint.DatabasePath",
            "ChargePoint.FirmwareVersion",
            "ChargePoint.Model",
            "ChargePoint.Vendor",
            "ChargePoint.Iccid",
            "ChargePoint.Imsi",
            "ChargePoint.ClientCertificateRequestEcCurve",
            "ChargePoint.ClientCertificateRequestHashType",
            "ChargePoint.ClientCertificateRequestKeyType",
            "ChargePoint.ClientCertificateRequestRsaKeyLength",
            "ChargePoint.ClientCertificateRequestSubjectCountry",
            "ChargePoint.ClientCertificateRequestSubjectEmail",
            "ChargePoint.ClientCertificateRequestSubjectLocation",
            "ChargePoint.ClientCertificateRequestSubjectOrganizationUnit",
            "ChargePoint.ClientCertificateRequestSubjectState",
            "ChargePoint.InternalCertificateManagementEnabled",
            "OCPP1_6.AuthorizationCacheEnabled",
            "OCPP1_6.AuthorizeRemoteTxRequests",
            "OCPP1_6.AllowOfflineTxForUnknownId"
        };

        m_restartWebSocketKeys =
        {
            "ChargePoint.ConnectionUrl",
            "ChargePoint.ConnectionTimeout",
            "ChargePoint.RetryInterval",
            "ChargePoint.CallRequestTimeout",
            "OCPP1_6.AuthorizationKey",
            "ChargePoint.Tlsv12CipherList",
            "ChargePoint.Tlsv13CipherList",
            "ChargePoint.TlsServerCertificateCa",
            "ChargePoint.TlsClientCertificate",
            "ChargePoint.TlsClientCertificatePrivateKey",
            "ChargePoint.TlsClientCertificatePrivateKeyPassphrase",
            "ChargePoint.TlsAllowSelfSignedCertificates",
            "ChargePoint.TlsAllowExpiredCertificates",
            "ChargePoint.TlsAcceptNonTrustedCertificates",
            "ChargePoint.TlsSkipServerNameCheck",
            "OCPP1_6.WebSocketPingInterval"
        };

        m_immediateEffectKeys =
        {
            "OCPP1_6.ClockAlignedDataInterval",
            "OCPP1_6.ConnectorPhaseRotation",
            "OCPP1_6.GetConfigurationMaxKeys",
            "OCPP1_6.HeartbeatInterval",
            "OCPP1_6.LocalAuthListEnabled",
            "OCPP1_6.LocalAuthListMaxLength",
            "OCPP1_6.LocalAuthorizeOffline",
            "OCPP1_6.LocalPreAuthorize",
            "OCPP1_6.MeterValueSampleInterval",
            "OCPP1_6.MeterValuesAlignedData",
            "OCPP1_6.MeterValuesAlignedDataMaxLength",
            "OCPP1_6.MeterValuesSampledData",
            "OCPP1_6.MeterValuesSampledDataMaxLength",
            "OCPP1_6.NumberOfConnectors",
            "OCPP1_6.ReserveConnectorZeroSupported",
            "OCPP1_6.ResetRetries",
            "OCPP1_6.SendLocalListMaxLength",
            "OCPP1_6.StopTransactionOnEVSideDisconnect",
            "OCPP1_6.StopTransactionOnInvalidId",
            "OCPP1_6.StopTxnAlignedData",
            "OCPP1_6.StopTxnAlignedDataMaxLength",
            "OCPP1_6.StopTxnSampledData",
            "OCPP1_6.StopTxnSampledDataMaxLength",
            "OCPP1_6.SupportedFeatureProfiles",
            "OCPP1_6.SupportedFeatureProfilesMaxLength",
            "OCPP1_6.TransactionMessageAttempts",
            "OCPP1_6.TransactionMessageRetryInterval",
            "OCPP1_6.ConnectionTimeOut"
        };
    }

    ConfigManager::~ConfigManager()
    {
    }

    void ConfigManager::initReadOnlyConfig()
    {
        /* ocpp1_6配置 */
        m_CfgOnlyRead[ocpp1_6::config::AllowOfflineTxForUnknownId] = false;
        m_CfgOnlyRead[ocpp1_6::config::AuthorizationCacheEnabled] = false;
        m_CfgOnlyRead[ocpp1_6::config::AuthorizeRemoteTxRequests] = true;
        // m_CfgOnlyRead[ocpp1_6::config::BlinkRepeat] = false;
        m_CfgOnlyRead[ocpp1_6::config::ClockAlignedDataInterval] = false;
        m_CfgOnlyRead[ocpp1_6::config::ConnectionTimeOut] = false;
        m_CfgOnlyRead[ocpp1_6::config::ConnectorPhaseRotation] = false;
        m_CfgOnlyRead[ocpp1_6::config::ConnectorPhaseRotationMaxLength] = true;
        m_CfgOnlyRead[ocpp1_6::config::GetConfigurationMaxKeys] = true;
        m_CfgOnlyRead[ocpp1_6::config::HeartbeatInterval] = false;
        // m_CfgOnlyRead[ocpp1_6::config::LightIntensity] = false;
        m_CfgOnlyRead[ocpp1_6::config::LocalAuthorizeOffline] = false;
        m_CfgOnlyRead[ocpp1_6::config::LocalPreAuthorize] = false;
        // m_CfgOnlyRead[ocpp1_6::config::MaxEnergyOnInvalidId] = false;
        m_CfgOnlyRead[ocpp1_6::config::MeterValuesAlignedData] = false;
        m_CfgOnlyRead[ocpp1_6::config::MeterValuesAlignedDataMaxLength] = true;
        m_CfgOnlyRead[ocpp1_6::config::MeterValuesSampledData] = false;
        m_CfgOnlyRead[ocpp1_6::config::MeterValuesSampledDataMaxLength] = true;
        m_CfgOnlyRead[ocpp1_6::config::MeterValueSampleInterval] = false;
        // m_CfgOnlyRead[ocpp1_6::config::MinimumStatusDuration] = false;
        m_CfgOnlyRead[ocpp1_6::config::NumberOfConnectors] = true;
        m_CfgOnlyRead[ocpp1_6::config::ResetRetries] = false;
        m_CfgOnlyRead[ocpp1_6::config::StopTransactionOnEVSideDisconnect] = false;
        m_CfgOnlyRead[ocpp1_6::config::StopTransactionOnInvalidId] = false;
        m_CfgOnlyRead[ocpp1_6::config::StopTxnAlignedData] = false;
        m_CfgOnlyRead[ocpp1_6::config::StopTxnAlignedDataMaxLength] = true;
        m_CfgOnlyRead[ocpp1_6::config::StopTxnSampledData] = false;
        m_CfgOnlyRead[ocpp1_6::config::StopTxnSampledDataMaxLength] = true;
        m_CfgOnlyRead[ocpp1_6::config::SupportedFeatureProfiles] = true;
        m_CfgOnlyRead[ocpp1_6::config::SupportedFeatureProfilesMaxLength] = true;
        m_CfgOnlyRead[ocpp1_6::config::TransactionMessageAttempts] = false;
        m_CfgOnlyRead[ocpp1_6::config::TransactionMessageRetryInterval] = false;
        m_CfgOnlyRead[ocpp1_6::config::UnlockConnectorOnEVSideDisconnect] = false;
        m_CfgOnlyRead[ocpp1_6::config::WebSocketPingInterval] = false;
        m_CfgOnlyRead[ocpp1_6::config::LocalAuthListEnabled] = false;
        m_CfgOnlyRead[ocpp1_6::config::LocalAuthListMaxLength] = true;
        m_CfgOnlyRead[ocpp1_6::config::SendLocalListMaxLength] = true;
        m_CfgOnlyRead[ocpp1_6::config::ReserveConnectorZeroSupported] = true;
        // m_CfgOnlyRead[ocpp1_6::config::ChargeProfileMaxStackLevel] = true;
        // m_CfgOnlyRead[ocpp1_6::config::ChargingScheduleAllowedChargingRateUnit] = true;
        // m_CfgOnlyRead[ocpp1_6::config::ChargingScheduleMaxPeriods] = true;
        // m_CfgOnlyRead[ocpp1_6::config::ConnectorSwitch3to1PhaseSupported] = true;
        // m_CfgOnlyRead[ocpp1_6::config::MaxChargingProfilesInstalled] = true;

        /* 拓展 配置 */
        m_CfgOnlyRead[ocpp1_6::config::AdditionalRootCertificateCheck] = true;
        m_CfgOnlyRead[ocpp1_6::config::AuthorizationKey] = false;
        m_CfgOnlyRead[ocpp1_6::config::CertificateSignedMaxChainSize] = true;
        m_CfgOnlyRead[ocpp1_6::config::CertificateStoreMaxLength] = true;
        m_CfgOnlyRead[ocpp1_6::config::CpoName] = false;
        m_CfgOnlyRead[ocpp1_6::config::SecurityProfile] = true;

        /* Firmware and Diagnostics File Transfer */
        m_CfgOnlyRead[ocpp1_6::config::SupportedFileTransferProtocols] = true;

    }

    bool ConfigManager::loadConfig()
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (false == file_exist(m_defaultCfgPath.c_str())) /* 默认配置不存在 */
        {
            e_log("ocpp default config file not exist:%s", m_defaultCfgPath.c_str());
            if (false == createDefaultConfig(m_defaultCfgPath))
            {
                /* 默认配置生成失败 */
                e_log("create defaul config error");
                return false;
            }
        }

        m_currentCfgPath = std::string (extract_directory(m_defaultCfgPath.c_str())) + "/ocpp_cache_config.json";
        d_log("cache config path:%s", m_currentCfgPath.c_str());
        if (false == file_exist(m_currentCfgPath.c_str()))
        {
            /* cache 文件不存在，默认从default复制一份 */
            if (false == file_copy(m_currentCfgPath.c_str(), m_defaultCfgPath.c_str()))
            {
                e_log("copy default ocpp config failed dst:%s, src:%s", m_currentCfgPath.c_str(), m_defaultCfgPath.c_str());
                return false;
            }
        }

        /* 加载cache 配置 */
        d_log("Loading configuration from: %s", m_currentCfgPath.c_str());
        FILE *fp = fopen(m_currentCfgPath.c_str(), "rb");
        if (nullptr == fp)
        {
            e_log("Failed to open config file: %s", m_currentCfgPath.c_str());
            return false;
        }
        char readBuffer[MAX_BUF_LEN_KB]; // 64KB 缓冲区
        rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
        if (m_cacheConfig.ParseStream(is).HasParseError())
        {
            e_log("JSON parse error at offset %zu: %s", m_cacheConfig.GetErrorOffset(),GetParseError_En(m_cacheConfig.GetParseError()));
            fclose(fp);
            return false;
        }
        fclose(fp);
        return true;

    }

    bool ConfigManager::createDefaultConfig(const std::string &path)
    {

        /* 创建根节点 */
        rapidjson::Document defaultConfig;
        defaultConfig.SetObject();
        rapidjson::Document::AllocatorType &allocator = defaultConfig.GetAllocator();

        /* 创建子对象 */
        {
            rapidjson::Value ocppConfig(rapidjson::kObjectType);
            /* core feature profile */
            ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::AllowOfflineTxForUnknownId), false, allocator);
            ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::AuthorizationCacheEnabled), false, allocator);
            ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::AuthorizeRemoteTxRequests), true, allocator);
            // ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::BlinkRepeat), 0, allocator);
            ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::ClockAlignedDataInterval), 900, allocator);
            ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::ConnectionTimeOut), 300, allocator);
            ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::ConnectorPhaseRotation), "NotApplicable", allocator);
            ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::ConnectorPhaseRotationMaxLength), 1, allocator);
            ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::GetConfigurationMaxKeys), 150, allocator);
            ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::HeartbeatInterval), 15, allocator);
            // ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::LightIntensity), 100, allocator);
            ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::LocalAuthorizeOffline), false, allocator);
            ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::LocalPreAuthorize), false, allocator);
            // ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::MaxEnergyOnInvalidId), 0, allocator);
            ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::MeterValuesAlignedData), "Energy.Active.Import.Register", allocator);
            ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::MeterValuesAlignedDataMaxLength), 64, allocator);
            ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::MeterValuesSampledData), "Energy.Active.Import.Register", allocator);
            ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::MeterValuesSampledDataMaxLength), 64, allocator);
            ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::MeterValueSampleInterval), 60, allocator);
            // ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::MinimumStatusDuration), 60, allocator);
            ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::NumberOfConnectors), 8, allocator);
            ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::ResetRetries), 2, allocator);
            ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::StopTransactionOnEVSideDisconnect), true, allocator);
            ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::StopTransactionOnInvalidId), false, allocator);
            ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::StopTxnAlignedData), "Energy.Active.Import.Register", allocator);
            ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::StopTxnAlignedDataMaxLength), 64, allocator);
            ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::StopTxnSampledData), "Energy.Active.Import.Register", allocator);
            ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::StopTxnSampledDataMaxLength), 64, allocator);
            ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::SupportedFeatureProfiles), "Core,FirmwareManagement,LocalAuthListManagement,Reservation,RemoteTrigger", allocator);
            ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::SupportedFeatureProfilesMaxLength), 128, allocator);
            ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::TransactionMessageAttempts), 0, allocator);
            ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::TransactionMessageRetryInterval), 60, allocator);
            ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::UnlockConnectorOnEVSideDisconnect), true, allocator);
            ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::WebSocketPingInterval), 60000, allocator);

            /* Local Auth List Management Profile */
            ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::LocalAuthListEnabled), false, allocator);
            ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::LocalAuthListMaxLength), 5000, allocator);
            ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::SendLocalListMaxLength), 100, allocator);

            /* Reservation Profile */
            ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::ReserveConnectorZeroSupported), false, allocator);

            /*  Smart Charging Profile */
            // ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::ChargeProfileMaxStackLevel), true, allocator);
            // ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::ChargingScheduleAllowedChargingRateUnit), "Current", allocator);
            // ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::ChargingScheduleMaxPeriods), 0, allocator);
            // ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::ConnectorSwitch3to1PhaseSupported), true, allocator);
            // ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::MaxChargingProfilesInstalled), 0, allocator);

            /* 拓展安全配置项 */
            // ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::AdditionalRootCertificateCheck), false, allocator);
            ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::AuthorizationKey), "1234", allocator);
            ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::CertificateSignedMaxChainSize), 10000, allocator);
            ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::CertificateStoreMaxLength), 50, allocator);
            ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::CpoName), "DeYe OCPP1_6", allocator);
            ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::SecurityProfile), 1, allocator);

            /* Firmware and Diagnostics File Transfer */
            ocppConfig.AddMember(rapidjson::StringRef(ocpp1_6::config::SupportedFileTransferProtocols), "FTP,FTPS,HTTP,HTTPS", allocator);
            defaultConfig.AddMember("OCPP1_6", ocppConfig, allocator);
        }

        /* 创建子对象 */
        {
            rapidjson::Value ChargePointConfig(rapidjson::kObjectType);
            ChargePointConfig.AddMember(rapidjson::StringRef(CallRequestTimeout), 30, allocator);
            ChargePointConfig.AddMember(rapidjson::StringRef(ChargePointIdentifier), "DeYe_L430", allocator);
            ChargePointConfig.AddMember(rapidjson::StringRef(ChargePointSerialNumber), "SN123456", allocator);
            ChargePointConfig.AddMember(rapidjson::StringRef(ClientCertificateRequestEcCurve), "P-256", allocator);
            ChargePointConfig.AddMember(rapidjson::StringRef(ClientCertificateRequestHashType), "SHA256", allocator);
            ChargePointConfig.AddMember(rapidjson::StringRef(ClientCertificateRequestKeyType), "RSA", allocator);
            ChargePointConfig.AddMember(rapidjson::StringRef(ClientCertificateRequestRsaKeyLength), 2048, allocator);
            ChargePointConfig.AddMember(rapidjson::StringRef(ClientCertificateRequestSubjectCountry), "CN", allocator);
            ChargePointConfig.AddMember(rapidjson::StringRef(ClientCertificateRequestSubjectEmail), "it@deye.com.cn", allocator);
            ChargePointConfig.AddMember(rapidjson::StringRef(ClientCertificateRequestSubjectLocation), "Ningbo", allocator);
            ChargePointConfig.AddMember(rapidjson::StringRef(ClientCertificateRequestSubjectOrganizationUnit), "Energy storage", allocator);
            ChargePointConfig.AddMember(rapidjson::StringRef(ClientCertificateRequestSubjectState), "Zhejiang", allocator);
            ChargePointConfig.AddMember(rapidjson::StringRef(ClientCertificateRequestSubjectOrganization), "Deye Group", allocator);
            ChargePointConfig.AddMember(rapidjson::StringRef(ConnectionTimeout), 3000, allocator);
            ChargePointConfig.AddMember(rapidjson::StringRef(ConnectionUrl), "ws://172.30.1.5:8080/steve/websocket/CentralSystemService", allocator);

            std::string dbPath = std::string(DEFAULT_PATH) + "chargepoint.db";
            ChargePointConfig.AddMember(rapidjson::StringRef(DatabasePath), rapidjson::Value(dbPath.c_str(), static_cast<rapidjson::SizeType>(dbPath.length()), allocator).Move(), allocator);
            ChargePointConfig.AddMember(rapidjson::StringRef(FirmwareVersion), "v1.0", allocator);
            ChargePointConfig.AddMember(rapidjson::StringRef(Iccid), "*", allocator);
            ChargePointConfig.AddMember(rapidjson::StringRef(Imsi), "*", allocator);

            std::string jsPath = std::string(DEFAULT_PATH) + "schema";
            ChargePointConfig.AddMember(rapidjson::StringRef(JsonSchemasPath), rapidjson::Value(jsPath.c_str(), static_cast<rapidjson::SizeType>(jsPath.length()), allocator).Move(), allocator);

            ChargePointConfig.AddMember(rapidjson::StringRef(MeterSerialNumber), "-", allocator);
            ChargePointConfig.AddMember(rapidjson::StringRef(MeterType), "DC", allocator);
            ChargePointConfig.AddMember(rapidjson::StringRef(Model), "L430", allocator);
            ChargePointConfig.AddMember(rapidjson::StringRef(Vendor), "DeYe", allocator);
            ChargePointConfig.AddMember(rapidjson::StringRef(Protocol), "ocpp1.6", allocator);
            ChargePointConfig.AddMember(rapidjson::StringRef(RetryInterval), 60000, allocator);
            ChargePointConfig.AddMember(rapidjson::StringRef(TransactionMessageAttempts), 3, allocator);
            ChargePointConfig.AddMember(rapidjson::StringRef(Tlsv12CipherList), "ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:DHE-RSA-AES256-GCM-SHA384:PSK-AES256-GCM-SHA384:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256:DHE-RSA-AES128-GCM-SHA256:PSK-AES128-GCM-SHA256", allocator);
            ChargePointConfig.AddMember(rapidjson::StringRef(Tlsv13CipherList), "TLS_AES_128_GCM_SHA256:TLS_AES_256_GCM_SHA384", allocator);

            std::string TlsServerCertificateCaPath = std::string(DEFAULT_PATH) + "open-ocpp_ca.crt";
            ChargePointConfig.AddMember(rapidjson::StringRef(TlsServerCertificateCa), rapidjson::Value(TlsServerCertificateCaPath.c_str(), static_cast<rapidjson::SizeType>(TlsServerCertificateCaPath.length()), allocator).Move(), allocator);

            std::string TlsClientCertificatePath = std::string(DEFAULT_PATH) + "open-ocpp_charge-point.crt";
            ChargePointConfig.AddMember(rapidjson::StringRef(TlsClientCertificate), rapidjson::Value(TlsClientCertificatePath.c_str(), static_cast<rapidjson::SizeType>(TlsClientCertificatePath.length()), allocator).Move(), allocator);

            std::string TlsClientCertificatePrivateKeyPath = std::string(DEFAULT_PATH) + "open-ocpp_charge-point.key";
            ChargePointConfig.AddMember(rapidjson::StringRef(TlsClientCertificatePrivateKey), rapidjson::Value(TlsClientCertificatePrivateKeyPath.c_str(), static_cast<rapidjson::SizeType>(TlsClientCertificatePrivateKeyPath.length()), allocator).Move(), allocator);

            ChargePointConfig.AddMember(rapidjson::StringRef(TlsClientCertificatePrivateKeyPassphrase), "", allocator);
            ChargePointConfig.AddMember(rapidjson::StringRef(InternalCertificateManagementEnabled), false, allocator);
            ChargePointConfig.AddMember(rapidjson::StringRef(TlsAllowSelfSignedCertificates), true, allocator);
            ChargePointConfig.AddMember(rapidjson::StringRef(TlsAllowExpiredCertificates), false, allocator);
            ChargePointConfig.AddMember(rapidjson::StringRef(TlsAcceptNonTrustedCertificates), false, allocator);
            ChargePointConfig.AddMember(rapidjson::StringRef(TlsSkipServerNameCheck), false, allocator);
            ChargePointConfig.AddMember(rapidjson::StringRef(AuthentCacheMaxEntriesCount), 1000, allocator);
            ChargePointConfig.AddMember(rapidjson::StringRef(LogMaxEntriesCount), 2000, allocator);
            ChargePointConfig.AddMember(rapidjson::StringRef(SecurityLogMaxEntriesCount), 1000, allocator);
            ChargePointConfig.AddMember(rapidjson::StringRef(SecurityProfile), 1, allocator);
            defaultConfig.AddMember("ChargePoint", ChargePointConfig, allocator);
        }

        if (false == file_exist(path.c_str()))
        {
            char *dir_path = extract_directory(path.c_str());
            d_log("dir path:%s", dir_path);
            (void)make_dir_recursive(dir_path);
        }

        std::ofstream ofs(path);
        if (false == ofs.is_open())
        {
            e_log("Failed to open config file for writing: %s", path.c_str());
            return false;
        }

        rapidjson::StringBuffer buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
        defaultConfig.Accept(writer);

        ofs << buffer.GetString();
        ofs.close();

        return true;
    };

    bool ConfigManager::getConfig(const std::string &name, rapidjson::Value &value)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        rapidjson::Value *current = &m_cacheConfig;
        std::istringstream iss(name);
        std::string token;

        while (std::getline(iss, token, '.'))
        {
            if (!current->IsObject() || !current->HasMember(token.c_str()))
            {
                e_log("Failed to get config: %s", name.c_str());
                return false;
            }
            current = &(*current)[token.c_str()];
        }
        value = *current;
        return true;
    }

    bool ConfigManager::SetConfig(const std::string &name, const rapidjson::Value &value)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        rapidjson::Value oldCacheConfig;
        oldCacheConfig.CopyFrom(m_cacheConfig, m_cacheConfig.GetAllocator());
        std::istringstream iss(name);
        std::string token;
        std::vector<std::string> tokens;

        while (std::getline(iss, token, '.'))
        {
            tokens.push_back(token);
        }
        if (true == tokens.empty())
        {
            e_log("tokens empty, error name:%s", name.c_str());
            return false;
        }

        rapidjson::Value *current = &m_cacheConfig;
        for (size_t i = 0; i < tokens.size() - 1; ++i)
        {
            const char *key = tokens[i].c_str();
            if ((false == current->IsObject()) || (false == current->HasMember(key)))
            {
                e_log("Failed to set config: missing intermediate key '%s' in path '%s'", key, name.c_str());
                return false;
            }
            current = &(*current)[key];
        }
        const char *finalKey = tokens.back().c_str();
        if (false == current->IsObject())
        {
            e_log("Failed to set config: parent is not an object in path '%s'", name.c_str());
            return false;
        }

        if(current->HasMember(finalKey))
        {
            current->RemoveMember(finalKey);
        }

        rapidjson::Value newValue;
        newValue.CopyFrom(value, m_cacheConfig.GetAllocator());
        current->AddMember(rapidjson::StringRef(finalKey), newValue, m_cacheConfig.GetAllocator());

        if (false == saveCacheConfig());
        {
            e_log("save cache config failed");
            return false;
        }

        std::vector<std::string> changeKeys;
        findChangedKeys(oldCacheConfig, m_cacheConfig, "", changeKeys);

        m_configActionLevel = evaluateConfigActionLevel(changeKeys);
        switch (m_configActionLevel)
        {
        case ERROR_KEY:
            e_log("error %u", m_configActionLevel);
            break;
        case IMMEDIATE:
            d_log(" IMMEDIATE ");
            break;
        case RESETWEBSOCKET:
            d_log(" RESETWEBSOCKET ");
            break;
        case RESETOCPP:
            d_log(" RESETOCPP ");
            break;
        default:
            e_log("error %u", m_configActionLevel);
            break;
        }


        return true;
    }

    bool ConfigManager::saveCacheConfig()
    {
        std::ofstream ofs(m_currentCfgPath);
        if (false == ofs.is_open())
        {
            e_log("Failed to open config file for writing: %s", m_currentCfgPath.c_str());
            return false;
        }
        rapidjson::StringBuffer buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
        m_cacheConfig.Accept(writer);

        ofs << buffer.GetString();
        ofs.close();
    }

    void ConfigManager::findChangedKeys(const rapidjson::Value &oldVal, const rapidjson::Value &newVal, const std::string &path, std::vector<std::string> &changedKeys)
    {
        if (oldVal.GetType() != newVal.GetType())
        {
            changedKeys.push_back(path);
        }

        if (newVal.IsObject())
        {
            /* new有 old无 */
            for (auto it = newVal.MemberBegin(); it != newVal.MemberEnd(); it++)
            {
                const char *key = it->name.GetString();
                std::string childPath = path.empty() ? key : path + "." + key;

                if (false == oldVal.HasMember(key))
                {
                    changedKeys.push_back(childPath);
                }
                else
                {
                    findChangedKeys(oldVal[key], it->value, childPath, changedKeys);
                }
            }

            /* new无 old有 */
            for (auto it = oldVal.MemberBegin(); it != oldVal.MemberEnd(); ++it)
            {
                const char* key = it->name.GetString();
                if (!newVal.HasMember(key))
                {
                    std::string childPath = path.empty() ? std::string(key) : (path + "." + key);
                    changedKeys.push_back(childPath);
                }
            }
        }
        else if (newVal.IsArray())
        {
            if (!oldVal.IsArray())
            {
                changedKeys.push_back(path);
                return;
            }

            size_t newSize = newVal.Size();
            size_t oldSize = oldVal.Size();
            size_t miniSize = (newSize > oldSize) ? newSize : oldSize;

            /* 比较公共部分 */
            for (size_t i = 0; i < miniSize; i++)
            {
                std::string childPath = path + "[" + std::to_string(i) + "]";
                findChangedKeys(oldVal[i], newVal[i], childPath, changedKeys);
            }

            /* 整个数据不相等，是同变化 */
            if (newSize != oldSize)
            {
                changedKeys.push_back(path);
            }
        }
        else /* 基本类型 null bool number string */
        {
            bool equal = false;
            switch (newVal.GetType())
            {
            case rapidjson::kNullType:
                equal = (rapidjson::kNullType == oldVal.GetType()) ? true : false;
                break;

            case rapidjson::kTrueType:
            case rapidjson::kFalseType:
                equal = (oldVal.GetBool() == newVal.GetBool());
                break;

            case rapidjson::kNumberType:
            {
                /* 注意：RapidJSON 不区分 int/float，需根据实际存储方式比较 */
                if (oldVal.IsDouble() || newVal.IsDouble())
                {
                    equal = (oldVal.GetDouble() == newVal.GetDouble());
                }
                else if (oldVal.IsInt64() || newVal.IsInt64())
                {
                    equal = (oldVal.GetInt64() == newVal.GetInt64());
                }
                else
                {
                    equal = (oldVal.GetInt() == newVal.GetInt());
                }
                break;
            }

            case rapidjson::kStringType:
                equal = (strcmp(oldVal.GetString(), newVal.GetString()) == 0);
                break;

            default:
                equal = false;
            }
            if (false == equal)
            {
                changedKeys.push_back(path);
            }
        }
    }

    ActionLevel_e ConfigManager::evaluateConfigActionLevel(const std::vector<std::string>& keys)
    {
        bool needRestartOcpp = false;
        bool needReconnect = false;
        bool allRecognized = true;

        for (const auto key : keys)
        {
            if (m_restartOcppKeys.count(key))
            {
                return RESETOCPP;
            }
            else if (m_restartWebSocketKeys.count(key))
            {
                return RESETWEBSOCKET;
            }
            else if (m_immediateEffectKeys.count(key))
            {
                return IMMEDIATE;
            }
            else
            {
                e_log("Unknown config key for action: %s", key.c_str());
                return ERROR_KEY;
            }
        }
    }

    bool ConfigManager::isReadOnly(const std::string& key, bool& bread)
    {

    }
}