/**
 * @file security_datatypes.h
 * @brief OCPP 1.6 安全模块核心数据类型定义
 * 包含证书管理、固件更新、日志处理、安全事件、消息触发等相关枚举/结构体，
 * 全量覆盖OCPP 1.6-J安全扩展规范的类型需求，用于充电桩与中心系统的安全交互
 */

#ifndef SECURITY_DATATYPES_H
#define SECURITY_DATATYPES_H

#include <string>
#include <vector>
#include <optional>
#include <stdexcept>

namespace ocpp1_6
{

    /**
     * @brief OCPP规范定义的受限字符串类型（最大40字符）
     * 用于需要严格长度限制的证书相关字段，构造时会检查长度
     */
    struct CiString40Type : public std::string
    {
        CiString40Type(const std::string &s)
        {
            if (s.size() > 40)
            {
                throw std::invalid_argument("CiString40Type: string length > 40");
            }
            assign(s);
        }
    };

    /**
     * @brief OCPP规范定义的受限字符串类型（最大128字符）
     * 用于需要严格长度限制的证书相关字段，构造时会检查长度
     */
    struct CiString128Type : public std::string
    {
        CiString128Type(const std::string &s)
        {
            if (s.size() > 128)
            {
                throw std::invalid_argument("CiString128Type: string length > 128");
            }
            assign(s);
        }
    };

    /**
     * @brief 证书类型
     */
    enum class CertificateUseEnumType
    {
        CentralSystemRootCertificate,
        ManufacturerRootCertificate,
    };

    /**
     * @brief OCPP 1.6 安全事件码枚举
     * 定义了充电桩（Charge Point）可能触发的所有安全事件标识，
     * 用于安全事件上报、日志记录及状态管理等场景
     */
    enum class SecurityEventCode
    {
        FirmwareUpdated,                     ///< 充电桩固件已更新
        FailedToAuthenticateAtCentralSystem, ///< 充电桩在中心系统的认证失败（充电桩凭证被拒绝）
        CentralSystemFailedToAuthenticate,   ///< 中心系统在充电桩的认证失败（中心系统凭证被拒绝）
        SettingSystemTime,                   ///< 充电桩系统时间被修改
        StartupOfTheDevice,                  ///< 充电桩设备启动（上电或重启后初始化完成）
        ResetOrReboot,                       ///< 充电桩被重置或重启
        SecurityLogWasCleared,               ///< 安全日志被清除
        ReconfigurationOfSecurityParameters, ///< 安全参数重新配置（如密钥、安全配置文件变更）
        MemoryExhaustion,                    ///< 充电桩Flash或RAM内存耗尽
        InvalidMessages,                     ///< 接收到无效消息（格式错误或签名无效）
        AttemptedReplayAttacks,              ///< 检测到重放攻击尝试（排除正常重传场景）
        TamperDetectionActivated,            ///< 物理篡改检测传感器被触发
        InvalidFirmwareSignature,            ///< 固件签名无效
        InvalidFirmwareSigningCertificate,   ///< 用于验证固件签名的证书无效
        InvalidCentralSystemCertificate,     ///< 中心系统使用的证书无效或无法验证
        InvalidChargePointCertificate,       ///< 通过SignCertificate.conf消息发送的充电桩证书无效
        InvalidTLSVersion,                   ///< 中心系统使用的TLS版本低于1.2且不被安全规范允许
        InvalidTLSCipherSuite                ///< 中心系统仅允许使用不被安全规范允许的TLS密码套件
    };

