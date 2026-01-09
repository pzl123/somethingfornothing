#include "Connectors.h"

#include "utils/utils.h"


namespace ocpp1_6
{
    namespace chargepoint
    {

        Connectors::Connectors(
            Database &db, ITimerPool &timerPool, ThreadPool &threadPool, unsigned int connectorCount, state::StatusNotification &notifier)
            : m_db(db), m_timerPool(timerPool), m_threadPool(threadPool), m_notifier(notifier)
        {
            m_connectors.reserve(connectorCount + 1);

            // 0：代表充电桩本身 1-N：对应枪编号
            for (unsigned int i = 0; i <= connectorCount; i++)
            {
                Connector c(i);
                m_connectors.emplace_back(c);
            }
        }

        Connectors::~Connectors() { }

        bool Connectors::initDatabaseTable()
        {
            auto query = m_db.query("CREATE TABLE IF NOT EXISTS Connector ("
                                    "connector_id    INT UNSIGNED PRIMARY KEY NOT NULL,"
                                    "status         VARCHAR(20) NOT NULL,"
                                    "transaction_id             INT UNSIGNED,"
                                    "transaction_id_tag        VARCHAR(128),"
                                    "transaction_parent_id_tag VARCHAR(128),"
                                    "reservation_id            INT UNSIGNED,"
                                    "reservation_id_tag        VARCHAR(128),"
                                    "reservation_parent_id_tag VARCHAR(128),"
                                    "reservation_expiry_date   BIGINT,"
                                    "availability              INT UNSIGNED);");
            if (!query)
            {
                e_log("create table connector error");
                return false;
            }

            if (!query->exec())
            {
                e_log("exec table connector error");
                return false;
            }

            m_qInsert = m_db.query("INSERT INTO Connector ("
                                   "connector_id, status, "
                                   "transaction_id, transaction_id_tag, transaction_parent_id_tag, "
                                   "reservation_id, reservation_id_tag, reservation_parent_id_tag, reservation_expiry_date, availability"
                                   ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);");

            m_qUpdate = m_db.query("UPDATE Connector SET "
                                   "status=?, "
                                   "transaction_id=?, transaction_id_tag=?, transaction_parent_id_tag=?, "
                                   "reservation_id=?, reservation_id_tag=?, reservation_parent_id_tag=?, "
                                   "reservation_expiry_date=?, availability=? "
                                   "WHERE connector_id=?;");

            m_qFind = m_db.query("SELECT * FROM Connector WHERE connector_id = ?;");

            bool ret = m_qInsert && m_qUpdate && m_qFind;
            if (ret)
            {
                initConnectorData();
            }
            return ret;
        }

        Connector *Connectors::get(unsigned int id)
        {
            unsigned int maxid = count();
            if (id > maxid)
            {
                e_log("id:%u error, max id is:%u", id, maxid);
                return nullptr;
            }
            return &m_connectors[id];
        }

        Connector &Connectors::operator[](unsigned int id)
        {
            return m_connectors[id];
        }

        bool Connectors::updateStatus(unsigned int id, ChargePointStatus targetStatus,
                                      ChargePointErrorCode error, const std::string &info, bool force)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            Connector *c = get(id);
            if (!c)
            {
                return false;
            }

            ChargePointStatus currentStatus = c->status;
            if ((currentStatus < ChargePointStatus::Available) || (currentStatus > ChargePointStatus::Faulted) ||
                (targetStatus < ChargePointStatus::Available) || (targetStatus > ChargePointStatus::Faulted))
            {
                e_log("error status, currentStatus:%d, targetStatus:%d", currentStatus, targetStatus);
                return false;
            }

            if (!force)
            {
                // 校验状态转移是否合法（对照状态转移表）
                bool isTransitionAllowed = checkStatusTransition(currentStatus, targetStatus);
                if (!isTransitionAllowed)
                {
                    w_log("Connector %u: status transition from %s to %s is not allowed",
                             id,
                             chargePointStatusMap.toString(currentStatus, "unkonw_status"),
                             chargePointStatusMap.toString(targetStatus, "unkonw_status"));
                    return false;
                }
            }

