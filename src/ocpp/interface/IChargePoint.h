#ifndef OPENOCPP_ICHARGEPOINT_H
#define OPENOCPP_ICHARGEPOINT_H


#include "ocpp/credentials/credentials.h"
#include "ocpp/interface/IChargePointHandler.h"
#include "ocpp/type/OcppStatus.h"
#include "ocpp/type/ChargePointErrorCode.h"
#include "ocpp/type/Reason.h"
#include <memory>

namespace ocpp1_6
{

  namespace chargepoint
  {

    // 调用者直接使用
    class IChargePoint
    {
    public:
      /**
       * @brief 创建一个充电桩实例
       * @param ocppCfg ocpp1_6 配置信息
       * @param eventsHandler 充电桩事件处理器
       * @return
       */
      static std::unique_ptr<IChargePoint> create(const std::string cfgPath, IChargePointHandler &eventsHandler);

      virtual ~IChargePoint() = default;

      // 模块启动, 包括连接，心跳流程
      virtual bool start() = 0;

      // 关闭模块 只是关闭连接，不会清理授权的换成
      virtual bool stop() = 0;

      virtual bool resetConnectorData(unsigned int connectorId) = 0;

      // 触发充电桩与中央系统的重新连接
      virtual bool reconnect() = 0;

      // 获取CP和 CS之间连接的状态
      virtual bool getConnectStatus() = 0;

      virtual AuthorizationStatus authorize(const std::string &idTag) = 0;

      /**
       * @brief 认证用户，并启动充电
       * @param connectorId 充电枪ID
       * @param idTag 用户ID
       * @return AuthorizationStatus 认证结果
       */
      virtual bool authAndStartTransaction(unsigned int connectorId, const std::string &idTag) = 0;

      /**
       * @brief 停止充电
       * @param connectorId 枪ID
       * @param idTag 用户的ID
       * @param reason 停止充电的原因
       * @return true 停止成功
       * @return false 停止失败
       */
      virtual AuthorizationStatus stopTransaction(unsigned int connector_id, const std::string &id_tag, Reason reason) = 0;

      /**
       * @brief 发送充电状态通知
       * @param connectorId 枪ID
       * @param status 充电状态
       * @param notifyErrorInfoCfg 错误信息
       */
      virtual bool statusNotification(unsigned int connector_id, ChargePointStatus status, ChargePointErrorCode &errorCode, const std::string &other) = 0;

      /**
       * @brief 设置连接器是否可用
       * @param connectorId 枪ID
       * @param Avail 是否可用
       * @return 返回操作是否成功，成功返回 true，失败返回 false
       */
      virtual bool setConnStaus(unsigned int connectorId, ChargePointStatus ConnectStatus) = 0;

      /**
       * @brief 发送电表数据
       * @param connectorId 枪ID
       * @param bSend
       * @return
       */
      virtual bool sendMeterValuesPos(unsigned int connectorId, std::string posId, bool bSend) = 0;

      /**
       * @brief 通知固件更新操作的结束
       * @param success 如果固件已安装则设置为 true，
       *                否则设置为 false，如果安装失败
       * @return 如果通知已发送，返回 true，否则返回 false
       */
      virtual bool notifyFirmwareUpdateStatus(FirmwareStatus Status) = 0;

      /**
       * @brief 通知诊断上传状态
       * @param Status 诊断上传状态
       * @return 返回诊断上传状态
       */
      virtual bool notifyDiagnosticsUploadStatus(DiagnosticsStatus Status) = 0;

      /**
       * @brief 发送 DataTransfer 请求
       * @param vendorId 厂商ID
       * @param messageId 消息ID
       * @param requestData 请求数据
       * @return 如果请求已发送并接受，返回 true，否则返回 false
       */
      virtual bool dataTransfer(const std::string &vendorId, const std::string &messageId, const std::string &requestData) = 0;

      /**************************暂时不实现***************************/
      // /**
      //  * @brief 记录安全事件
      //  * @param type 安全事件的类型
      //  * @param message 关于发生的安全事件的附加信息
      //  * @param critical 如果是非标准安全事件，表示其严重性
      //  *                 （只有关键事件会转发给中央系统）
      //  * @return 如果安全事件已记录，返回 true，否则返回 false
      //  */
      // virtual bool logSecurityEvent(const std::string& type, const std::string& message, bool critical = false) = 0;

      // /**
      //  * @brief 清除所有安全事件
      //  * @return 如果安全事件已清除，返回 true，否则返回 false
      //  */
      // virtual bool clearSecurityEvents() = 0;

      // /**
      //  * @brief 发送 CSR 请求以签署证书
      //  *        （仅在 InternalCertificateManagementEnabled = false 时可以使用）
      //  * @param csr CSR 请求
      //  * @return 如果请求已发送并接受，返回 true，否则返回 false
      //  */
      // virtual bool sendCSR(const std::string& csr) = 0;

      // /**
      //  * @brief 发送请求以获取一个证书
      //  * @param certificate 证书（使用 PEM 编码）
      //  * @return 如果请求已发送并接受，返回 true，否则返回 false
      //  */
      // virtual bool sendCertificate(const std::string& certificate) = 0;

      /**
       * @brief 获取配置信息
       * @param key 配置项的键
       * @param value 配置项的值
       * @return 如果获取成功，返回 true，否则返回 false
       */
      virtual bool getConfig(const std::string &key, std::string &value) = 0;

      /**
       * @brief 设置配置信息
       * @param key 配置项的键
       * @param value 配置项的值
       * @return 如果设置成功，返回 true，否则返回 false
       */
      virtual bool setConfig(const std::string &key, const std::string &value) = 0;
    };

  } // namespace chargepoint
} // namespace ocpp1_6

#endif // OPENOCPP_ICHARGEPOINT_H
