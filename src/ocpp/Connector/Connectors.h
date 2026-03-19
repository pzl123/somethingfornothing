#ifndef CONNECTORS_H
#define CONNECTORS_H

#include <string>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "ocpp/type/OcppStatus.h"
#include "ocpp/common/Time.h"
#include "ocpp/tool/timer/ITimerPool.h"
#include "ocpp/tool/database/DataBase.h"
#include "ocpp/config/ConfigManager.h"
#include "ocpp/action/status/StatusNotification.h"


namespace ocpp1_6
{

    inline unsigned int statusToTableId(ChargePointStatus status)
    {
        switch (status)
        {
        case ChargePointStatus::Available:
            return 1;
        case ChargePointStatus::Preparing:
            return 2;
        case ChargePointStatus::Charging:
            return 3;
        case ChargePointStatus::SuspendedEV:
            return 4;
        case ChargePointStatus::SuspendedEVSE:
            return 5;
        case ChargePointStatus::Finishing:
            return 6;
        case ChargePointStatus::Reserved:
            return 7;
        case ChargePointStatus::Unavailable:
            return 8;
        case ChargePointStatus::Faulted:
            return 9;
        default:
            return 0; // 无效状态
        }
    }

    /* 见ocpp1.6 edition2  4.9.Status Notification */
    inline bool checkStatusTransition(ChargePointStatus current, ChargePointStatus target)
    {
        unsigned int fromId = statusToTableId(current);
        unsigned int toId   = statusToTableId(target);
        if (fromId == 0 || toId == 0)
            return false; // 无效状态

        // 对照状态转移表，判断“fromId → toId”是否允许
        // 表格规则：行是“State From”（A~I对应1~9），列是“State To”（1~9）
        // 例如：当前是Available（1，行A），目标是Preparing（2）→ 表中A2存在 → 允许
        const std::unordered_map<unsigned int, std::unordered_set<unsigned int>> allowedTransitions = {
            {1, {2, 4, 5, 7, 8, 9}},   // Available（行A）可转移到列2、3、4、5、7、8、9
            {2, {1, 3, 4, 5, 6, 9}},   // Preparing（行B）可转移到列1、3、4、5、6、9
            {3, {1, 4, 5, 6, 9}},      // Charging（行C）可转移到列1、4、5、6、8、9
            {4, {1, 3, 5, 6, 8, 9}},   // SuspendedEV（行D）可转移到列1、3、5、6、8、9
            {5, {1, 3, 4, 6, 8, 9}},   // SuspendedEVSE（行E）可转移到列1、3、4、6、8、9
            {6, {1, 2, 8, 9}},         // Finishing（行F）可转移到列1、2、8、9
            {7, {1, 2, 8, 9}},         // Reserved（行G）可转移到列1、2、8、9
            {8, {1, 2, 9}},            // Unavailable（行H）可转移到列1、2、3、4、5、9
            {9, {1, 2, 4, 5, 6, 7, 8}} // Faulted（行I）可转移到列1~8
        };

        auto it = allowedTransitions.find(fromId);
        if (it == allowedTransitions.end())
            return false;
        return it->second.count(toId) > 0;
    }

    /**
     * @brief 充电桩事务信息结构体
     */
    struct ConnectorTransaction
    {
        unsigned int id = 0;     ///< 事务ID
        std::string idTag;       ///< 用户标识标签
        std::string parentIdTag; ///< 父级标识标签

        /**
         * @brief 检查事务是否处于活跃状态
         * @return true表示事务活跃，false表示事务不活跃
         */
        bool active() const { return id != 0; }
    };

    /**
     * @brief 充电桩预约信息结构体
     */
    struct ConnectorReservation
    {
        ReservationStatus status = ReservationStatus::Rejected; ///< 预约状态
        unsigned int id = 0;                                    ///< 预约ID
        std::string idTag;                                      ///< 用户标识标签
        std::string parentIdTag;                                ///< 父级标识标签
        Time::DateTime expiry = Time::DateTime::now();          ///< 预约过期时间

        /**
         * @brief 检查预约是否处于活跃状态
         * @return true表示预约活跃，false表示预约不活跃
         */
        bool active() const { return id != 0; }
    };

    /**
     * @brief 充电桩相关定时器结构体
     */
    struct ConnectorTimers
    {
        Timer *reservation = nullptr;  ///< 预约定时器
        Timer *meterValues = nullptr;  ///< 电表值定时器
        Timer *idle = nullptr;         ///< 空闲定时器
        Timer *prepare = nullptr;      ///< 准备定时器
        Timer *availability = nullptr; ///< 可用性定时器
    };

    /**
     * @brief 充电连接器结构体
     */
    struct Connector
    {
        unsigned int id = 0; ///< 连接器ID
        ChargePointStatus status = ChargePointStatus::Available;     ///< 充电点状态
        AvailabilityType availability = AvailabilityType::Operative; ///< 可用性类型

