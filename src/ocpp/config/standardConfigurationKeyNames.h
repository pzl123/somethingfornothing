#ifndef STANDARD_CONFIGURATION_KEY_NAMES_H
#define STANDARD_CONFIGURATION_KEY_NAMES_H

namespace ocpp1_6
{
    namespace config
    {

        // =============================================================================
        // 🔹 Core Profile (Required)
        // =============================================================================

        // AllowOfflineTxForUnknownId: [bool] [optional] [RW] Whether to allow transactions for unknown IdTags when offline.
        constexpr const char *AllowOfflineTxForUnknownId = "AllowOfflineTxForUnknownId";

        // AuthorizationCacheEnabled: [bool] [optional] [RW] Whether the Charge Point uses an authorization cache.
        constexpr const char *AuthorizationCacheEnabled = "AuthorizationCacheEnabled";

        // AuthorizeRemoteTxRequests: [bool] [required] [R or RW.] Whether remote start requests require authorization.
        constexpr const char *AuthorizeRemoteTxRequests = "AuthorizeRemoteTxRequests";

        // AuthorizeRemoteTxRequests: [integer] [optional] [RW] [Unit times] Number of times to blink Charge Point lighting when signalling
        constexpr const char *BlinkRepeat = "BlinkRepeat";

        // ClockAlignedDataInterval: [int] [required] [RW] [Unit seconds] Interval (sec) for clock-aligned data transfer. 0 = disabled.
        constexpr const char *ClockAlignedDataInterval = "ClockAlignedDataInterval";

        // ConnectionTimeOut: [int] [required] [RW] [Unit seconds] Time (sec) to wait for EV connection after plug-in.
        constexpr const char * ConnectionTimeOut = "ConnectionTimeOut";

        // ConnectorPhaseRotation: [CSL] [required] [RW] Phase rotation per connector (e.g., "NotApplicable,RST,RST").
        constexpr const char *ConnectorPhaseRotation = "ConnectorPhaseRotation";

        // ConnectorPhaseRotationMaxLength: [int] [optional] [R] Max length of ConnectorPhaseRotation string (read-only).
        constexpr const char *ConnectorPhaseRotationMaxLength = "ConnectorPhaseRotationMaxLength";

        // GetConfigurationMaxKeys: [int] [required] [R] Max number of keys in GetConfiguration request (read-only).
        constexpr const char *GetConfigurationMaxKeys = "GetConfigurationMaxKeys";

        // HeartbeatInterval: [int] [required] [RW] [Unit seconds] Interval (sec) between Heartbeat messages.
        constexpr const char *HeartbeatInterval = "HeartbeatInterval";

        // LightIntensity: [int] [optional] [RW] Current light intensity (if supported).
        constexpr const char *LightIntensity = "LightIntensity";

        // LocalAuthorizeOffline: [bool] [required] [RW] Use local auth list when offline.
        constexpr const char *LocalAuthorizeOffline = "LocalAuthorizeOffline";

        // LocalPreAuthorize: [bool] [required] [RW] Pre-authorize using local list before online check.
        constexpr const char *LocalPreAuthorize = "LocalPreAuthorize";

        // MaxEnergyOnInvalidId: [int] [optional] [RW] [Unit Wh] Max energy (Wh) allowed for invalid/unknown IdTag.
        constexpr const char *MaxEnergyOnInvalidId = "MaxEnergyOnInvalidId";

        // MeterValuesAlignedData: [CSL] [required] [RW] Comma-separated measurands for aligned data (e.g., "Power.Active.Import").
        constexpr const char *MeterValuesAlignedData = "MeterValuesAlignedData";

        // MeterValuesAlignedDataMaxLength: [int] [optional] [R] Max length of MeterValuesAlignedData (read-only).
        constexpr const char *MeterValuesAlignedDataMaxLength = "MeterValuesAlignedDataMaxLength";

        // MeterValuesSampledData: [CSL] [required] [RW] Comma-separated measurands for sampled data.
        constexpr const char *MeterValuesSampledData = "MeterValuesSampledData";

        // MeterValuesSampledDataMaxLength: [int] [optional] [R] Max length of MeterValuesSampledData (read-only).
        constexpr const char *MeterValuesSampledDataMaxLength = "MeterValuesSampledDataMaxLength";

        // MeterValueSampleInterval: [int] [required] [RW] [Unit seconds] Interval (sec) between meter value samples.
        constexpr const char *MeterValueSampleInterval = "MeterValueSampleInterval";

        // MinimumStatusDuration: [int] [optional] [RW] [Unit seconds] Min time (sec) a status must persist before sending StatusNotification.
        constexpr const char *MinimumStatusDuration = "MinimumStatusDuration";

        // NumberOfConnectors: [int] [required] [R] Number of connectors (read-only).
        constexpr const char *NumberOfConnectors = "NumberOfConnectors";

        // ResetRetries: [int] [required] [R] [Unit times] Number of times to retry reset before giving up.
        constexpr const char *ResetRetries = "ResetRetries";

