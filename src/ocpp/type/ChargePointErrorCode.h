#ifndef CHARGE_POINT_ERROR_CODE_H
#define CHARGE_POINT_ERROR_CODE_H
namespace ocpp1_6
{
    enum class ChargePointErrorCode
    {
        ConnectorLockFailure, ///< 解锁或锁定连接器失败。
        EVCommunicationError, ///< 与车辆的通信失败，可能是 Mode 3 或其他通信协议问题。
        GroundFailure,        ///< 接地故障电路断路器已激活。
        HighTemperature,      ///< 充电点内部温度过高。
        InternalError,        ///< 内部硬件或软件组件出错。
        LocalListConflict,    ///< 授权信息与本地授权列表冲突。
        NoError,              ///< 无错误报告。
        OtherError,           ///< 其他类型的错误。
        OverCurrentFailure,   ///< 过电流保护装置跳闸。
        OverVoltage,          ///< 电压超过可接受水平。
        PowerMeterFailure,    ///< 无法读取电气/能量/功率计。
        PowerSwitchFailure,   ///< 无法控制电源开关。
        ReaderFailure,        ///< ID 标签读取器故障。
        ResetFailure,         ///< 无法执行重置操作。
        UnderVoltage,         ///< 电压低于可接受水平。
        WeakSignal            ///< 无线通信设备报告信号弱。
    };
} //  namespace ocpp1_6
#endif // CHARGE_POINT_ERROR