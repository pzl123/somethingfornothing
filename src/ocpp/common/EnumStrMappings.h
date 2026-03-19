#ifndef ENUMSTRMAPPINGS_H
#define ENUMSTRMAPPINGS_H

#include <string>

#include "ocpp/common/BidirectionalMap.h"
#include "ocpp/type/Reason.h"
#include "ocpp/type/OcppStatus.h"
#include "ocpp/type/ChargePointErrorCode.h"
#include "ocpp/security-extension/SecurityDatatypes.h"


namespace ocpp1_6
{
    static const BidirectionalMap<Reason> reasonMap
    {
        {"DeAuthorized", Reason::DeAuthorized},
        {"EmergencyStop", Reason::EmergencyStop},
        {"EVDisconnected", Reason::EVDisconnected},
        {"HardReset", Reason::HardReset},
        {"Local", Reason::Local},
        {"Other", Reason::Other},
        {"PowerLoss", Reason::PowerLoss},
        {"Reboot", Reason::Reboot},
        {"Remote", Reason::Remote},
        {"SoftReset", Reason::SoftReset},
        {"UnlockCommand", Reason::UnlockCommand}
    };

    // ChargePointStatus 枚举类映射
    static const BidirectionalMap<ChargePointStatus> chargePointStatusMap
    {
        {"Available", ChargePointStatus::Available},
        {"Preparing", ChargePointStatus::Preparing},
        {"Charging", ChargePointStatus::Charging},
        {"SuspendedEVSE", ChargePointStatus::SuspendedEVSE},
        {"SuspendedEV", ChargePointStatus::SuspendedEV},
        {"Finishing", ChargePointStatus::Finishing},
        {"Reserved", ChargePointStatus::Reserved},
        {"Unavailable", ChargePointStatus::Unavailable},
        {"Faulted", ChargePointStatus::Faulted}
    };

    // AuthorizationStatus 枚举类映射
    static const BidirectionalMap<AuthorizationStatus> authorizationStatusMap
    {
        {"Accepted", AuthorizationStatus::Accepted},
        {"Blocked", AuthorizationStatus::Blocked},
        {"Expired", AuthorizationStatus::Expired},
        {"Invalid", AuthorizationStatus::Invalid},
        {"ConcurrentTx", AuthorizationStatus::ConcurrentTx},
        {"Other", AuthorizationStatus::Other}
    };

    // AvailabilityStatus 枚举类映射
    static const BidirectionalMap<AvailabilityStatus> availabilityStatusMap
    {
        {"Accepted", AvailabilityStatus::Accepted},
        {"Rejected", AvailabilityStatus::Rejected},
        {"Scheduled", AvailabilityStatus::Scheduled}
    };

    // CancelReservationStatus 枚举类映射
    static const BidirectionalMap<CancelReservationStatus> cancelReservationStatusMap
    {
        {"Accepted", CancelReservationStatus::Accepted},
        {"Rejected", CancelReservationStatus::Rejected}
    };

    // ChargingProfileStatus 枚举类映射
    static const BidirectionalMap<ChargingProfileStatus> chargingProfileStatusMap
    {
        {"Accepted", ChargingProfileStatus::Accepted},
        {"Rejected", ChargingProfileStatus::Rejected},
        {"NotSupported", ChargingProfileStatus::NotSupported}
    };

    // ClearCacheStatus 枚举类映射
    static const BidirectionalMap<ClearCacheStatus> clearCacheStatusMap
    {
        {"Accepted", ClearCacheStatus::Accepted},
        {"Rejected", ClearCacheStatus::Rejected}
    };

    // ClearChargingProfileStatus 枚举类映射
    static const BidirectionalMap<ClearChargingProfileStatus> clearChargingProfileStatusMap
    {
        {"Accepted", ClearChargingProfileStatus::Accepted},
        {"Unknown", ClearChargingProfileStatus::Unknown}
    };

    // ConfigurationStatus 枚举类映射
    static const BidirectionalMap<ConfigurationStatus> configurationStatusMap
    {
        {"Accepted", ConfigurationStatus::Accepted},
        {"Rejected", ConfigurationStatus::Rejected},
        {"RebootRequired", ConfigurationStatus::RebootRequired},
        {"NotSupported", ConfigurationStatus::NotSupported}
    };

    // DataTransferStatus 枚举类映射
    static const BidirectionalMap<DataTransferStatus> dataTransferStatusMap
    {
        {"Accepted", DataTransferStatus::Accepted},
        {"Rejected", DataTransferStatus::Rejected},
        {"UnknownMessageId", DataTransferStatus::UnknownMessageId}
    };