        ConnectorTransaction transaction; ///< 当前事务信息
        ConnectorReservation reservation; ///< 当前预约信息

        ConnectorTimers timers; ///< 定时器集合

        /**
         * @brief 默认构造函数
         */
        Connector() = default;

        /**
         * @brief 带参数的构造函数
         * @param cid 连接器ID
         */
        explicit Connector(unsigned int cid) : id(cid) {}
    };

    namespace chargepoint
    {
        /**
         * @brief 充电桩连接器管理类
         * 负责管理多个充电连接器的状态、数据库操作和定时任务
         */
        class Connectors
        {
        public:
            /**
             * @brief 构造函数
             * @param db 数据库引用
             * @param timerPool 定时器池引用
             * @param threadPool 线程池引用
             * @param config 配置管理器引用
             * @param connectorCount 连接器数量
             * @param notifier 状态通知器引用
             */
            Connectors(Database &db, ITimerPool &timerPool, ThreadPool &threadPool, unsigned int connectorCount, state::StatusNotification &notifier);

            /**
             * @brief 析构函数
             */
            ~Connectors();

            /**
             * @brief 初始化数据库表
             * @return 成功返回true，失败返回false
             */
            bool initDatabaseTable();
            /**
             * @brief 重置所有连接器
             * @return 成功返回true，失败返回false
             */
            bool resetConnectors();

            /**
             * @brief 获取指定ID的连接器
             * @param id 连接器ID
             * @return 返回连接器指针，如果不存在则返回nullptr
             */
            Connector *get(unsigned int id);

            /**
             * @brief 通过索引获取连接器引用
             * @param id 连接器ID
             * @return 返回连接器引用
             */
            Connector &operator[](unsigned int id);

            /**
             * @brief 获取连接器总数
             * @return 返回连接器数量
             */
            unsigned int count() const { return m_connectors.size() - 1; }

            /**
             * @brief 更新连接器状态
             * @param id 连接器ID
             * @param status 新的状态
             * @param error 错误代码，默认为无错误
             * @param info 附加信息，默认为空字符串
             * @return 成功返回true，失败返回false
             */
            bool updateStatus(unsigned int id,
                              ChargePointStatus status,
                              ChargePointErrorCode error = ChargePointErrorCode::NoError,
                              const std::string &info = "",
                              bool force = false);

            bool updateTransaction(unsigned int       id,
                           unsigned int       transactionId = 0,
                           const std::string& idTag         = "",
                           const std::string& parentIdTag   = "");

            bool updateReservation(unsigned int          id,
                           unsigned int          reservationId = 0,
                           const std::string&    idTag         = " ",
                           const std::string&    parentIdTag   = "",
                           const Time::DateTime& expiryDate    = Time::DateTime::now(),
                           ReservationStatus     status        = ReservationStatus::Rejected);

            bool updateAvailability(unsigned int id, AvailabilityType availability = AvailabilityType::Operative);

            /**
             * @brief 保存连接器数据到数据库
             * @param id 连接器ID
             * @return 成功返回true，失败返回false
             */
            bool save(unsigned int id);

            /**
             * @brief 从数据库加载连接器数据
             * @param id 连接器ID
             * @return 成功返回true，失败返回false
             */
            bool load(unsigned int id);

            /**
             * @brief 插入连接器数据到数据库
             * @param id 连接器ID
             * @return 插入成功返回true，失败返回false
             */
            bool insert(unsigned int id);

            /**
             * @brief 获取注册状态
             * @return 返回当前注册状态
             */
            RegistrationStatus registrationStatus() const { return m_regStatus; }

            /**
             * @brief 设置注册状态
             * @param status 新的注册状态
             */
            void setRegistrationStatus(RegistrationStatus status) { m_regStatus = status; }

        private:
            Database &m_db;                        ///< 数据库引用
            ITimerPool &m_timerPool;               ///< 定时器池引用
            ThreadPool &m_threadPool;              ///< 线程池引用
            state::StatusNotification &m_notifier; ///< 状态通知器引用

            RegistrationStatus m_regStatus = RegistrationStatus::Rejected; ///< 注册状态

            std::vector<Connector> m_connectors; ///< 连接器容器

            std::unique_ptr<Database::Query> m_qInsert; ///< 插入查询对象
            std::unique_ptr<Database::Query> m_qUpdate; ///< 更新查询对象
            std::unique_ptr<Database::Query> m_qFind;   ///< 查找查询对象

            std::mutex m_mutex; ///< 互斥锁

            /**
             * @brief 初始化连接器数据
             */
            void initConnectorData();

            /**
             * @brief 初始化连接器定时器
             * @param id 连接器ID
             */
            void initConnectorTimers(unsigned int id);
        };
    } // namespace chargepoint
} // namespace ocpp1_6

#endif /* CONNECTORS_H */
