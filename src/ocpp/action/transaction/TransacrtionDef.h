#ifndef TRANSACTION_DEF_H
#define TRANSACTION_DEF_H

#include "ocpp/common/Time.h"
#include "ocpp/type/Reason.h"
#include <string>
namespace ocpp1_6
{

    namespace txn
    {

        // 交易状态枚举
        enum class TransactionStatus
        {
            Started, // 已启动未停止
            Stopped, // 已停止未同步
            Synced   // 已同步完成
        };

        // 同步状态枚举
        enum class SyncStatus
        {
            Pending, // 待同步
            Success, // 同步成功
            Failed   // 同步失败
        };

        // 消息类型枚举
        enum class TnxType
        {
            StartTransaction,
            StopTransaction,
            MeterValues
        };

        constexpr uint32_t STOP_METER_UNFINISHED = 0xFFFFFFFF; // 4294967295：交易未结束的停止电量
        constexpr bool DEFAULT_ONLINE_FLAG = false;            // 在线状态默认值（贴合数据库DEFAULT 0）

        // // 交易数据模型
        // struct Transaction {
        //     int                   id;               // 本地自增ID
        //     uint32_t              tx_id;            // 交易ID
        //     uint32_t              connector_id;     // 连接器ID（1,2,...）
        //     std::string           id_tag;           // 用户标识（RFID/账号）
        //     std::string           start_time;       // 启动时间（ISO 8601，如"2024-09-11T10:00:00"）
        //     uint32_t              start_meter;      // 启动电量(Wh)（整数，精度足够）
        //     std::string           stop_time;        // 停止时间（同上）
        //     uint32_t              stop_meter;       // 停止电量(Wh)
        //     Reason                stop_reason;      // 停止原因
        //     bool                  is_online_start;  // 启动时是否在线
        //     bool                  is_online_stop;   // 停止时是否在线
        //     TransactionStatus     tx_status;        // 交易状态
        //     std::string           created_at;       // 创建时间（同上）
        //     std::string           updated_at;       // 更新时间（同上）
        // };

        struct Transaction
        {
            // 成员声明
            uint32_t id;                 // 本地自增ID
            uint32_t tx_id;              // 交易ID
            uint32_t connector_id;       // 连接器ID（1,2,...）
            std::string id_tag;          // 用户标识（RFID/账号）
            std::string start_time;      // 启动时间（ISO 8601）
            uint32_t start_meter;        // 启动电量(Wh)
            std::string stop_time;       // 停止时间（ISO 8601）
            uint32_t stop_meter;         // 停止电量(Wh)
            Reason stop_reason;          // 停止原因（Reason枚举来自Reason.h）
            bool is_online_start;        // 启动时是否在线
            bool is_online_stop;         // 停止时是否在线
            TransactionStatus tx_status; // 交易状态
            std::string created_at;      // 创建时间（ISO 8601）
            std::string updated_at;      // 更新时间（ISO 8601）

            Transaction()
                : id(0) // 自增ID默认0（数据库插入后生成）
                  ,
                  tx_id(0) // 交易ID默认0（启动时赋值）
                  ,
                  connector_id(0) // 连接器ID默认0（启动时赋值）
                  ,
                  id_tag("") // 用户标识默认空
                  ,
                  start_time("") // 启动时间默认空
                  ,
                  start_meter(0) // 启动电量默认0（初始值）
                  ,
                  stop_time("") // 停止时间默认空（未停止）
                  ,
                  stop_meter(STOP_METER_UNFINISHED) // 停止电量默认「未结束」标记
                  ,
                  stop_reason(Reason::Other) // 停止原因默认Other（匹配数据库DEFAULT 5）
                  ,
                  is_online_start(DEFAULT_ONLINE_FLAG) // 启动在线默认false
                  ,
                  is_online_stop(DEFAULT_ONLINE_FLAG) // 停止在线默认false
                  ,
                  tx_status(TransactionStatus::Started) // 交易状态默认「已启动未停止」
                  ,
                  created_at("") // 创建时间默认空
                  ,
                  updated_at("") // 更新时间默认空
            {
            }