        // StopTransactionOnEVSideDisconnect: [bool][required] [RW] Stop transaction when EV disconnects.
        constexpr const char *StopTransactionOnEVSideDisconnect = "StopTransactionOnEVSideDisconnect";

        // StopTransactionOnInvalidId: [bool] [required] [RW] Stop transaction if invalid IdTag presented during active tx.
        constexpr const char *StopTransactionOnInvalidId = "StopTransactionOnInvalidId";

        // StopTxnAlignedData: [CSL] [required] [RW]  Measurands to include in StopTransaction message (aligned).
        constexpr const char *StopTxnAlignedData = "StopTxnAlignedData";

        // StopTxnAlignedDataMaxLength: [int] [optional] [R] Max length (read-only).
        constexpr const char *StopTxnAlignedDataMaxLength = "StopTxnAlignedDataMaxLength";

        // StopTxnSampledData: [CSL] [required] [RW] Measurands to include in StopTransaction message (sampled).
        constexpr const char *StopTxnSampledData = "StopTxnSampledData";

        // StopTxnSampledDataMaxLength: [int] Max length (read-only).
        constexpr const char *StopTxnSampledDataMaxLength = "StopTxnSampledDataMaxLength";

        // SupportedFeatureProfiles: [CSL] [required] [R] Comma-separated list of supported profiles (read-only).
        constexpr const char *SupportedFeatureProfiles = "SupportedFeatureProfiles";

        // SupportedFeatureProfilesMaxLength: [int] Max length (read-only).
        constexpr const char *SupportedFeatureProfilesMaxLength = "SupportedFeatureProfilesMaxLength";

        // TransactionMessageAttempts: [int] [required] [RW] [Unit times] Number of attempts to send a transaction-related message.
        constexpr const char *TransactionMessageAttempts = "TransactionMessageAttempts";

        // TransactionMessageRetryInterval: [int] [required] [RW] Delay (sec) between retries.
        constexpr const char *TransactionMessageRetryInterval = "TransactionMessageRetryInterval";

        // UnlockConnectorOnEVSideDisconnect: [bool] [required] [RW] Unlock connector when EV disconnects.
        constexpr const char *UnlockConnectorOnEVSideDisconnect = "UnlockConnectorOnEVSideDisconnect";

        // WebSocketPingInterval: [int] [required] [RW] [Unit seconds] Interval (sec) for WebSocket ping (read-only or RW depending on impl).
        constexpr const char *WebSocketPingInterval = "WebSocketPingInterval";

        // =============================================================================
        // 🔹 Local Authorization List Management Profile (Optional)
        // =============================================================================

        // LocalAuthListEnabled: [bool] [required] [RW] Whether local authorization list is enabled.
        constexpr const char *LocalAuthListEnabled = "LocalAuthListEnabled";

        // LocalAuthListMaxLength: [int] [required] [R] Max number of entries in local auth list (read-only).
        constexpr const char *LocalAuthListMaxLength = "LocalAuthListMaxLength";

        // SendLocalListMaxLength: [int] [required] [R] Maximum number of identifications that can be send in a single SendLocalList.req (read-only)
        constexpr const char *SendLocalListMaxLength = "LocalAuthListMaxLength";

        // =============================================================================
        // 🔹 Reservation Profile (Optional)
        // =============================================================================

        // ReserveConnectorZeroSupported: [bool] [optional] [R] Whether connector 0 (entire CP) can be reserved (read-only).
        constexpr const char *ReserveConnectorZeroSupported = "ReserveConnectorZeroSupported";

        // =============================================================================
        // 🔹 Smart Charging Profile (Optional)
        // =============================================================================

        // ChargeProfileMaxStackLevel: [int] [required] [R] Maximum stack level for charge profiles.
        constexpr const char *ChargeProfileMaxStackLevel = "ChargeProfileMaxStackLevel";

        // ChargingScheduleAllowedChargingRateUnit: [CSL] [required] [R] Allowed units: "Current" and/or "Power" (e.g., "Current,Power").
        constexpr const char *ChargingScheduleAllowedChargingRateUnit = "ChargingScheduleAllowedChargingRateUnit";

        // ChargingScheduleMaxPeriods: [int] [required] [R] Max number of periods in a charging schedule.
        constexpr const char *ChargingScheduleMaxPeriods = "ChargingScheduleMaxPeriods";

        // ConnectorSwitch3to1PhaseSupported: [bool] [optional] [R] Whether connector supports switching from 3-phase to 1-phase (read-only).
        constexpr const char *ConnectorSwitch3to1PhaseSupported = "ConnectorSwitch3to1PhaseSupported";

        // MaxChargingProfilesInstalled: [int] [required] [R] Max number of installed charging profiles.
        constexpr const char *MaxChargingProfilesInstalled = "MaxChargingProfilesInstalled";

    } // namespace config
} // namespace ocpp

#endif /* STANDARD_CONFIGURATION_KEY_NAMES_H */
