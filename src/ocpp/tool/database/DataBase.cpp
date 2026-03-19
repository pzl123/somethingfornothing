
#include "DataBase.h"
#include "utils/utils.h"
#include <cstring>
#include "DBExecutor.h"

Database::Database() : m_db(nullptr) { }

Database::~Database()
{
    close();
}

bool Database::open(const std::string& database_path)
{
    return DBExecutor::instance().postAndWait(
        [&]()
        {
            if (m_db)
                return true;

            i_log("[SQLite3] path: %s", database_path.c_str());

            int code =
                sqlite3_open_v2(database_path.c_str(), &m_db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX, nullptr);

            if (code != SQLITE_OK)
            {
                w_log("[SQLite3] code:%d, %s", code, lastError().c_str());
                return false;
            }

            // sqlite3_exec(m_db, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);
            // sqlite3_exec(m_db, "PRAGMA synchronous=NORMAL;", nullptr, nullptr, nullptr);

            sqlite3_exec(m_db, "PRAGMA journal_mode=DELETE;", nullptr, nullptr, nullptr);
            sqlite3_exec(m_db, "PRAGMA synchronous=FULL;", nullptr, nullptr, nullptr);

            return true;
        });
}

bool Database::close()
{
    return DBExecutor::instance().postAndWait(
        [&]()
        {
            if (!m_db)
                return false;

            sqlite3_close_v2(m_db);
            m_db = nullptr;
            return true;
        });
}


std::unique_ptr<Database::Query> Database::query(const std::string &sql)
{
    return DBExecutor::instance().postAndWait(
        [&]() -> std::unique_ptr<Query>
        {
            if (!m_db)
                return nullptr;

            sqlite3_stmt* stmt = nullptr;
            if (sqlite3_prepare_v2(m_db, sql.c_str(), static_cast<int>(sql.size()) + 1, &stmt, nullptr) != SQLITE_OK)
            {
                e_log("[SQLite3] %s", sqlite3_errmsg(m_db));
                return nullptr;
            }

            i_log("[SQLite3] %s", sql.c_str());
            return std::make_unique<Query>(*this, stmt);
        });
}

std::string Database::sanitizeIdentifier(const std::string &input)
{
    std::string output;
    for (char c : input)
    {
        switch (c)
        {
        case '"':
            output += "\"\"";
            break; // 双引号转义
        case '-':
            output += "_";
            break; // 处理可能的关键字冲突
        default:
            output += c;
        }
    }
    return "\"" + output + "\""; // 添加标识符引用
}

std::string Database::lastError() const
{
    return DBExecutor::instance().postAndWait(
        [&]() -> std::string
        {
            std::string error;
            if (m_db)
            {
                error = sqlite3_errmsg(m_db);
            }
            return error;
        });
}

// Database::Query

Database::Query::Query(Database &database, sqlite3_stmt *stmt) : m_database(database), m_stmt(stmt), m_has_rows(false) {}

Database::Query::~Query()
{
    auto stmt = m_stmt;
    m_stmt    = nullptr;

    if (DBExecutor::instance().isRunning())
    {
        DBExecutor::instance().postAndWait(
            [stmt]()
            {
                if (stmt)
                    sqlite3_finalize(stmt);
            });
    }
    else
    {
        if (stmt)
            sqlite3_finalize(stmt);
    }
}

void Database::Query::reset()
{
    DBExecutor::instance().postAndWait(
        [&]()
        {
            sqlite3_reset(m_stmt);
            sqlite3_clear_bindings(m_stmt);
            m_has_rows = false;
        });
}

bool Database::Query::bind(int number)
{
    return DBExecutor::instance().postAndWait([&]() { return sqlite3_bind_null(m_stmt, number) == SQLITE_OK; });
}
bool Database::Query::bind(int number, const std::vector<uint8_t>& value)
{
    return DBExecutor::instance().postAndWait(
        [&]()
        {
            if (value.empty())
            {
                return sqlite3_bind_blob(m_stmt, number, nullptr, 0, SQLITE_TRANSIENT) == SQLITE_OK;
            }
            return sqlite3_bind_blob(m_stmt, number, &value[0], static_cast<int>(value.size()), SQLITE_TRANSIENT) == SQLITE_OK;
        });
}

bool Database::Query::bind(int number, bool value)
{
    return DBExecutor::instance().postAndWait([&]() { return sqlite3_bind_int(m_stmt, number, value ? 1 : 0) == SQLITE_OK; });
}

