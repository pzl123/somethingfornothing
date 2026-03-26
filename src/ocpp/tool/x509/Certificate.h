#ifndef OCPP_CERTIFICATE_H
#define OCPP_CERTIFICATE_H



#include "x509Document.h"

namespace ocpp
{
    namespace x509
    {
        class Certificate : public x509Document
        {
        public:
            /**
             * @brief Constructor from PEM file
             * @param pem_file PEM file to load
             */
            Certificate(const std::filesystem::path& pem_file);

            /**
             * @brief Constructor from PEM data
             * @param pem_data PEM encoded data
             */
            Certificate(const std::string& pem_data);

            // /**
            //  * @brief Constructor from certificate request and signing certificate
            //  * @param certificate_request Certificate request to be signed
            //  * @param signing_certificate Certificate which will sign the request
            //  * @param private_key Private key of the certificate which will sign the request
            //  * @param sha Secure hash algorithm to use to sign the request
            //  * @param days New certficate validity in days
            //  */
            // Certificate(const CertificateRequest& certificate_request,
            //             const Certificate&        signing_certificate,
            //             const PrivateKey&         private_key,
            //             Sha2::Type                sha,
            //             unsigned int              days);
            /** @brief Destructor */
            virtual ~Certificate();

            bool isSelfSigned() const { return m_is_self_signed; }

            std::size_t get_pem_num_in_pem_chain() const { return m_pem_chain.capacity(); }
            std::size_t get_certificate_num_in_certificate_chain() const { return m_certificate_chain.capacity(); }
            const std::vector<Certificate>& get_certificate_chain() const { return m_certificate_chain; }
            const Certificate& get_certificate_chain(std::size_t cert_index) const { return m_certificate_chain.at(cert_index); }

        private:
            /** @brief PEM encoded data representation of each certificate composing the certificate chain (if any) */
            std::vector<std::string> m_pem_chain;
            /** @brief Certificates composing the certificate chain (if any) */
            std::vector<Certificate> m_certificate_chain;

            /** @brief Serial number */
            std::vector<uint8_t> m_serial_number;
            /** @brief Serial number as string */
            std::string m_serial_number_string;
            /** @brief Serial number as an hex string */
            std::string m_serial_number_hex_string;
            /** @brief Date of start of validity */
            time_t m_validity_from;
            /** @brief Date of end of validity */
            time_t m_validity_to;
            /** @brief Issuer */
            Subject m_issuer;
            /** @brief Issuer string */
            std::string m_issuer_string;
            /** @brief Issuer raw data */
            std::vector<uint8_t> m_issuer_der;

            /** @brief Indicate if it is a self-signed certificate */
            bool m_is_self_signed;

            /** @brief Extract all the PEM certificates in the certificate chain */
            void extractPemChain();
            /** @brief Load OpenSSL X509 certificate structure from a PEM encoded data string */
            static void* loadX509(const std::string& pem_data);
            /** @brief Read X509 informations stored inside a certificate */
            static void readInfos(Certificate& certificate);

            /** @brief Verify a certificate against a chain of certificates 证书链验证证书 */
            static bool verify(const Certificate& certificate, const std::vector<Certificate>& certificate_chain, size_t start_index);
        };
    } // namespace x509
} // namespace ocpp

#endif /* OCPP_CERTIFICATE_H */