    /**
     * @brief 固件状态枚举类型，用于SignedFirmwareStatusNotification.req消息
     * 描述固件下载、安装过程中的各种状态，包括：
     * - 中间状态（Intermediate state）：更新过程未完成
     * - 失败结束状态（Failure end state）：更新过程停止且失败
     * - 成功结束状态（Successful end state）：更新过程停止且成功
     */
    enum class FirmwareStatusEnumType
    {
        Downloaded,                ///< 中间状态：充电桩已成功下载新固件
        DownloadFailed,            ///< 失败结束状态：充电桩下载固件失败
        Downloading,               ///< 中间状态：固件正在下载中
        DownloadScheduled,         ///< 中间状态：新固件下载已被安排
        DownloadPaused,            ///< 中间状态：固件下载已暂停
        Idle,                      ///< 充电桩未执行固件更新相关任务（仅在由ExtendedTriggerMessage.req触发的通知中使用）
        InstallationFailed,        ///< 失败结束状态：新固件安装失败
        Installing,                ///< 中间状态：固件正在安装中
        Installed,                 ///< 成功结束状态：新固件已在充电桩中成功安装
        InstallRebooting,          ///< 中间状态：充电桩即将重启以激活新固件（若重启是安装的必要部分且无法单独上报，可省略此状态）
        InstallScheduled,          ///< 中间状态：已下载固件的安装已按SignedUpdateFirmware.req中的installDateTime安排
        InstallVerificationFailed, ///< 失败结束状态：新固件验证（如校验和等方式）失败，安装将终止
        InvalidSignature,          ///< 失败结束状态：固件签名无效
        SignatureVerified          ///< 中间状态：固件签名已成功验证
    };

    /**
     * @brief 证书签名状态枚举类型，用于CertificateSigned.conf消息
     * 表示充电桩对中心系统发送的签名证书的验证结果
     */
    enum class CertificateSignedStatusEnumType
    {
        Accepted, ///< 签名证书有效，已被充电桩接受
        Rejected  ///< 签名证书无效，被充电桩拒绝
    };

    /**
     * @brief 证书删除状态枚举类型，用于DeleteCertificate.conf消息
     * 表示充电桩对证书删除操作的处理结果
     */
    enum class DeleteCertificateStatusEnumType
    {
        Accepted, ///< 正常成功完成（无错误），证书已被删除
        Failed,   ///< 处理失败，证书未被删除
        NotFound
    };

    /**
     * @brief 获取已安装证书状态枚举类型，用于GetInstalledCertificateIds.conf消息
     * 表示充电桩对获取已安装证书ID请求的处理结果
     */
    enum class GetInstalledCertificateStatusEnumType
    {
        Accepted, ///< 正常成功完成（无错误），已返回请求的证书ID列表
        NotFound  ///< 请求的证书未找到，无法返回对应ID
    };

    /**
     * @brief 哈希算法枚举类型，用于CertificateHashDataType
     * 定义证书哈希计算时所使用的哈希算法类型
     */
    enum class HashAlgorithmEnumType
    {
        SHA256, ///< SHA-256哈希算法
        SHA384, ///< SHA-384哈希算法
        SHA512  ///< SHA-512哈希算法
    };

    /**
     * @brief 日志类型枚举，用于GetLog.req消息
     * 定义可请求上传的日志类型
     */
    enum class LogEnumType
    {
        DiagnosticsLog, ///< 包含诊断日志文件的字段定义
        SecurityLog     ///< 由中心系统发送给充电桩，用于请求充电桩上传安全日志
    };

    /**
     * @brief 日志参数类型结构体，用于GetLog.req消息
     * 包含日志检索的详细信息，如日志存储位置、时间范围等
     */
    struct LogParametersType
    {
        /**
         * @brief 远程系统中存储日志的URL
         * 类型：string[0..512]（最大512字符的字符串），必填（1..1）
         */
        std::string remoteLocation;

        /**
         * @brief 包含在诊断中的最旧日志的日期时间
         * 类型：dateTime（日期时间格式字符串），可选（0..1）
         */
        std::string oldestTimestamp;

        /**
         * @brief 包含在诊断中的最新日志的日期时间
         * 类型：dateTime（日期时间格式字符串），可选（0..1）
         */
        std::string latestTimestamp;
    };

