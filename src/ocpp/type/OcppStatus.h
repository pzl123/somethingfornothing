#ifndef OCPPSTATUS_H
#define OCPPSTATUS_H
#include <string>

namespace ocpp1_6
{
    // ChargePointStatus 枚举类
    enum class ChargePointStatus
    {
        Available,     ///< 当连接器对新用户可用时（可操作）。
        Preparing,     ///< 当连接器正在准备充电时（可操作）。
        Charging,      ///< 当连接器的接触器闭合，允许车辆充电时（可操作）。
        SuspendedEVSE, ///< 当EV连接到EVSE，但EVSE不向EV提供能量时（可操作）。
        SuspendedEV,   ///< 当EV连接到EVSE，且EVSE提供能量但EV未使用任何能量时（可操作）。
        Finishing,     ///< 当连接器正在结束充电时（可操作）。
        Reserved,      ///< 当连接器被预约时（可操作）。
        Unavailable,   ///< 当连接器不可用时（不可操作）。
        Faulted,       ///< 当连接器故障时（不可操作）。
    };

    // AuthorizationStatus 枚举类
    enum class AuthorizationStatus
    {
        Accepted,     ///< 标识符允许充电。
        Blocked,      ///< 标识符已被阻止。不允许充电。
        Expired,      ///< 标识符已过期。不允许充电。
        Invalid,      ///< 标识符未知。不允许充电。
        ConcurrentTx, ///< 标识符已参与另一交易，不允许多个交易。
        Other,        ///< 其他状态
    };

    // AvailabilityStatus 枚举类
    enum class AvailabilityStatus
    {
        Accepted,  ///< 请求已被接受并将被执行。
        Rejected,  ///< 请求已被拒绝。
        Scheduled, ///< 请求已被接受，并将在正在进行的事务完成后执行。
    };

    // CancelReservationStatus 枚举类
    enum class CancelReservationStatus
    {
        Accepted, ///< 标识符的预约已被取消。
        Rejected, ///< 预约无法取消，因为该标识符没有激活的预约。
    };

    // ChargingProfileStatus 枚举类
    enum class ChargingProfileStatus
    {
        Accepted,     ///< 请求已被接受并将被执行。
        Rejected,     ///< 请求未被接受，将不会执行。
        NotSupported, ///< 充电点表示该请求不被支持。
    };

    // ClearCacheStatus 枚举类
    enum class ClearCacheStatus
    {
        Accepted, ///< 命令已被执行。
        Rejected, ///< 命令未被执行。
    };

    // ClearChargingProfileStatus 枚举类
    enum class ClearChargingProfileStatus
    {
        Accepted, ///< 请求已被接受并将被执行。
        Unknown,  ///< 找不到与请求匹配的充电配置文件。
    };

    // ConfigurationStatus 枚举类
    enum class ConfigurationStatus
    {
        Accepted,       ///< 配置键被支持，设置已更改。
        Rejected,       ///< 配置键被支持，但设置未能更改。
        RebootRequired, ///< 配置键被支持，设置已更改，但更改将在重启后生效。
        NotSupported,   ///< 配置键不被支持。
    };

    // DataTransferStatus 枚举类
    enum class DataTransferStatus
    {
        Accepted,         ///< 消息已被接受，包含的请求已被接受。
        Rejected,         ///< 消息已被接受，但包含的请求被拒绝。
        UnknownMessageId, ///< 消息无法解释，因为未知的 messageId 字符串。
        UnknownVendorId   ///< 配置键不被支持。
    };

    // DiagnosticsStatus 枚举类
    enum class DiagnosticsStatus
    {
        Idle,         ///< 充电点未执行与诊断相关的任务。
        Uploaded,     ///< 诊断信息已上传。
        UploadFailed, ///< 诊断上传失败。
        Uploading     ///< 文件正在上传。
    };

    // FirmwareStatus 枚举类
    enum class FirmwareStatus
    {
        Downloaded,         ///< 新固件已被充电点下载。
        DownloadFailed,     ///< 充电点下载固件失败。
        Downloading,        ///< 固件正在下载。
        Idle,               ///< 充电点未执行固件更新相关的任务。
        InstallationFailed, ///< 新固件安装失败。
        Installing,         ///< 固件正在安装。
        Installed           ///< 新固件已成功安装在充电点中。
    };

    // GetCompositeScheduleStatus 枚举类
    enum class GetCompositeScheduleStatus
    {
        Accepted, ///< 请求已被接受并将被执行。
        Rejected  ///< 请求未被接受，将不会执行。
    };

