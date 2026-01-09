#ifndef TXDB_HANDLE_H
#define TXDB_HANDLE_H

#include <vector>

#include "ocpp/action/transaction/TransacrtionDef.h"
namespace ocpp1_6
{
  namespace txn
  {

    // 数据库操作接口
    class ITXDBHandle
    {
    public:
      virtual ~ITXDBHandle() = default;

      virtual bool initDatabaseTable() = 0;

      virtual bool insertTransaction(const Transaction &transaction) = 0;
      virtual bool deleteTransaction(uint32_t tx_id) = 0;
      virtual bool updateTransaction(const Transaction &transaction) = 0;
      virtual bool getTransactionByTxId(uint32_t tx_id, Transaction &tx) = 0;

      virtual std::vector<Transaction> getTransactionsByStatus(TransactionStatus status) = 0;

      virtual bool insertMeterValue(const MeterValue &meterValue) = 0;
      virtual bool updateMeterValue(const MeterValue &meterValue) = 0;
      virtual bool deleteMeterValue(uint32_t tx_id) = 0;
      virtual bool deleteMeterValueById(uint32_t id) = 0;

      virtual std::vector<MeterValue> getMeterValue(uint32_t tx_id, bool isSync) = 0;
    };
  } // namespace txn
} // namespace ocpp1_6

#endif // TXDB_HANDLE_H