    /**
     * @brief 日志状态枚举类型，用于GetLog.conf消息
     * 表示充电桩对日志上传请求（GetLog.req）的处理结果
     */
    enum class LogStatusEnumType
    {
        Accepted,        ///< 接受日志上传请求（不表示上传成功，充电桩将开始日志文件上传）
        Rejected,        ///< 拒绝日志上传请求
        AcceptedCanceled ///< 接受日志上传请求，但同时取消了正在进行的日志文件上传
    };

    /**
     * @brief 证书状态枚举类型，用于InstallCertificate.conf消息
     * 表示充电桩对证书安装操作的处理结果及状态
     */
    enum class CertificateStatusEnumType
    {
        Accepted, ///< 证书安装成功
        Failed,   ///< 证书有效且正确，但因其他原因导致安装失败
        Rejected  ///< 证书无效和/或不正确，或CPO尝试安装的证书数量超过允许上限
    };

    /**
     * @brief 日志上传状态枚举类型，用于LogStatusNotification.req消息
     * 表示充电桩上传日志文件过程中的各种状态
     */
    enum class UploadLogStatusEnumType
    {
        BadMessage,            ///< 检测到格式错误的数据包或其他协议不兼容问题
        Idle,                  ///< 充电桩未在上传日志文件（仅当消息由ExtendedTriggerMessage.req触发时使用）
        NotSupportedOperation, ///< 服务器不支持当前操作
        PermissionDenied,      ///< 权限不足，无法执行该操作
        Uploaded,              ///< 日志文件已成功上传
        UploadFailure,         ///< 日志文件上传失败
        Uploading              ///< 日志文件正在上传过程中
    };

    /**
     * @brief 通用消息响应状态枚举
     * 用于表示对请求的处理结果，适用于多种通用消息的响应场景
     */
    enum class GenericStatusEnumType
    {
        Accepted, ///< 请求已被接受并将执行
        Rejected  ///< 请求未被接受且不会执行
    };

    /**
     * @brief 固件更新状态枚举类型，用于SignedUpdateFirmware.conf消息
     * 表示充电桩对固件更新请求（SignedUpdateFirmware.req）的处理结果
     */
    enum class UpdateFirmwareStatusEnumType
    {
        Accepted,           ///< 接受固件更新请求（不表示更新成功，充电桩将开始固件更新流程）
        Rejected,           ///< 拒绝固件更新请求
        AcceptedCanceled,   ///< 接受固件更新请求，但同时取消了正在进行的固件更新
        InvalidCertificate, ///< 证书无效
        RevokedCertificate  ///< 失败结束状态：固件签名证书已被吊销
    };

    /**
     * @brief 消息触发类型枚举（MessageTriggerEnumType）
     * 用于ExtendedTriggerMessage.req消息，定义可通过触发请求主动发起的各类OCPP消息类型，
     * 中心系统可通过指定该枚举值，要求充电桩立即发送特定类型的请求消息
     */
    enum class MessageTriggerEnumType
    {
        BootNotification,           ///< 触发BootNotification.req消息，要求充电桩发送设备启动通知
        LogStatusNotification,      ///< 触发LogStatusNotification.req消息，要求充电桩上报日志上传状态
        FirmwareStatusNotification, ///< 触发SignedFirmwareStatusNotification.req消息，要求充电桩上报安全固件更新状态
                                    ///< （对应本文档中引入的安全固件更新机制的状态）
        Heartbeat,                  ///< 触发Heartbeat.req消息，要求充电桩立即发送心跳包
        MeterValues,                ///< 触发MeterValues.req消息，要求充电桩上报计量数据
        SignChargePointCertificate, ///< 触发SignCertificate.req消息，且证书类型为ChargePointCertificate，
                                    ///< 要求充电桩发起充电桩证书的签名请求
        StatusNotification          ///< 触发StatusNotification.req消息，要求充电桩上报设备状态通知
    };