bool Database::Query::bind(int number, double value)
{
    return DBExecutor::instance().postAndWait([&]() { return sqlite3_bind_double(m_stmt, number, value) == SQLITE_OK; });
}

bool Database::Query::bind(int number, int32_t value)
{
    return DBExecutor::instance().postAndWait([&]() { return sqlite3_bind_int(m_stmt, number, value) == SQLITE_OK; });
}

bool Database::Query::bind(int number, uint32_t value)
{
    return DBExecutor::instance().postAndWait([&]()
                                              { return sqlite3_bind_int64(m_stmt, number, static_cast<int64_t>(value)) == SQLITE_OK; });
}

bool Database::Query::bind(int number, int64_t value)
{
    return DBExecutor::instance().postAndWait([&]() { return sqlite3_bind_int64(m_stmt, number, value) == SQLITE_OK; });
}

bool Database::Query::bind(int number, uint64_t value)
{
    return DBExecutor::instance().postAndWait([&]()
                                              { return sqlite3_bind_int64(m_stmt, number, static_cast<int64_t>(value)) == SQLITE_OK; });
}

bool Database::Query::bind(int number, const std::string& value)
{
    return DBExecutor::instance().postAndWait(
        [&]() { return sqlite3_bind_text(m_stmt, number, value.c_str(), -1, SQLITE_TRANSIENT) == SQLITE_OK; });
}

bool Database::Query::bind(int number, const char* value)
{
    return DBExecutor::instance().postAndWait([&]()
                                              { return sqlite3_bind_text(m_stmt, number, value, -1, SQLITE_TRANSIENT) == SQLITE_OK; });
}

bool Database::Query::exec()
{
    return DBExecutor::instance().postAndWait(
        [&]()
        {
            m_has_rows = false;
            int rc     = sqlite3_step(m_stmt);
            if (rc == SQLITE_ROW)
                m_has_rows = true;
            return rc == SQLITE_ROW || rc == SQLITE_DONE;
        });
}

bool Database::Query::hasRows() const
{
    return m_has_rows;
}

bool Database::Query::next()
{
    return DBExecutor::instance().postAndWait(
        [&]() -> bool
        {
            int rc = sqlite3_step(m_stmt);
            if (rc == SQLITE_ROW)
            {
                m_has_rows = true;
                return true;
            }
            // if no more rows, keep m_has_rows false (exec() will be used for initial)
            return false;
        });
}

std::string Database::Query::lastError() const
{
    return m_database.lastError();
}

bool Database::Query::isNull(int column) const
{
    return DBExecutor::instance().postAndWait([&]() { return sqlite3_column_type(m_stmt, column) == SQLITE_NULL; });
}

std::vector<uint8_t> Database::Query::getBlob(int column) const
{
    return DBExecutor::instance().postAndWait(
        [&]()
        {
            std::vector<uint8_t> value;
            const void*          blob = sqlite3_column_blob(m_stmt, column);
            if (!blob)
                return value;
            int size = sqlite3_column_bytes(m_stmt, column);
            value.resize(size);
            if (size > 0)
                memcpy(&value[0], blob, static_cast<size_t>(size));
            return value;
        });
}

bool Database::Query::getBool(int column) const
{
    return DBExecutor::instance().postAndWait([&]() { return sqlite3_column_int(m_stmt, column) != 0; });
}

double Database::Query::getFloat(int column) const
{
    return DBExecutor::instance().postAndWait([&]() { return sqlite3_column_double(m_stmt, column); });
}

int32_t Database::Query::getInt32(int column) const
{
    return DBExecutor::instance().postAndWait([&]() { return sqlite3_column_int(m_stmt, column); });
}

uint32_t Database::Query::getUInt32(int column) const
{
    return DBExecutor::instance().postAndWait([&]() { return static_cast<uint32_t>(sqlite3_column_int64(m_stmt, column)); });
}

int64_t Database::Query::getInt64(int column) const
{
    return DBExecutor::instance().postAndWait([&]() { return sqlite3_column_int64(m_stmt, column); });
}

uint64_t Database::Query::getUInt64(int column) const
{
    return DBExecutor::instance().postAndWait([&]() { return static_cast<uint64_t>(sqlite3_column_int64(m_stmt, column)); });
}

std::string Database::Query::getString(int column) const
{
    return DBExecutor::instance().postAndWait(
        [&]()
        {
            const unsigned char* text = sqlite3_column_text(m_stmt, column);
            return text ? std::string(reinterpret_cast<const char*>(text)) : std::string();
        });
}
