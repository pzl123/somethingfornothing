#ifndef STANDARD_CONFIGURATION_KEY_NAMES_H
#define STANDARD_CONFIGURATION_KEY_NAMES_H

namespace ocpp1_6
{
    namespace config
    {

        // =============================================================================
        // 🔹 Core Profile (Required)
        // =============================================================================

        // //  [bool] [optional] [RW] Whether to allow transactions for unknown IdTags when offline.
        // constexpr const char *AllowOfflineTxForUnknownId = "AllowOfflineTxForUnknownId";

        // //  [bool] [optional] [RW] Whether the Charge Point uses an authorization cache.
        // constexpr const char *AuthorizationCacheEnabled = "AuthorizationCacheEnabled";

        // //  [bool] [required] [R or RW.] Whether remote start requests require authorization.
        // constexpr const char *AuthorizeRemoteTxRequests = "AuthorizeRemoteTxRequests";

        // //  [integer] [optional] [RW] [Unit times] Number of times to blink Charge Point lighting when signalling
        // constexpr const char *BlinkRepeat = "BlinkRepeat";

        // //  [int] [required] [RW] [Unit seconds] Interval (sec) for clock-aligned data transfer. 0 = disabled.
        // constexpr const char *ClockAlignedDataInterval = "ClockAlignedDataInterval";

        // //  [int] [required] [RW] [Unit seconds] Time (sec) to wait for EV connection after plug-in.
        // constexpr const char * ConnectionTimeOut = "ConnectionTimeOut";

        // //  [CSL] [required] [RW] Phase rotation per connector (e.g., "NotApplicable,RST,RST").
        // constexpr const char *ConnectorPhaseRotation = "ConnectorPhaseRotation";

        // //  [int] [optional] [R] Max length of ConnectorPhaseRotation string (read-only).
        // constexpr const char *ConnectorPhaseRotationMaxLength = "ConnectorPhaseRotationMaxLength";

        // //  [int] [required] [R] Max number of keys in GetConfiguration request (read-only).
        // constexpr const char *GetConfigurationMaxKeys = "GetConfigurationMaxKeys";

        // //  [int] [required] [RW] [Unit seconds] Interval (sec) between Heartbeat messages.
        // constexpr const char *HeartbeatInterval = "HeartbeatInterval";

        // //  [int] [optional] [RW] Current light intensity (if supported).
        // constexpr const char *LightIntensity = "LightIntensity";

        // //  [bool] [required] [RW] Use local auth list when offline.
        // constexpr const char *LocalAuthorizeOffline = "LocalAuthorizeOffline";

        // //  [bool] [required] [RW] Pre-authorize using local list before online check.
        // constexpr const char *LocalPreAuthorize = "LocalPreAuthorize";

        // //  [int] [optional] [RW] [Unit Wh] Max energy (Wh) allowed for invalid/unknown IdTag.
        // constexpr const char *MaxEnergyOnInvalidId = "MaxEnergyOnInvalidId";

        // //  [CSL] [required] [RW] Comma-separated measurands for aligned data (e.g., "Power.Active.Import").
        // constexpr const char *MeterValuesAlignedData = "MeterValuesAlignedData";

        // //  [int] [optional] [R] Max length of MeterValuesAlignedData (read-only).
        // constexpr const char *MeterValuesAlignedDataMaxLength = "MeterValuesAlignedDataMaxLength";

        // //  [CSL] [required] [RW] Comma-separated measurands for sampled data.
        // constexpr const char *MeterValuesSampledData = "MeterValuesSampledData";

        // //  [int] [optional] [R] Max length of MeterValuesSampledData (read-only).
        // constexpr const char *MeterValuesSampledDataMaxLength = "MeterValuesSampledDataMaxLength";

        // //  [int] [required] [RW] [Unit seconds] Interval (sec) between meter value samples.
        // constexpr const char *MeterValueSampleInterval = "MeterValueSampleInterval";

        // //  [int] [optional] [RW] [Unit seconds] Min time (sec) a status must persist before sending StatusNotification.
        // constexpr const char *MinimumStatusDuration = "MinimumStatusDuration";

        // //  [int] [required] [R] Number of connectors (read-only).
        // constexpr const char *NumberOfConnectors = "NumberOfConnectors";

        // //  [int] [required] [R] [Unit times] Number of times to retry reset before giving up.
        // constexpr const char *ResetRetries = "ResetRetries";

        // //  [bool][required] [RW] Stop transaction when EV disconnects.
        // constexpr const char *StopTransactionOnEVSideDisconnect = "StopTransactionOnEVSideDisconnect";

        // //  [bool] [required] [RW] Stop transaction if invalid IdTag presented during active tx.
        // constexpr const char *StopTransactionOnInvalidId = "StopTransactionOnInvalidId";

