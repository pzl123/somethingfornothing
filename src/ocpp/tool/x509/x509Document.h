#ifndef OCPP_X509DOCUMENT_H
#define OCPP_X509DOCUMENT_H


#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>
#include <iostream>


namespace ocpp
{
    namespace x509
    {
        class x509Document
        {
        public:
            struct Subject
            {
                std::string country;
                std::string state;
                std::string location;
                std::string organization;
                std::string organization_unit;
                std::string common_name;
                std::string email_address;
            };

            /* 基础限制 */
            struct BasicConstraints
            {
                BasicConstraints() : present(false), is_ca(false), path_length(0) {};

                /** @brief Indicate if the extension is present */
                bool present;
                /** @brief Indicate if CA = true*/
                bool is_ca;
                /** @brief Path length */
                unsigned int path_length;
            };

            /* 拓展 */
            struct Extensions
            {
                /** @brief Basic constraints */
                BasicConstraints basic_constraints;
                /** @brief Issuer alternate names */
                std::vector<std::string> issuer_alternate_names;
                /** @brief Subject alternate names */
                std::vector<std::string> subject_alternate_names;
            };

            /**
             * @brief Constructor from PEM file
             * @param pem_file PEM file to load
             */
            x509Document(const std::filesystem::path& pem_file);
        protected:
            /** @brief Indicate if the document is valid */
            bool m_is_valid;
            /** @brief PEM encoded data representation of the document */
            std::string m_pem;

            /** @brief Subject */
            Subject m_subject;
            /** @brief Subject string */
            std::string m_subject_string;
            /** @brief Signature algorithm */
            std::string m_sig_algo;
            /** @brief Signature hash */
            std::string m_sig_hash;
            /** @brief Public key */
            std::vector<uint8_t> m_pub_key;
            /** @brief Public key as hexadecimal string */
            std::string m_pub_key_string;
            /** @brief Size of the public key in bits */
            unsigned int m_pub_key_size;
            /** @brief Public key algorithm */
            std::string m_pub_key_algo;
            /** @brief Public key algorithm parameter */
            std::string m_pub_key_algo_param;
            /** @brief X509v3 extensions */
            Extensions m_x509v3_extensions;
            /** @brief X509v3 extensions names*/
            std::vector<std::string> m_x509v3_extensions_names;

            /** @brief Internal OpenSSL object */
            void* m_openssl_object;

            /** @brief Parse a public key */
            void parsePublicKey(void* ppub_key);
            /** @brief Convert a date in ASN1_TIME format to a standard time_t representation */
            static time_t convertAsn1Time(const void* pasn1_time);
            /** @brief Convert a string in ASN1_STRING format to a standard string representation */
            static std::string convertAsn1String(const void* pasn1_string);
            /** @brief Convert a string in X509_NAME format to a standard string representation */
            static std::string convertX509Name(const void* px509_name);
            /** @brief Convert a list of strings in GENERAL_NAMES format to a standard vector of strings representation */
            static std::vector<std::string> convertGeneralNames(void* pgeneral_names);
            /** @brief Parse a subject's string */
            static void parseSubjectString(const void* px509_name, Subject& subject);

        };
    }
}


#endif /* OCPP_X509DOCUMENT_H */