            if (c->status != targetStatus)
            {
                // 合法则更新状态、保存
                c->status = targetStatus;
                save(id);
                d_log("Connector %u: status transition from %s to %s",
                             id,
                             chargePointStatusMap.toString(currentStatus, "unkonw_status"),
                             chargePointStatusMap.toString(targetStatus, "unkonw_status"));
            }

            m_threadPool.enqueue(
                [this, id, targetStatus, error, info]()
                {
                    if (this->registrationStatus() != RegistrationStatus::Accepted)
                    {
                        e_log("Chargepoint is not registered");
                        return;
                    }
                    m_notifier.sendCall(id, error, targetStatus, info);
                });

            return true;
        }

        bool Connectors::updateTransaction(unsigned int id, unsigned int transactionId, const std::string &idTag, const std::string &parentIdTag)
        {
            Connector *c = get(id);
            if (!c || !m_qUpdate)
            {
                e_log("Connectors::updateTransaction: invalid connector id:%u or m_qUpdate == null", id);
                return false;
            }
            c->transaction.id = transactionId;
            c->transaction.idTag = idTag;
            c->transaction.parentIdTag = parentIdTag;
            return save(id);
        }

        bool Connectors::updateReservation(unsigned int id,
                                           unsigned int reservationId,
                                           const std::string &idTag,
                                           const std::string &parentIdTag,
                                           const Time::DateTime &expiryDate,
                                           ReservationStatus status)
        {
            Connector *c = get(id);
            if (!c || !m_qUpdate)
            {
                e_log("Connectors::updateTransaction: invalid connector id:%u or m_qUpdate == null", id);
                return false;
            }

            c->reservation.id = reservationId;
            c->reservation.idTag = idTag;
            c->reservation.parentIdTag = parentIdTag;
            c->reservation.expiry = expiryDate;
            c->reservation.status = status;

            return save(id);
        }

        bool Connectors::updateAvailability(unsigned int id, AvailabilityType availability)
        {
            Connector *c = get(id);
            if (!c || !m_qUpdate)
            {
                e_log("Connectors::updateTransaction: invalid connector id:%u or m_qUpdate == null", id);
                return false;
            }

            c->availability = availability;

            return save(id);
        }

        bool Connectors::save(unsigned int id)
        {
            Connector *c = get(id);
            if (!c || !m_qUpdate)
            {
                e_log("Connectors::updateTransaction: invalid connector id:%u or m_qUpdate == null", id);
                return false;
            }

            m_qUpdate->reset();
            m_qUpdate->bind(1, chargePointStatusMap.toString(c->status, "Available"));
            m_qUpdate->bind(2, c->transaction.id);
            m_qUpdate->bind(3, c->transaction.idTag);
            m_qUpdate->bind(4, c->transaction.parentIdTag);
            m_qUpdate->bind(5, c->reservation.id);
            m_qUpdate->bind(6, c->reservation.idTag);
            m_qUpdate->bind(7, c->reservation.parentIdTag);
            m_qUpdate->bind(8, static_cast<int64_t>(c->reservation.expiry));
            m_qUpdate->bind(9, static_cast<int>(c->availability));
            m_qUpdate->bind(10, id);
            if (!m_qUpdate->exec())
            {
                e_log("Failed to save connector %u: %s", id, m_qUpdate->lastError().c_str());
                return false;
            }
            m_qUpdate->reset();
            d_log("Saved connector %u", id);
            return true;
        }