    // DiagnosticsStatus 枚举类映射
    static const BidirectionalMap<DiagnosticsStatus> diagnosticsStatusMap
    {
        {"Idle", DiagnosticsStatus::Idle},
        {"Uploaded", DiagnosticsStatus::Uploaded},
        {"UploadFailed", DiagnosticsStatus::UploadFailed},
        {"Uploading", DiagnosticsStatus::Uploading}
    };

    // FirmwareStatus 枚举类映射
    static const BidirectionalMap<FirmwareStatus> firmwareStatusMap
    {
        {"Downloaded", FirmwareStatus::Downloaded},
        {"DownloadFailed", FirmwareStatus::DownloadFailed},
        {"Downloading", FirmwareStatus::Downloading},
        {"Idle", FirmwareStatus::Idle},
        {"InstallationFailed", FirmwareStatus::InstallationFailed},
        {"Installing", FirmwareStatus::Installing},
        {"Installed", FirmwareStatus::Installed}
    };

    // GetCompositeScheduleStatus 枚举类映射
    static const BidirectionalMap<GetCompositeScheduleStatus> getCompositeScheduleStatusMap
    {
        {"Accepted", GetCompositeScheduleStatus::Accepted},
        {"Rejected", GetCompositeScheduleStatus::Rejected}
    };

    // RegistrationStatus 枚举类映射
    static const BidirectionalMap<RegistrationStatus> registrationStatusMap
    {
        {"Accepted", RegistrationStatus::Accepted},
        {"Pending", RegistrationStatus::Pending},
        {"Rejected", RegistrationStatus::Rejected}
    };

    // RemoteStartStopStatus 枚举类映射
    static const BidirectionalMap<RemoteStartStopStatus> remoteStartStopStatusMap
    {
        {"Accepted", RemoteStartStopStatus::Accepted},
        {"Rejected", RemoteStartStopStatus::Rejected}
    };

    // ReservationStatus 枚举类映射
    static const BidirectionalMap<ReservationStatus> reservationStatusMap
    {
        {"Accepted", ReservationStatus::Accepted},
        {"Faulted", ReservationStatus::Faulted},
        {"Occupied", ReservationStatus::Occupied},
        {"Rejected", ReservationStatus::Rejected}
    };

    // ResetStatus 枚举类映射
    static const BidirectionalMap<ResetStatus> resetStatusMap
    {
        {"Accepted", ResetStatus::Accepted},
        {"Rejected", ResetStatus::Rejected}
    };

    // TriggerMessageStatus 枚举类映射
    static const BidirectionalMap<TriggerMessageStatus> triggerMessageStatusMap
    {
        {"Accepted", TriggerMessageStatus::Accepted},
        {"Rejected", TriggerMessageStatus::Rejected},
        {"NotImplemented", TriggerMessageStatus::NotImplemented}
    };

    // UnlockStatus 枚举类映射
    static const BidirectionalMap<UnlockStatus> unlockStatusMap
    {
        {"Unlocked", UnlockStatus::Unlocked},
        {"UnlockFailed", UnlockStatus::UnlockFailed},
        {"NotSupported", UnlockStatus::NotSupported}
    };

    // UpdateStatus 枚举类映射
    static const BidirectionalMap<UpdateStatus> updateStatusMap
    {
        {"Accepted", UpdateStatus::Accepted},
        {"Failed", UpdateStatus::Failed},
        {"NotSupported", UpdateStatus::NotSupported},
        {"VersionMismatch", UpdateStatus::VersionMismatch}
    };

    //重置类型枚举类映射
    static const BidirectionalMap<ResetType> resetTypeMap
    {
        {"Hard", ResetType::Hard},
        {"Soft", ResetType::Soft}
    };

    //消息触发类型枚举类映射
    static const BidirectionalMap<MessageTriggerType> messageTriggerTypeMap
    {
        {"BootNotification", MessageTriggerType::BootNotification},
        {"DiagnosticsStatusNotification", MessageTriggerType::DiagnosticsStatusNotification},
        {"FirmwareStatusNotification", MessageTriggerType::FirmwareStatusNotification},
        {"Heartbeat", MessageTriggerType::Heartbeat},
        {"MeterValues", MessageTriggerType::MeterValues},
        {"StatusNotification", MessageTriggerType::StatusNotification}
    };