    // RegistrationStatus 枚举类
    enum class RegistrationStatus
    {
        Accepted, ///< 充电点已被中央系统接受。
        Pending,  ///< 中央系统尚未准备好接受充电点。
        Rejected  ///< 充电点未被中央系统接受。
    };

    // RemoteStartStopStatus 枚举类
    enum class RemoteStartStopStatus
    {
        Accepted, ///< 命令将被执行。
        Rejected  ///< 命令将不会被执行。
    };

    // ReservationStatus 枚举类
    enum class ReservationStatus
    {
        Accepted,   ///< 预约已成功创建。
        Faulted,    ///< 预约未能创建，连接器故障。
        Occupied,   ///< 预约未能创建，连接器已被占用。
        Rejected,   ///< 预约未能创建，未配置为接受预约。
        Unavailable ///< 预约未能创建，连接器不可用。
    };

    // ResetStatus 枚举类
    enum class ResetStatus
    {
        Accepted, ///< 命令将被执行。
        Rejected  ///< 命令将不会被执行。
    };

    // TriggerMessageStatus 枚举类
    enum class TriggerMessageStatus
    {
        Accepted,      ///< 请求的通知将被发送。
        Rejected,      ///< 请求的通知将不会被发送。
        NotImplemented ///< 请求的通知无法发送。
    };

    // UnlockStatus 枚举类
    enum class UnlockStatus
    {
        Unlocked,     ///< 连接器已成功解锁。
        UnlockFailed, ///< 解锁连接器失败。
        NotSupported  ///< 充电点没有连接器锁或未知的连接器ID。
    };

    // UpdateStatus 枚举类
    enum class UpdateStatus
    {
        Accepted,       ///< 本地授权列表已成功更新。
        Failed,         ///< 更新本地授权列表失败。
        NotSupported,   ///< 充电点不支持本地授权列表的更新。
        VersionMismatch ///< 请求中的版本号不匹配。
    };

    // ResetType 枚举类
    enum class ResetType
    {
        Hard, ///< 硬重置
        Soft  ///< 软重置
    };

    // MessageTriggerType 枚举类
    enum class MessageTriggerType
    {
        BootNotification,              ///< 触发 BootNotification 请求
        DiagnosticsStatusNotification, ///< 触发 DiagnosticsStatusNotification 请求
        FirmwareStatusNotification,    ///< 触发 FirmwareStatusNotification 请求
        Heartbeat,                     ///< 触发 Heartbeat 请求
        MeterValues,                   ///< 触发 MeterValues 请求
        StatusNotification             ///< 触发 StatusNotification 请求
    };

    enum class AvailabilityType
    {
        Inoperative, ///< 充电点不可操作
        Operative    ///< 充电点可操作
    };

    enum class ErrorCode
    {
        NotImplemented,                ///< 功能未实现
        NotSupported,                  ///< 功能不被支持
        InternalError,                 ///< 内部错误
        ProtocolError,                 ///< 协议错误
        SecurityError,                 ///< 安全错误
        FormationViolation,            ///< 形成违规
        PropertyConstraintViolation,   ///< 属性约束违规
        OccurrenceConstraintViolation, ///< 发生约束违规
        TypeConstraintViolation,       ///< 类型约束违规
        GenericError                   ///< 通用错误
    };

    // --------SampledValue 结构----------
    // Energy.Active.Import.Register
    // 车从充电桩吸收的累计有功电能（充入电池的电量，单位kWh），能反映充了多少电。
    // 充电前后对比能知道充了多少电。
    typedef struct
    {
        std::string value;
        std::string unit;
    } EnergyActiveImportRegister;

    // Power.Active.Import
    // 当前瞬时充电功率（单位kW），能反映充电速度。
    // 方便监控充电是否正常，是否达到预期功率。
    typedef struct
    {
        std::string value;
        std::string unit;
    } PowerActiveImport;

    // Voltage
    // 当前充电电压（单位V），监控电压是否在正常范围内。
    typedef struct
    {
        std::string value;
        std::string unit;
    } Voltage;

    // Current.Import
    // 当前充电电流（单位A），实时反映充电电流大小。
    typedef struct
    {
        std::string value;
        std::string unit;
    } CurrentImport;

    // SoC
    // 电池当前的荷电状态（百分比），显示电池电量变化，能帮助判断充电进度。
    typedef struct
    {
        std::string value;
        std::string unit;
    } SoC;

    typedef struct
    {
        EnergyActiveImportRegister energyActiveImportRegister;
        PowerActiveImport powerActiveImport;
        Voltage voltage;
        CurrentImport currentImport;
        SoC soc;
    } SampledValue;
} //  namespace ocpp1_6
#endif
