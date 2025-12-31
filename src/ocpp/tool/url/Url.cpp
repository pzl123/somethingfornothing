#include "Url.h"
#include <regex>
#include <sstream>
#include <iomanip>
#include <cctype>
#include <unordered_map>
#include <arpa/inet.h> // inet_pton

const std::unordered_map<std::string, uint16_t> Url::s_defaultPorts = {
    {"http", 80},
    {"https", 443},
    {"ftp", 21},
    {"ftps", 990},
    {"sftp", 22},
    {"ws", 80},
    {"wss", 443}};

Url::Url(const std::string &url)
{
    parse(url);
}

Url &Url::operator=(const std::string &url)
{
    parse(url);
    return *this;
}

bool Url::parse(const std::string &url)
{
    m_valid = false;

    static const std::regex urlRegex(
        R"(^([a-zA-Z][a-zA-Z0-9+\-.]*)://)" // 协议 (1)
        R"((?:([^:@]+)(?::([^@]*))?@)?)"    // 用户名 (2), 密码 (3)
        R"((\[[^\]]+\]|[^:/?#]+))"          // 主机 (IPv6 或普通域名) (4)
        R"((?::(\d+))?)"                    // 端口 (5)
        R"(([^?#]*))"                       // 路径 (6)
        R"((?:\?([^#]*))?)"                 // 查询 (7)
        R"((?:#(.*))?$)"                    // 片段 (8)
    );

    std::smatch match;
    if (!std::regex_match(url, match, urlRegex))
    {
        return false;
    }

    m_protocol = match[1].str();
    m_username = match[2].str();
    m_password = match[3].str();
    m_host = match[4].str();

    if (!match[5].str().empty())
    {
        m_port = static_cast<uint16_t>(std::stoi(match[5].str()));
    }
    else
    {
        auto it = s_defaultPorts.find(m_protocol);
        if (it != s_defaultPorts.end())
        {
            m_port = it->second;
        }
        else
        {
            m_port = 0; // 未知协议没有默认端口
        }
    }

    m_path = match[6].str().empty() ? "/" : match[6].str();
    m_query = match[7].str();
    m_fragment = match[8].str();

    parseQueryParams();
    m_valid = validate();
    return m_valid;
}

std::string Url::getFilename() const
{
    if (!m_valid)
        return "";
    return extractFilenameFromPath(m_path);
}

std::string Url::extractFilenameFromPath(const std::string &path) const
{
    if (path.empty() || path == "/")
    {
        return "";
    }
    std::string trimmed = path;
    if (!trimmed.empty() && trimmed.back() == '/')
    {
        trimmed.pop_back();
    }
    auto pos = trimmed.find_last_of('/');
    return (pos == std::string::npos) ? trimmed : trimmed.substr(pos + 1);
}

void Url::parseQueryParams()
{
    m_queryParams.clear();
    if (m_query.empty())
        return;
    std::istringstream ss(m_query);
    std::string param;
    while (std::getline(ss, param, '&'))
    {
        auto pos = param.find('=');
        if (pos != std::string::npos)
        {
            m_queryParams[param.substr(0, pos)] = param.substr(pos + 1);
        }
        else
        {
            m_queryParams[param] = "";
        }
    }
}

bool Url::validate()
{
    if (m_host.empty())
        return false;
    if (!m_protocol.size())
        return false;

    if (m_host.front() == '[' && m_host.back() == ']')
    {
        return validateIPv6(m_host.substr(1, m_host.size() - 2));
    }
    else if (std::regex_match(m_host, std::regex(R"(\d+\.\d+\.\d+\.\d+)")))
    {
        return validateIPv4(m_host);
    }
    else
    {
        return validateDomain(m_host);
    }
}

bool Url::validateIPv4(const std::string &ip) const
{
    std::istringstream ss(ip);
    std::string part;
    int count = 0;
    while (std::getline(ss, part, '.'))
    {
        if (++count > 4)
            return false;
        if (part.empty() || part.size() > 3)
            return false;
        for (char c : part)
            if (!isdigit(c))
                return false;
        int num = std::stoi(part);
        if (num < 0 || num > 255)
            return false;
    }
    return count == 4;
}

bool Url::validateIPv6(const std::string &ip) const
{
    struct in6_addr addr6;
    return inet_pton(AF_INET6, ip.c_str(), &addr6) == 1;
}

bool Url::validateDomain(const std::string &domain) const
{
    static const std::regex domainRegex(
        R"(^([a-zA-Z0-9]([a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?)(\.([a-zA-Z0-9]([a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?))*$)");
    return std::regex_match(domain, domainRegex);
}

std::string Url::toString() const
{
    if (!m_valid)
        return "";
    std::string out = m_protocol + "://";
    if (!m_username.empty())
    {
        out += m_username;
        if (!m_password.empty())
            out += ":" + m_password;
        out += "@";
    }
    out += m_host;
    auto it = s_defaultPorts.find(m_protocol);
    uint16_t defaultPort = (it != s_defaultPorts.end()) ? it->second : 0;
    if (m_port != 0 && m_port != defaultPort)
    {
        out += ":" + std::to_string(m_port);
    }
    out += m_path;
    if (!m_query.empty())
        out += "?" + m_query;
    if (!m_fragment.empty())
        out += "#" + m_fragment;
    return out;
}

/**
 * @brief 对 URL 的部分（如路径、参数）进行 RFC3986 百分比编码
 * @param str 要编码的字符串
 * @return 编码后的字符串
 */
std::string Url::encode(const std::string &str)
{
    static const char unreserved_char[] = {'-', '_', '.', '~'};

    std::stringstream encoded;
    encoded << std::uppercase << std::hex;

    for (const unsigned char c : str)
    {
        bool shouldEncode = true;

        if (isalnum(c))
        {
            shouldEncode = false;
        }
        else
        {
            for (char u : unreserved_char)
            {
                if (c == static_cast<unsigned char>(u))
                {
                    shouldEncode = false;
                    break;
                }
            }
        }

        if (shouldEncode)
        {
            encoded << '%' << std::setw(2) << std::setfill('0') << int(c);
        }
        else
        {
            encoded << c;
        }
    }
    return encoded.str();
}
