#include "configManager.h"
#include "utils/utils.h"

#include "rapidjson/stringbuffer.h"

namespace ocpp1_6
{
    ConfigManager::ConfigManager(std::string path) : m_default_cfgPath(path), m_current_cfgPath("")
    {
        m_default_cfgPath = m_default_cfgPath + "/ocpp_default_config.json";
        d_log("config path: %s", m_default_cfgPath.c_str());
        m_restartOcppKeys = {
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
            "OCPP1_6.AllowOfflineTxForUnknownId"};

        m_restartWebSocketKeys = {
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
            "OCPP1_6.WebSocketPingInterval"};

        m_immediateEffectKeys = {
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
            "OCPP1_6.ConnectionTimeOut"};
    }

    ConfigManager::~ConfigManager()
    {
        clearCacheConfig(false);
    }

    bool ConfigManager::createDefaultConfig(const std::string &path)
    {
        /* 创建根节点 */
        rapidjson::Document defaultConfig;
        defaultConfig.SetObject();
        rapidjson::Document::AllocatorType &allocator = defaultConfig.GetAllocator();

        /* 创建子对象 */
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
        defaultConfig.AddMember("OCPP1_6", ocppConfig, allocator);
    };
}