        // //  [CSL] [required] [RW]  Measurands to include in StopTransaction message (aligned).
        // constexpr const char *StopTxnAlignedData = "StopTxnAlignedData";

        // //  [int] [optional] [R] Max length (read-only).
        // constexpr const char *StopTxnAlignedDataMaxLength = "StopTxnAlignedDataMaxLength";

        // //  [CSL] [required] [RW] Measurands to include in StopTransaction message (sampled).
        // constexpr const char *StopTxnSampledData = "StopTxnSampledData";

        // //  [int] Max length (read-only).
        // constexpr const char *StopTxnSampledDataMaxLength = "StopTxnSampledDataMaxLength";

        // //  [CSL] [required] [R] Comma-separated list of supported profiles (read-only).
        // constexpr const char *SupportedFeatureProfiles = "SupportedFeatureProfiles";

        // //  [int] Max length (read-only).
        // constexpr const char *SupportedFeatureProfilesMaxLength = "SupportedFeatureProfilesMaxLength";

        // //  [int] [required] [RW] [Unit times] Number of attempts to send a transaction-related message.
        // constexpr const char *TransactionMessageAttempts = "TransactionMessageAttempts";

        // //  [int] [required] [RW] Delay (sec) between retries.
        // constexpr const char *TransactionMessageRetryInterval = "TransactionMessageRetryInterval";

        // //  [bool] [required] [RW] Unlock connector when EV disconnects.
        // constexpr const char *UnlockConnectorOnEVSideDisconnect = "UnlockConnectorOnEVSideDisconnect";

        // //  [int] [required] [RW] [Unit seconds] Interval (sec) for WebSocket ping (read-only or RW depending on impl).
        // constexpr const char *WebSocketPingInterval = "WebSocketPingInterval";

        // // =============================================================================
        // // 🔹 Local Authorization List Management Profile (Optional)
        // // =============================================================================

        // // [bool] [required] [RW] Whether local authorization list is enabled.
        // constexpr const char *LocalAuthListEnabled = "LocalAuthListEnabled";

        // // [int] [required] [R] Max number of entries in local auth list (read-only).
        // constexpr const char *LocalAuthListMaxLength = "LocalAuthListMaxLength";

        // //[int] [required] [R] Maximum number of identifications that can be send in a single SendLocalList.req (read-only)
        // constexpr const char *SendLocalListMaxLength = "LocalAuthListMaxLength";

        // // =============================================================================
        // // 🔹 Reservation Profile (Optional)
        // // =============================================================================

        // // [bool] [optional] [R] Whether connector 0 (entire CP) can be reserved (read-only).
        // constexpr const char *ReserveConnectorZeroSupported = "ReserveConnectorZeroSupported";

        // // =============================================================================
        // // 🔹 Smart Charging Profile (Optional)
        // // =============================================================================

        // // [int] [required] [R] Maximum stack level for charge profiles.
        // constexpr const char *ChargeProfileMaxStackLevel = "ChargeProfileMaxStackLevel";

        // // [CSL] [required] [R] Allowed units: "Current" and/or "Power" (e.g., "Current,Power").
        // constexpr const char *ChargingScheduleAllowedChargingRateUnit = "ChargingScheduleAllowedChargingRateUnit";

        // // [int] [required] [R] Max number of periods in a charging schedule.
        // constexpr const char *ChargingScheduleMaxPeriods = "ChargingScheduleMaxPeriods";

        // // [bool] [optional] [R] Whether connector supports switching from 3-phase to 1-phase (read-only).
        // constexpr const char *ConnectorSwitch3to1PhaseSupported = "ConnectorSwitch3to1PhaseSupported";

        // // [int] [required] [R] Max number of installed charging profiles.
        // constexpr const char *MaxChargingProfilesInstalled = "MaxChargingProfilesInstalled";


        // /* 安全拓展配置 */
        // // [bool] [optional] [R]
        // constexpr const char *AdditionalRootCertificateCheck = "AdditionalRootCertificateCheck";

        // // [String] [optional] [W]
        // constexpr const char *AuthorizationKey = "AuthorizationKey";

        // // [int] [optional] [R]
        // constexpr const char *CertificateSignedMaxChainSize = "CertificateSignedMaxChainSize";

        // // [int] [optional] [R]
        // constexpr const char *CertificateStoreMaxLength = "CertificateStoreMaxLength";

        // // [String] [optional] [RW]
        // constexpr const char *CpoName = "CpoName";

        // // [int] [optional] [R] Default, when no security profile is yet configured: 0.
        // constexpr const char *SecurityProfile = "SecurityProfile";

        // /*  Firmware and Diagnostics File Transfer */
        // // [CSL] [optional] [R]
        // constexpr const char *SupportedFileTransferProtocols = "SupportedFileTransferProtocols";

    } // namespace config
} // namespace ocpp

#endif /* STANDARD_CONFIGURATION_KEY_NAMES_H */