        bool Connectors::load(unsigned int id)
        {
            Connector *c = get(id);
            if (!c || !m_qFind)
            {
                e_log("Connectors::updateTransaction: invalid connector id:%u or m_qUpdate == null", id);
                return false;
            }

            m_qFind->reset();
            m_qFind->bind(1, id);

            if (!m_qFind->exec())
            {
                e_log("Failed to load connector %u: %s", id, m_qFind->lastError().c_str());
                return false;
            }

            if (m_qFind->hasRows() == false || m_qFind->getUInt32(0) != id)
            {
                w_log("Connector %u not found in database", id);
                return false;
            }

            c->id = id;
            c->status = chargePointStatusMap.toEnum(m_qFind->getString(1), ChargePointStatus::Available);
            c->transaction.id = m_qFind->getInt64(2);
            c->transaction.idTag = m_qFind->getString(3);
            c->transaction.parentIdTag = m_qFind->getString(4);
            c->reservation.id = m_qFind->getInt64(5);
            c->reservation.idTag = m_qFind->getString(6);
            c->reservation.parentIdTag = m_qFind->getString(7);
            c->reservation.expiry = static_cast<std::time_t>(m_qFind->getInt64(8));
            c->availability = static_cast<AvailabilityType>(m_qFind->getInt64(9));

            d_log("Loaded connector %u | "
                      "status: %s | "
                      "txId: %u | "
                      "txTag: %s | "
                      "txParent: %s | "
                      "resvId: %u | "
                      "resvTag: %s | "
                      "resvParent: %s | "
                      "resvExpiry: %s | "
                      "availability: %d",
                      id,
                      chargePointStatusMap.toString(c->status, "Available").c_str(),
                      c->transaction.id,
                      c->transaction.idTag.c_str(),
                      c->transaction.parentIdTag.c_str(),
                      c->reservation.id,
                      c->reservation.idTag.c_str(),
                      c->reservation.parentIdTag.c_str(),
                      Time::DateTime(c->reservation.expiry).str().c_str(),
                      static_cast<int>(c->availability));
            return true;
        }

        bool Connectors::insert(unsigned int id)
        {
            Connector *c = get(id);
            if (!c || !m_qInsert)
            {
                e_log("Connectors::updateTransaction: invalid connector id:%u or m_qUpdate == null", id);
                return false;
            }

            m_qInsert->reset();
            m_qInsert->bind(1, c->id);
            m_qInsert->bind(2, chargePointStatusMap.toString(c->status, "Available"));
            m_qInsert->bind(3, c->transaction.id);
            m_qInsert->bind(4, c->transaction.idTag);
            m_qInsert->bind(5, c->transaction.parentIdTag);
            m_qInsert->bind(6, c->reservation.id);
            m_qInsert->bind(7, c->reservation.idTag);
            m_qInsert->bind(8, c->reservation.parentIdTag);
            m_qInsert->bind(9, static_cast<int64_t>(c->reservation.expiry));
            m_qInsert->bind(10, static_cast<int>(c->availability));

            if (!m_qInsert->exec())
            {
                e_log("Failed to insert connector %u: %s", id, m_qInsert->lastError().c_str());
                return false;
            }
            return true;
        }

        void Connectors::initConnectorData()
        {
            unsigned int count = 0;
            unsigned int cfgCount = m_connectors.size();

            // 01. 查询数据库中已有的 connector 数
            auto query = m_db.query("SELECT count(connector_id) FROM Connector WHERE TRUE;");

            if (query && query->exec())
            {
                count = query->getUInt32(0);
            }
            i_log(""
                     "Database connector count: %u, Configured connector count: %u",
                     count,
                     cfgCount);

            if (count != cfgCount) // 不相等则使用默认枪数据
            {
                resetConnectors();
                return;
            }

            // 相等加载枪的数据库表
            for (auto &c : m_connectors)
            {
                if (!load(c.id))
                {
                    c = Connector(c.id);
                }
                initConnectorTimers(c.id);
            }
        }

        bool Connectors::resetConnectors()
        {
            m_timerPool.stopAllTimers();

            auto query = m_db.query("DELETE FROM Connector WHERE TRUE;");

            if (query && query->exec())
            {
                for (auto &c : m_connectors)
                {
                    c = Connector(c.id);

                    if (!insert(c.id))
                    {
                        e_log("Failed to insert connector %u: query failed", c.id);
                        return false;
                    }
                    initConnectorTimers(c.id);
                }
            }
            return true;
        }

        void Connectors::initConnectorTimers(unsigned int id)
        {
            Connector *c = get(id);
            if (!c)
                return;

            std::string reservationTimerName = "ReservationTimer_" + std::to_string(id);
            std::string meterValuesTimerName = "MeterValuesTimer_" + std::to_string(id);
            std::string idleTimerName = "IdleTimer_" + std::to_string(id);
            std::string prepareTimerName = "PrepareTimer_" + std::to_string(id);
            c->timers.reservation = m_timerPool.createTimer();
            c->timers.meterValues = m_timerPool.createTimer();
            c->timers.idle = m_timerPool.createTimer();
            c->timers.prepare = m_timerPool.createTimer();
            // c->timers.availability = m_timerPool.createTimer();
        }

    } // namespace chargepoint
} // namespace ocpp1_6
