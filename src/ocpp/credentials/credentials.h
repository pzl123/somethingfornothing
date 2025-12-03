#ifndef CREDENTIALS_H
#define CREDENTIALS_H
#include <string>

namespace ocpp1_6
{
    namespace auth
    {
        /**
         * @brief 存储用于连接的认证信息的结构体。
         *
         * 该结构体包含了进行连接时所需的各种认证凭据，包括用户凭据、TLS/SSL配置和密钥信息。
         */
        struct Credentials
        {
            // 用户名，用于数据库认证
            std::string user;

            // 密码，与用户名一起用于数据库认证
            std::string password;

            // TLS 1.2加密连接使用的密码列表
            std::string tls12_cipher_list;

            // TLS 1.3加密连接使用的密码列表
            std::string tls13_cipher_list;

            // 指示PEM格式证书是否被编码，用于TLS/SSL连接
            bool encoded_pem_certificates;

            // 服务器证书的CA（证书颁发机构），用于验证服务器证书
            std::string server_certificate_ca;

            // 客户端证书，用于TLS/SSL连接的互相认证
            std::string client_certificate;

            // 客户端证书的私钥，用于TLS/SSL连接
            std::string client_certificate_private_key;

            // 客户端证书私钥的密码短语，如果私钥被加密的话
            std::string client_certificate_private_key_passphrase;

            // 指示是否允许使用自签名证书
            bool allow_selfsigned_certificates;

            // 指示是否允许使用已过期的证书
            bool allow_expired_certificates;

            // 指示是否接受不受信任的证书，用于安全检查的放宽
            bool accept_untrusted_certificates;

            // 指示是否跳过服务器名称检查，以允许更灵活的TLS/SSL连接配置
            bool skip_server_name_check;
        };
    }
}
#endif // CREDENTIALS_H