    static const BidirectionalMap<ChargePointErrorCode> chargePointErrorCodeMap
    {
        {"ConnectorLockFailure", ChargePointErrorCode::ConnectorLockFailure},
        {"EVCommunicationError", ChargePointErrorCode::EVCommunicationError},
        {"GroundFailure", ChargePointErrorCode::GroundFailure},
        {"HighTemperature", ChargePointErrorCode::HighTemperature},
        {"InternalError", ChargePointErrorCode::InternalError},
        {"LocalListConflict", ChargePointErrorCode::LocalListConflict},
        {"NoError", ChargePointErrorCode::NoError},
        {"OtherError", ChargePointErrorCode::OtherError},
        {"OverCurrentFailure", ChargePointErrorCode::OverCurrentFailure},
        {"OverVoltage", ChargePointErrorCode::OverVoltage},
        {"PowerMeterFailure", ChargePointErrorCode::PowerMeterFailure},
        {"PowerSwitchFailure", ChargePointErrorCode::PowerSwitchFailure},
        {"ReaderFailure", ChargePointErrorCode::ReaderFailure},
        {"ResetFailure", ChargePointErrorCode::ResetFailure},
        {"UnderVoltage", ChargePointErrorCode::UnderVoltage},
        {"WeakSignal", ChargePointErrorCode::WeakSignal}
    };

    static const BidirectionalMap<AvailabilityType> AvailabilityTypeMap
    {
        {"Inoperative", AvailabilityType::Inoperative},
        {"Operative", AvailabilityType::Operative}
    };


    static const BidirectionalMap<CertificateUseEnumType> certificateUseEnumTypeMap
    {
        {"CentralSystemRootCertificate", CertificateUseEnumType::CentralSystemRootCertificate},
        {"ManufacturerRootCertificate", CertificateUseEnumType::ManufacturerRootCertificate}
    };


    static const BidirectionalMap<SecurityEventCode> securityEventCodeMap
    {
        {"FirmwareUpdated", SecurityEventCode::FirmwareUpdated},
        {"FailedToAuthenticateAtCentralSystem", SecurityEventCode::FailedToAuthenticateAtCentralSystem},
        {"CentralSystemFailedToAuthenticate", SecurityEventCode::CentralSystemFailedToAuthenticate},
        {"SettingSystemTime", SecurityEventCode::SettingSystemTime},
        {"StartupOfTheDevice", SecurityEventCode::StartupOfTheDevice},
        {"ResetOrReboot", SecurityEventCode::ResetOrReboot},
        {"SecurityLogWasCleared", SecurityEventCode::SecurityLogWasCleared},
        {"ReconfigurationOfSecurityParameters", SecurityEventCode::ReconfigurationOfSecurityParameters},
        {"MemoryExhaustion", SecurityEventCode::MemoryExhaustion},
        {"InvalidMessages", SecurityEventCode::InvalidMessages},
        {"AttemptedReplayAttacks", SecurityEventCode::AttemptedReplayAttacks},
        {"TamperDetectionActivated", SecurityEventCode::TamperDetectionActivated},
        {"InvalidFirmwareSignature", SecurityEventCode::InvalidFirmwareSignature},
        {"InvalidFirmwareSigningCertificate", SecurityEventCode::InvalidFirmwareSigningCertificate},
        {"InvalidCentralSystemCertificate", SecurityEventCode::InvalidCentralSystemCertificate},
        {"InvalidChargePointCertificate", SecurityEventCode::InvalidChargePointCertificate},
        {"InvalidTLSVersion", SecurityEventCode::InvalidTLSVersion},
        {"InvalidTLSCipherSuite", SecurityEventCode::InvalidTLSCipherSuite}
    };

    static const BidirectionalMap<FirmwareStatusEnumType> firmwareStatusEnumTypeMap
    {
        {"Downloaded", FirmwareStatusEnumType::Downloaded},
        {"DownloadFailed", FirmwareStatusEnumType::DownloadFailed},
        {"Downloading", FirmwareStatusEnumType::Downloading},
        {"DownloadScheduled", FirmwareStatusEnumType::DownloadScheduled},
        {"DownloadPaused", FirmwareStatusEnumType::DownloadPaused},
        {"Idle", FirmwareStatusEnumType::Idle},
        {"InstallationFailed", FirmwareStatusEnumType::InstallationFailed},
        {"Installing", FirmwareStatusEnumType::Installing},
        {"Installed", FirmwareStatusEnumType::Installed},
        {"InstallRebooting", FirmwareStatusEnumType::InstallRebooting},
        {"InstallScheduled", FirmwareStatusEnumType::InstallScheduled},
        {"InstallVerificationFailed", FirmwareStatusEnumType::InstallVerificationFailed},
        {"InvalidSignature", FirmwareStatusEnumType::InvalidSignature},
        {"SignatureVerified", FirmwareStatusEnumType::SignatureVerified}
    };

