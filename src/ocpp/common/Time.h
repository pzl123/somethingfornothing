#ifndef DATETIME_H
#define DATETIME_H

#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>

namespace ocpp1_6
{
    namespace Time
    {

        /**
         * @brief 日期和时间表示类（字符串表示为ISO-8601格式）
         */
        class DateTime
        {
        public:
            /**
             * @brief 使用当前日期和时间实例化一个日期时间对象
             * @return 实例化的日期时间对象
             */
            static DateTime now() { return DateTime(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())); }

            /**
             * @brief 默认构造函数
             */
            DateTime() : m_datetime(0) {}

            /**
             * @brief 使用std::time_t构造函数
             * @param init std::time_t类型的时间
             */
            DateTime(const std::time_t &init) : m_datetime(init) {}

            /**
             * @brief 拷贝构造函数
             * @param copy 需要复制的对象
             */
            DateTime(const DateTime &copy) : m_datetime(copy.m_datetime) {}

            /**
             * @brief 赋值运算符
             * @param copy 需要赋值的对象
             * @return 返回自身引用
             */
            DateTime &operator=(const DateTime &copy)
            {
                m_datetime = copy.m_datetime;
                return (*this);
            }

            /**
             * @brief 从UTC时间字符串表示中赋值一个新的日期时间
             * @param value 时间的字符串表示
             * @return 如果字符串表示符合最大字符串长度则返回true，否则返回false
             */
            bool assign(const std::string &value)
            {
                bool ret = false;
                std::istringstream ss(value);
                std::tm t = {};
                ss >> std::get_time(&t, "%Y-%m-%dT%TZ");
                if (ss.fail())
                {
                    ss.clear();
                    ss.str(value);
                    ss >> std::get_time(&t, "%Y-%m-%dT%T");
                }
                if (!ss.fail())
                {
#ifdef _MSC_VER
                    m_datetime = _mkgmtime(&t);
#else  // _MSC_VER
                    m_datetime = std::mktime(&t);
                    m_datetime += t.tm_gmtoff;
                    m_datetime -= (t.tm_isdst * 3600);
#endif // _MSC_VER
                    ret = true;
                }
                return ret;
            }

            /**
             * @brief 隐式转换运算符
             * @return 返回底层的std::time_t类型的日期时间
             */
            operator const std::time_t &() const { return m_datetime; }

            /**
             * @brief 时间间隔加法运算
             * @param duration 要添加的时间间隔（秒）
             * @return 相加后的DateTime对象
             */
            DateTime operator+(const std::chrono::seconds &duration) const
            {
                return DateTime(m_datetime + std::chrono::duration_cast<std::chrono::seconds>(duration).count());
            }

            /**
             * @brief 时间间隔减法运算
             * @param duration 要减去的时间间隔（秒）
             * @return 相减后的DateTime对象
             */
            DateTime operator-(const std::chrono::seconds &duration) const
            {
                return DateTime(m_datetime - std::chrono::duration_cast<std::chrono::seconds>(duration).count());
            }

            /**
             * @brief 隐式比较运算符
             * @param value 要比较的值
             * @return 如果两个日期时间相等返回true，否则返回false
             */
            bool operator==(const std::time_t &value) const { return (value == m_datetime); }

            /**
             * @brief 隐式比较运算符
             * @param value 要比较的值
             * @return 如果两个日期时间不相等返回true，否则返回false
             */
            bool operator!=(const std::time_t &value) const { return (value != m_datetime); }

            /**
             * @brief 隐式比较运算符
             * @param value 要比较的值
             * @return 如果当前日期时间小于给定的日期时间返回true，否则返回false
             */
            bool operator<(const std::time_t &value) const { return (m_datetime < value); }

            /**
             * @brief 隐式比较运算符
             * @param value 要比较的值
             * @return 如果当前日期时间大于给定的日期时间返回true，否则返回false
             */
            bool operator>(const std::time_t &value) const { return (m_datetime > value); }

            /**
             * @brief 隐式比较运算符
             * @param value 要比较的值
             * @return 如果当前日期时间小于等于给定的日期时间返回true，否则返回false
             */
            bool operator<=(const std::time_t &value) const { return (m_datetime <= value); }

            /**
             * @brief 隐式比较运算符
             * @param value 要比较的值
             * @return 如果当前日期时间大于等于给定的日期时间返回true，否则返回false
             */
            bool operator>=(const std::time_t &value) const { return (m_datetime >= value); }

            /**
             * @brief 比较运算符
             * @param value 要比较的值
             * @return 如果当前日期时间小于给定的日期时间返回true，否则返回false
             */
            bool operator<(const DateTime &value) const { return (m_datetime < value.m_datetime); }

            /**
             * @brief 比较运算符
             * @param value 要比较的值
             * @return 如果当前日期时间大于给定的日期时间返回true，否则返回false
             */
            bool operator>(const DateTime &value) const { return (m_datetime > value.m_datetime); }

            /**
             * @brief 比较运算符
             * @param value 要比较的值
             * @return 如果当前日期时间小于等于给定的日期时间返回true，否则返回false
             */
            bool operator<=(const DateTime &value) const { return (m_datetime <= value.m_datetime); }

            /**
             * @brief 比较运算符
             * @param value 要比较的值
             * @return 如果当前日期时间大于等于给定的日期时间返回true，否则返回false
             */
            bool operator>=(const DateTime &value) const { return (m_datetime >= value.m_datetime); }

            /**
             * @brief 获取日期时间的字符串表示（ISO-8601格式）
             * @return 日期时间的字符串表示（ISO-8601格式）
             */
            std::string str() const
            {
                std::ostringstream ss;
                std::tm t = {};
#ifdef _MSC_VER
                gmtime_s(&t, &m_datetime);
#else  // _MSC_VER
                gmtime_r(&m_datetime, &t);
#endif // _MSC_VER
                ss << std::put_time(&t, "%Y-%m-%dT%TZ");
                return ss.str();
            }

            /**
             * @brief 获取与当前日期时间对应的UNIX时间戳
             * @return UNIX时间戳
             */
            std::time_t timestamp() const { return m_datetime; }

            /**
             * @brief 判断日期时间是否为空（即为纪元时间）
             * @return 如果日期时间为空（为纪元时间），则返回true，否则返回false
             */
            bool empty() const { return (m_datetime == 0); }

        private:
            /**
             * @brief 底层的日期时间（本地时间）
             */
            std::time_t m_datetime;
        };

    } // namespace Time
} // namespace ocpp1_6

#endif // DATETIME_H
