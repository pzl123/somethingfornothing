#ifndef REASON_H
#define REASON_H

#include <string>
namespace ocpp1_6
{

    enum class Reason
    {
        /// @brief DeAuthorized: 交易因授权状态被停止。
        DeAuthorized, ///< 交易因授权状态被停止。

        /// @brief EmergencyStop: 使用了紧急停止按钮。
        EmergencyStop, ///< 使用了紧急停止按钮。

        /// @brief EVDisconnected: 电缆断开，车辆移开充电单元。
        EVDisconnected, ///< 电缆断开，车辆移开充电单元。

        /// @brief HardReset: 收到硬重置命令。
        HardReset, ///< 收到硬重置命令。

        /// @brief Local: 用户在充电点本地请求停止。
        Local, ///< 用户在充电点本地请求停止。

        /// @brief Other: 其他原因。
        Other, ///< 其他原因。

        /// @brief PowerLoss: 完全失去电力。
        PowerLoss, ///< 完全失去电力。

        /// @brief Reboot: 本地发起的重置/重启。
        Reboot, ///< 本地发起的重置/重启。

        /// @brief Remote: 用户请求远程停止。
        Remote, ///< 用户请求远程停止。

        /// @brief SoftReset: 收到软重置命令。
        SoftReset, ///< 收到软重置命令。

        /// @brief UnlockCommand: 中央系统发送解锁命令。
        UnlockCommand ///< 中央系统发送解锁命令。
    };

} // namespace ocpp1_6
#endif