    /**
     * @brief 触发消息状态枚举（TriggerMessageStatusEnumType）
     * OCPP 1.6 6.17节定义，覆盖触发请求的三类核心处理结果，确保与协议交互逻辑对齐
     */
    enum class TriggerMessageStatusEnumType
    {
        Accepted,      ///< 处理结果：触发请求已接受
                       ///< 协议含义：充电桩确认将发送请求指定的OCPP消息（如BootNotification.req、MeterValues.req等）
                       ///< 适用场景：请求合法、充电桩当前状态支持发送目标消息（如在线、无资源冲突）
        Rejected,      ///< 处理结果：触发请求已拒绝
                       ///< 协议含义：充电桩不会发送请求指定的OCPP消息
                       ///< 常见原因：中心系统权限不足、充电桩离线/故障、当前操作与设备状态冲突（如正在执行固件更新）
        NotImplemented ///< 处理结果：触发请求无法执行
                       ///< 协议含义：请求的消息无法发送，仅因两种明确原因：
                       ///< 1. 目标消息类型在当前充电桩中未实现（如低版本设备不支持LogStatusNotification）；
                       ///< 2. 充电桩无法识别请求的消息类型（如非法/不存在的消息标识）
    };

    /**
     * @brief 证书哈希数据结构体（对应CertificateHashDataType）
     * 用于唯一标识待删除的证书，所有字段均为必选且有长度限制
     */
    struct CertificateHashDataType
    {
        // 哈希算法（必选）
        HashAlgorithmEnumType hashAlgorithm;

        // 颁发者名称哈希（必选，Base64编码，最大128字符）
        std::string issuerNameHash;

        // 颁发者公钥哈希（必选，Base64编码，最大128字符）
        std::string issuerKeyHash;

        // 证书序列号（必选，最大40字符，通常为十六进制字符串）
        std::string serialNumber;

        /**
         * @brief 构造函数（确保必选字段初始化）
         * @param algorithm 哈希算法
         * @param nameHash 颁发者名称哈希
         * @param keyHash 颁发者公钥哈希
         * @param serial 证书序列号
         * @throw std::invalid_argument 当字段长度超出限制时抛出
         */
        CertificateHashDataType()
            : hashAlgorithm(HashAlgorithmEnumType::SHA256),
              issuerNameHash(""),
              issuerKeyHash(""),
              serialNumber("")
        {
            // 验证字段长度约束
            if (issuerNameHash.length() > 128)
            {
                throw std::invalid_argument("issuerNameHash exceeds maximum length of 128 characters");
            }
            if (issuerKeyHash.length() > 128)
            {
                throw std::invalid_argument("issuerKeyHash exceeds maximum length of 128 characters");
            }
            if (serialNumber.length() > 40)
            {
                throw std::invalid_argument("serialNumber exceeds maximum length of 40 characters");
            }
        }
    };

    /**
     * @brief 固件类型结构体，用于SignedUpdateFirmware.req消息
     * 表示可在充电桩上加载/更新的固件副本信息，包含固件来源、检索时间、签名等关键信息
     */
    struct FirmwareType
    {
        /**
         * @brief 定义固件来源的URI
         * 类型：string[0..512]（最大512字符的字符串），必填（1..1）
         */
        std::string location;

        /**
         * @brief 固件应被检索的日期时间
         * 类型：dateTime（日期时间格式字符串），必填（1..1）
         */
        std::string retrieveDateTime;

        /**
         * @brief 固件应被安装的日期时间
         * 类型：dateTime（日期时间格式字符串），可选（0..1）
         */
        std::string installDateTime;

        /**
         * @brief 用于签名固件的证书
         * 类型：string[0..5500]（最大5500字符的字符串），必填（1..1）
         * 说明：PEM编码的X.509证书
         */
        std::string signingCertificate;

        /**
         * @brief Base64编码的固件签名
         * 类型：string[0..800]（最大800字符的字符串），必填（1..1）
         */
        std::string signature;
    };

} // namespace ocpp1_6

#endif // SECURITY_DATATYPES_H