            Transaction(uint32_t txId,
                        uint32_t connectorId,
                        const std::string &idTag,
                        const std::string &startTime,
                        uint32_t startMeter = 0,
                        bool onlineStart = true)
                : id(0),
                  tx_id(txId),
                  connector_id(connectorId),
                  id_tag(idTag),
                  start_time(startTime),
                  start_meter(startMeter),
                  stop_time(""),
                  stop_meter(STOP_METER_UNFINISHED),
                  stop_reason(Reason::Other),
                  is_online_start(onlineStart),
                  is_online_stop(DEFAULT_ONLINE_FLAG),
                  tx_status(TransactionStatus::Started),
                  created_at(startTime),
                  updated_at(startTime)
            {
            }
        };

        // 计量数据模型
        // struct MeterValue {
        //     int                   id;               // 本地自增ID
        //     uint32_t              tx_id;            // 交易ID
        //     std::string           timestamp;        // 计量时间（ISO 8601）
        //     std::string           energy;           // 累计电量(Wh)（支持小数）
        //     std::string           energy_unit;      // 电量单位
        //     std::string           voltage;          // 电压(V)
        //     std::string           voltage_unit;     // 电压单位
        //     std::string           current;          // 电流(A)
        //     std::string           current_unit;     // 电流单位
        //     std::string           soc;              // 当前 soc
        //     std::string           soc_unit;         // 当前 soc但单位
        //     std::string           power;            // 功率(W)
        //     std::string           power_unit;       // 功率单位
        //     uint32_t              context;          // 计量数据类型
        //     bool                  is_synced;        // 是否已同步
        // };

        struct MeterValue
        {
            // 成员声明
            uint32_t id;              // 本地自增ID
            uint32_t tx_id;           // 交易ID
            std::string timestamp;    // 计量时间（ISO 8601）
            std::string energy;       // 累计电量(Wh)（支持小数）
            std::string energy_unit;  // 电量单位
            std::string voltage;      // 电压(V)
            std::string voltage_unit; // 电压单位
            std::string current;      // 电流(A)
            std::string current_unit; // 电流单位
            std::string soc;          // 当前SOC
            std::string soc_unit;     // SOC单位
            std::string power;        // 功率(W)
            std::string power_unit;   // 功率单位
            uint32_t context;         // 计量数据类型（OCPP 1.6 MeterValueContext）
            bool is_synced;           // 是否已同步

            MeterValue()
                : id(0) // 自增ID默认0
                  ,
                  tx_id(0) // 交易ID默认0（关联交易时赋值）
                  ,
                  timestamp("") // 计量时间默认空
                  ,
                  energy("0.0") // 电量默认0.0
                  ,
                  energy_unit("Wh") // 电量单位默认Wh（OCPP 1.6 标准）
                  ,
                  voltage("0.0") // 电压默认0.0
                  ,
                  voltage_unit("V") // 电压单位默认V
                  ,
                  current("0.0") // 电流默认0.0
                  ,
                  current_unit("A") // 电流单位默认A
                  ,
                  soc("0") // SOC默认0
                  ,
                  soc_unit("%") // SOC单位默认%
                  ,
                  power("0.0") // 功率默认0.0
                  ,
                  power_unit("W") // 功率单位默认W
                  ,
                  context(0) // 计量类型默认0（OCPP 1.6 默认值）
                  ,
                  is_synced(false) // 同步状态默认未同步
            {
            }

            MeterValue(uint32_t txId, const std::string &ts, const std::string &energyVal)
                : id(0),
                  tx_id(txId),
                  timestamp(ts),
                  energy(energyVal),
                  energy_unit("Wh"),
                  voltage("0.0"),
                  voltage_unit("V"),
                  current("0.0"),
                  current_unit("A"),
                  soc("0"),
                  soc_unit("%"),
                  power("0.0"),
                  power_unit("W"),
                  context(0),
                  is_synced(false)
            {
            }
        };

    } // namespace txn
} // namespace ocpp1_6

#endif // TRANSACTION_DEF_H