    static const BidirectionalMap<CertificateSignedStatusEnumType> certificateSignedStatusEnumTypeMap
    {
        {"Accepted", CertificateSignedStatusEnumType::Accepted},
        {"Rejected", CertificateSignedStatusEnumType::Rejected}
    };

    static const BidirectionalMap<DeleteCertificateStatusEnumType> deleteCertificateStatusEnumTypeMap
    {
        {"Accepted", DeleteCertificateStatusEnumType::Accepted},
        {"Failed", DeleteCertificateStatusEnumType::Failed}
    };

    static const BidirectionalMap<GetInstalledCertificateStatusEnumType> getInstalledCertificateStatusEnumTypeMap
    {
        {"Accepted", GetInstalledCertificateStatusEnumType::Accepted},
        {"NotFound", GetInstalledCertificateStatusEnumType::NotFound}
    };

    static const BidirectionalMap<HashAlgorithmEnumType> hashAlgorithmEnumTypeMap
    {
        {"SHA256", HashAlgorithmEnumType::SHA256},
        {"SHA384", HashAlgorithmEnumType::SHA384},
        {"SHA512", HashAlgorithmEnumType::SHA512}
    };

    static const BidirectionalMap<LogEnumType> logEnumTypeMap
    {
        {"DiagnosticsLog", LogEnumType::DiagnosticsLog},
        {"SecurityLog", LogEnumType::SecurityLog}
    };

    static const BidirectionalMap<LogStatusEnumType> logStatusEnumTypeMap
    {
        {"Accepted", LogStatusEnumType::Accepted},
        {"Rejected", LogStatusEnumType::Rejected},
        {"AcceptedCanceled", LogStatusEnumType::AcceptedCanceled}
    };

    static const BidirectionalMap<CertificateStatusEnumType> certificateStatusEnumTypeMap
    {
        {"Accepted", CertificateStatusEnumType::Accepted},
        {"Failed", CertificateStatusEnumType::Failed},
        {"Rejected", CertificateStatusEnumType::Rejected}
    };

    static const BidirectionalMap<UploadLogStatusEnumType> uploadLogStatusEnumTypeMap
    {
        {"BadMessage", UploadLogStatusEnumType::BadMessage},
        {"Idle", UploadLogStatusEnumType::Idle},
        {"NotSupportedOperation", UploadLogStatusEnumType::NotSupportedOperation},
        {"PermissionDenied", UploadLogStatusEnumType::PermissionDenied},
        {"Uploaded", UploadLogStatusEnumType::Uploaded},
        {"UploadFailure", UploadLogStatusEnumType::UploadFailure},
        {"Uploading", UploadLogStatusEnumType::Uploading}
    };

    static const BidirectionalMap<GenericStatusEnumType> genericStatusEnumTypeMap
    {
        {"Accepted", GenericStatusEnumType::Accepted},
        {"Rejected", GenericStatusEnumType::Rejected}
    };

    static const BidirectionalMap<UpdateFirmwareStatusEnumType> updateFirmwareStatusEnumTypeMap
    {
        {"Accepted", UpdateFirmwareStatusEnumType::Accepted},
        {"Rejected", UpdateFirmwareStatusEnumType::Rejected},
        {"AcceptedCanceled", UpdateFirmwareStatusEnumType::AcceptedCanceled},
        {"InvalidCertificate", UpdateFirmwareStatusEnumType::InvalidCertificate},
        {"RevokedCertificate", UpdateFirmwareStatusEnumType::RevokedCertificate}
    };

    static const BidirectionalMap<MessageTriggerEnumType> messageTriggerEnumTypeMap
    {
        {"BootNotification", MessageTriggerEnumType::BootNotification},
        {"LogStatusNotification", MessageTriggerEnumType::LogStatusNotification},
        {"FirmwareStatusNotification", MessageTriggerEnumType::FirmwareStatusNotification},
        {"Heartbeat", MessageTriggerEnumType::Heartbeat},
        {"MeterValues", MessageTriggerEnumType::MeterValues},
        {"SignChargePointCertificate", MessageTriggerEnumType::SignChargePointCertificate},
        {"StatusNotification", MessageTriggerEnumType::StatusNotification}
    };

    static const BidirectionalMap<TriggerMessageStatusEnumType> triggerMessageStatusEnumTypeMap
    {
        {"Accepted", TriggerMessageStatusEnumType::Accepted},
        {"Rejected", TriggerMessageStatusEnumType::Rejected},
        {"NotImplemented", TriggerMessageStatusEnumType::NotImplemented}
    };



};


#endif