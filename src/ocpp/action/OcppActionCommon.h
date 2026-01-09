#ifndef OCPP_ACTION_COMMON_H
#define OCPP_ACTION_COMMON_H

// 从桩到平台 (CP → CS)
static constexpr const char* BOOT_NOTIFICATION               = "BootNotification";
static constexpr const char* AUTHORIZE                       = "Authorize";
static constexpr const char* DIAGNOSTICS_STATUS_NOTIFICATION = "DiagnosticsStatusNotification";
static constexpr const char* FIRMWARE_STATUS_NOTIFICATION    = "FirmwareStatusNotification";
static constexpr const char* HEARTBEAT                       = "Heartbeat";
static constexpr const char* METER_VALUES                    = "MeterValues";
static constexpr const char* START_TRANSACTION               = "StartTransaction";
static constexpr const char* STATUS_NOTIFICATION             = "StatusNotification";
static constexpr const char* STOP_TRANSACTION                = "StopTransaction";
// 从平台到桩 (CS → CP)
static constexpr const char* REMOTE_START_TRANSACTION = "RemoteStartTransaction";
static constexpr const char* CANCEL_RESERVATION       = "CancelReservation";
static constexpr const char* CHANGE_AVAILABILITY      = "ChangeAvailability";
static constexpr const char* CHANGE_CONFIGURATION     = "ChangeConfiguration";
static constexpr const char* CLEAR_CACHE              = "ClearCache";
static constexpr const char* GET_CONFIGURATION        = "GetConfiguration";
static constexpr const char* GET_DIAGNOSTICS          = "GetDiagnostics";
static constexpr const char* GET_LOCAL_LIST_VERSION   = "GetLocalListVersion";
static constexpr const char* REMOTE_STOP_TRANSACTION  = "RemoteStopTransaction";
static constexpr const char* RESERVE_NOW              = "ReserveNow";
static constexpr const char* RESET                    = "Reset";
static constexpr const char* SEND_LOCAL_LIST          = "SendLocalList";
static constexpr const char* SET_CHARGING_PROFILE     = "SetChargingProfile";
static constexpr const char* TRIGGER_MESSAGE          = "TriggerMessage";
static constexpr const char* UNLOCK_CONNECTOR         = "UnlockConnector";
static constexpr const char* UPDATE_FIRMWARE          = "UpdateFirmware";
static constexpr const char* CLEAR_CHARGING_PROFILE   = "ClearChargingProfile";
static constexpr const char* GET_COMPOSITE_SCHEDULE   = "GetCompositeSchedule";
// 双向通用 (CP ↔ CS)
static constexpr const char* DATA_TRANSFER_COMMON = "DataTransfer";

// timer 时间单位：s
static constexpr const char* TIMER_NAME_RESEVERE_CHECK  = "ReserveCheck";
static constexpr int         RESERVE_CHECK_INTERVAL_SEC = 60;

static constexpr const char* TIMER_NAME_METER_VALUES_CLOCK   = "MeterValuesClock";
static constexpr int         METER_VALUES_CLOCK_INTERVAL_SEC = 3600;

static constexpr const char* BOOT_RETRY_TIMER        = "BootRetryTimer";
static constexpr int         BOOT_RETRY_INTERVAL_SEC = 10;

static constexpr const char* HEART_BEAT_TIMER        = "HeartBeatTimer";
static constexpr int         HEART_BEAT_INTERVAL_SEC = 60;

#endif // OCPP_ACTION_COMMON_H