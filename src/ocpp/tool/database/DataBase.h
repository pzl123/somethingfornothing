#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <memory>
#include <vector>

#include "sqlite3/sqlite3.h"

#include "utils/utils.h"

class Database
{
public:
    // 前向声明
    class Query;

    /** @brief 构造函数 */
    Database();
    /** @brief 析构函数 */
    virtual ~Database();

    /**
     * @brief 打开数据库
     * @param database_path 数据库文件路径
     * @return 如果数据库存在则返回 true，否则返回 false
     */
    bool open(const std::string &database_path);

    /**
     * @brief 关闭数据库
     * @return 如果数据库存在则返回 true，否则返回 false
     */
    bool close();

    /**
     * @brief 创建要在数据库上执行的新查询
     * @param sql 要执行的 SQL 查询
     * @return 如果数据库已打开且查询有效，则返回创建的查询对象，否则返回 nullptr
     */
    std::unique_ptr<Query> query(const std::string &sql);

    std::string sanitizeIdentifier(const std::string &input);

    /**
     * @brief 获取解释最近错误的字符串
     * @return 解释最近错误的字符串
     */
    std::string lastError() const;

    /** @brief 表示要在数据库上执行的查询 */
    class Query
    {
    public:
        /** @brief 构造函数 */
        Query(Database &database, sqlite3_stmt *stmt);
        /** @brief 析构函数 */
        virtual ~Query();

        /** @brief 重置查询，以便可以重用于另一次执行 */
        void reset();

        /**
         * @brief 将 NULL 值绑定到查询参数
         * @param number 查询中参数的编号
         * @return 如果绑定完成则返回 true，否则返回 false
         */
        bool bind(int number);

        /**
         * @brief 将 blob 值绑定到查询参数
         * @param number 查询中参数的编号
         * @param value 要绑定的值
         * @return 如果绑定完成则返回 true，否则返回 false
         */
        bool bind(int number, const std::vector<uint8_t> &value);

        /**
         * @brief 将布尔值绑定到查询参数
         * @param number 查询中参数的编号
         * @param value 要绑定的值
         * @return 如果绑定完成则返回 true，否则返回 false
         */
        bool bind(int number, bool value);

        /**
         * @brief 将浮点数值绑定到查询参数
         * @param number 查询中参数的编号
         * @param value 要绑定的值
         * @return 如果绑定完成则返回 true，否则返回 false
         */
        bool bind(int number, double value);

        /**
         * @brief 将 32 位有符号整数值绑定到查询参数
         * @param number 查询中参数的编号
         * @param value 要绑定的值
         * @return 如果绑定完成则返回 true，否则返回 false
         */
        bool bind(int number, int32_t value);

        /**
         * @brief 将 32 位无符号整数值绑定到查询参数
         * @param number 查询中参数的编号
         * @param value 要绑定的值
         * @return 如果绑定完成则返回 true，否则返回 false
         */
        bool bind(int number, uint32_t value);

        /**
         * @brief 将 64 位有符号整数值绑定到查询参数
         * @param number 查询中参数的编号
         * @param value 要绑定的值
         * @return 如果绑定完成则返回 true，否则返回 false
         */
        bool bind(int number, int64_t value);

        /**
         * @brief 将 64 位无符号整数值绑定到查询参数
         * @param number 查询中参数的编号
         * @param value 要绑定的值
         * @return 如果绑定完成则返回 true，否则返回 false
         */
        bool bind(int number, uint64_t value);

        /**
         * @brief 将字符串值绑定到查询参数
         * @param number 查询中参数的编号
         * @param value 要绑定的值
         * @return 如果绑定完成则返回 true，否则返回 false
         */
        bool bind(int number, const std::string &value);

        /**
         * @brief 将字符串值绑定到查询参数
         * @param number 查询中参数的编号
         * @param value 要绑定的值
         * @return 如果绑定完成则返回 true，否则返回 false
         */
        bool bind(int number, const char *value);

        /**
         * @brief 执行查询
         * @return 如果查询执行无错误则返回 true，否则返回 false
         */
        bool exec();

        /**
         * @brief 指示查询结果是否有要提取数据的行
         * @return 如果查询结果有行则返回 true，否则返回 false
         */
        bool hasRows() const;

        /**
         * @brief 获取查询结果的下一个值
         * @return 如果下一个值存在则返回 true，否则返回 false
         */
        bool next();

        /**
         * @brief 获取解释最近错误的字符串
         * @return 解释最近错误的字符串
         */
        std::string lastError() const;

        /**
         * @brief 指示查询结果中的某个值是否为 NULL
         * @param column 查询结果中值的列号
         * @return 如果该值为 NULL 则返回 true，否则返回 false
         */
        bool isNull(int column) const;

        /**
         * @brief 从查询结果中获取 blob 值
         * @param column 查询结果中值的列号
         * @return blob 值的指针
         */
        std::vector<uint8_t> getBlob(int column) const;

        /**
         * @brief 从查询结果中获取布尔值
         * @param column 查询结果中值的列号
         * @return 布尔值
         */
        bool getBool(int column) const;

        /**
         * @brief 从查询结果中获取浮点数值
         * @param column 查询结果中值的列号
         * @return 浮点数值
         */
        double getFloat(int column) const;

        /**
         * @brief 从查询结果中获取 32 位有符号整数值
         * @param column 查询结果中值的列号
         * @return 32 位有符号整数值
         */
        int32_t getInt32(int column) const;

        /**
         * @brief 从查询结果中获取 32 位无符号整数值
         * @param column 查询结果中值的列号
         * @return 32 位无符号整数值
         */
        uint32_t getUInt32(int column) const;

        /**
         * @brief 从查询结果中获取 64 位有符号整数值
         * @param column 查询结果中值的列号
         * @return 64 位有符号整数值
         */
        int64_t getInt64(int column) const;

        /**
         * @brief 从查询结果中获取 64 位无符号整数值
         * @param column 查询结果中值的列号
         * @return 64 位无符号整数值
         */
        uint64_t getUInt64(int column) const;

        /**
         * @brief 从查询结果中获取字符串值
         * @param column 查询结果中值的列号
         * @return 字符串值
         */
        std::string getString(int column) const;

    private:
        /** @brief 关联的数据库 */
        Database &m_database;
        /** @brief 语句句柄 */
        sqlite3_stmt *m_stmt;
        /** @brief 指示查询结果是否有行以提取数据 */
        bool m_has_rows;
    };

private:
    /** @brief 数据库句柄 */
    sqlite3 *m_db;
};

#endif /* DATABASE_H */
