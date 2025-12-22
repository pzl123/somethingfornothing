/*
 * @Author: hd hedu_0908@163.com
 * @Date: 2024-10-04 15:56:21
 * @LastEditors: hd hedu_0908@163.com
 * @LastEditTime: 2025-2-21 10:43:13
 * @FilePath: /ocpp-develop/src/interface/IChargePointHandler.h
 * @Description: 充电桩事件处理接口
 */
#ifndef OCPPP_ICHARGEPOINTEVENTSHANDLER_H
#define OCPPP_ICHARGEPOINTEVENTSHANDLER_H

#include "rapidjson/document.h"

#include "ocpp/type/OcppStatus.h"
#include "ocpp/common/Common.h"
#include <string>
#include <ctime>

namespace ocpp1_6
{
    namespace chargepoint
    {

        // 调用者需要实现此接口
        class IChargePointHandler
        {
        public:
            virtual ~IChargePointHandler() = default;

            virtual bool restartOcpp() = 0;

            virtual void setSystemDateAndTime(const std::string &strTime) = 0; // 传入current time做时间同步

            virtual void connectionStateChanged(bool isConnected) = 0;

            virtual bool changeAvailabilityRequested(unsigned int connector_id, bool availability) = 0;

            /**
             * @brief 开启预约
             * @param connector_id 连接器Id
             * @return ture：预约开启成功 false：预约开启失败
             */
            virtual bool reservationStarted(unsigned int connector_id) = 0;

            /**
             * @brief 预约结束
             * @param connector_id 连接器Id
             * @param canceled ture：平台取消 false:预约到期
             * @return ture：预约结束成功 false：预约结束失败
             */
            virtual bool reservationEnded(unsigned int connector_id, bool canceled) = 0;

            virtual DataTransferStatus dataTransferRequested(const std::string &vendor_id,
                                                             const std::string &message_id,
                                                             const std::string &request_data,
                                                             std::string &response_data) = 0;

            // 获取当前计量值
            virtual SampledValue getMeterValue(unsigned int connector_id) = 0;

            /**
             * @brief 远程启动充电请求
             * @param connector_id 连接器Id
             * @param idTag 用户标识
             * @param transactionId 交易Id
             */
            virtual bool notifyStartTransaction(unsigned int connector_id, const std::string &idTag, unsigned int transactionId, bool isRemote, bool isOffline) = 0;

            /**
             * @brief 远程停止充电请求
             * @param connector_id 连接器Id
             * @param idTag 用户标识
             * @param transactionId 交易Id
             */
            virtual bool notifyStopTransaction(unsigned int connectorId, const std::string &idTag, unsigned int transactionId, bool isRemote) = 0;

            // 获取交易开始/结束电表测量值采样
            virtual int getTxStartStopMeterValue(unsigned int connector_id) = 0;

            // 获取诊断上传状态
            virtual DiagnosticsStatus getDiagnosticsUploadStatus() = 0;
            // 获取诊断文件名字
            virtual std::string getDiagnosticsFileName() = 0;
            // 设置诊断文件发送地址url
            virtual void setDiagnosticsInfo(std::string url, const std::string startTime, const std::string stopTime) = 0;
            // 获取固件升级状态
            virtual FirmwareStatus getFirmwareStatus() = 0;

            /**
             * @brief 设置固件升级文件下载地址url等信息
             * @param payload 固件信息负载
             */
            virtual void setFirmwareInfo(const std::string FirmwareInfo) = 0;

            /**
             * @brief getConnectorNum -获取连接器数目
             * @return 连接器数目
             */
            virtual unsigned int getConnectorNum() = 0;

            /**
             * @brief 重置充电桩的内部数据（仅在充电桩停止时可以进行）
             * @param ResetType 固件信息负载
             * @return 连接器数目
             */
            virtual bool resetData(const std::string ResetType) = 0;

            // /**
            //  * @brief 重启OCPP
            //  * @return
            // */
            virtual ChargePointStatus getChargePointStatus() = 0;

            virtual bool notifyFirmwareUpdate(std::string firmwarePath) = 0;

            virtual bool downloadFile(const std::string &url,
                                      const std::string &filePath,
                                      int max_retries = 3,
                                      int retry_interval_sec = 1) = 0;

            virtual bool uploadFile(const std::string &url,
                                    const std::string &filePath,
                                    int max_retries = 3,
                                    int retry_interval_sec = 1) = 0;

            virtual UnlockStatus unlockConnector(unsigned int connectorId) = 0;

            virtual void transactionOperation(unsigned int connectorId, unsigned int action, unsigned int transactionId, unsigned int remoteTransactionId) = 0;
        };
    }
}
#endif // OPENOCPP_ICHARGEPOINTEVENTSHANDLER_H