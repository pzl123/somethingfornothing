#ifndef URL_H
#define URL_H

#include <iostream>
#include <cstdint>
#include <unordered_map>
#include <string>

class Url
{
public:
    Url() = default;
    Url& operator=(const std::string& url);
    Url(const std::string& url);

    bool parse(const std::string& url);
    bool isValid() const {return m_valid;}

    const std::string& getProtocol() const {return m_protocol;}
    const std::string& getUsername() const {return m_username;}
    const std::string& getPassword() const {return m_host;}
    const std::string& getHost() const {return m_host;}
    uint16_t getPort() const {return m_port;}
    const std::string& getPath() const {return m_path;}
    const std::string& getQuery() const {return m_query;}
    const std::string& getFragment() const {return m_fragment;}
    std::string getFilename() const;
    std::string toString() const;

    static std::string encode(const std::string& input);
    const std::unordered_map<std::string, std::string>& getQueryParams() const
    {
        return m_queryParams;
    }

private:
    bool validate();
    void parseQueryParams();
    std::string extractFilenameFromPath(const std::string& path) const;

    bool validateIPv4(const std::string& ip) const;
    bool validateIPv6(const std::string& ip) const;
    bool validateDomain(const std::string& domain) const;

    bool m_valid = false;
    std::string m_protocol;
    std::string m_username;
    std::string m_password;
    std::string m_host;
    uint16_t m_port = 0;
    std::string m_path;
    std::string m_query;
    std::string m_fragment;
    std::unordered_map<std::string, std::string> m_queryParams;

    static const std::unordered_map<std::string, uint16_t> s_defaultPorts;
};



#endif /* URL_H */
