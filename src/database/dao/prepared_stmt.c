#include "prepared_stmt.h"

#include "utils/utils.h"
#include "utils/cache/lru.h"
#include "database/dao/template.h"

#include <stdint.h>

#define MAX_MEM     (20 * 1000)
#define MAX_KEY_NUM (100)

bool new_prepared_stmt(prepared_stmt_t *ps, sqlite3 *db, db_dsn_e dsn)
{
    if ((NULL == db)
    || (NULL == ps))
    {
        d_log("api misuse");
        return false;
    }

    /* 分配内存用于 LRU 缓存 */
    ps->cache = (lru_cache_t*)malloc(sizeof(lru_cache_t));
    if (NULL == ps->cache)
    {
        d_log("malloc error dsn=%d", dsn);
        return false;
    }

    /* 初始化 LRU 缓存 */
    if (false == new_lru_cache(ps->cache, MAX_KEY_NUM, MAX_MEM))
    {
        free(ps->cache);
        return false;
    }

    ps->db = db;
    ps->dsn = dsn;
    (void)pthread_mutex_init(&ps->mutex, NULL);
    return true;
}

void delete_prepared_stmt(prepared_stmt_t *ps)
{
    if (NULL == ps)
    {
        d_log("api misuse");
        return;
    }

    ps->db = NULL;
    delete_lru_cache(ps->cache);
    if (NULL != ps->cache)
    {
        free(ps->cache);
    }
    ps->cache = NULL;
    pthread_mutex_destroy(&ps->mutex);
    return;
}

/**
 * @brief 创建新的 sqlite3_stmt 结构
 * @param db 指向 sqlite3 数据库连接的指针
 * @param sql SQL 语句字符串
 * @return 成功返回 sqlite3_stmt 结构指针 失败返回 NULL
 */
static sqlite3_stmt* new_pstmt(sqlite3 *db, const char *sql)
{
    sqlite3_stmt *pstmt = NULL;
    int32_t rc = sqlite3_prepare_v2(db, sql, -1, &pstmt, NULL);
    if ((SQLITE_OK != rc)
    || (NULL == pstmt))
    {
        d_log("sql_err=%s sql=%s", sqlite3_errmsg(db), sql);
        return NULL;
    }

    // d_log("func=%s sql=%s", __func__, sql);
    return pstmt;
}

/**
 * @brief 拷贝函数 用于缓存
 * @param dst 目标位置 用于存放拷贝后的数据
 * @param src 源位置 需要被拷贝的数据
 */
static void copy(void *dst, const void *src)
{
    int64_t *d = (int64_t*)dst;
    *d = (int64_t)src;
    return;
}

/**
 * @brief 析构函数 用于缓存达到限制后释放 sqlite3_stmt 结构
 * @param dst 需要被析构的数据指针
 */
static void dtor(void *dst)
{
    sqlite3_stmt *pstmt = (sqlite3_stmt*)(*(int64_t*)dst);
    sqlite3 *db = sqlite3_db_handle(pstmt);
    d_log("func=%s sql=%s", __func__, sqlite3_sql(pstmt));
    DAO_RETURN_IS_OK(sqlite3_finalize(pstmt));
    free(dst);
    return;
}

static UT_icd g_icd = {
    .sz = sizeof(sqlite3_stmt*),
    .init = NULL,
    .copy = copy,
    .dtor = dtor,
};

sqlite3_stmt* prepared_stmt_get(prepared_stmt_t *ps, const char *key)
{
    if ((NULL == ps)
    || (NULL == key))
    {
        d_log("api misuse");
        return NULL;
    }

    (void)pthread_mutex_lock(&ps->mutex);
    /* 从缓存中获取缓存项 */
    cache_item_t *item = lru_cache_get(ps->cache, key);
    if (NULL == item)
    {
        sqlite3 *db = ps->db;
        /* 创建新的 sqlite3_stmt 结构 */
        sqlite3_stmt *pstmt = new_pstmt(db, key);
        if (NULL == pstmt)
        {
            d_log("key=%s", key);
            (void)pthread_mutex_unlock(&ps->mutex);
            return NULL;
        }

        /* 将 sqlite3_stmt 结构添加到缓存 */
        bool ret = lru_cache_put(ps->cache, key, &g_icd, pstmt);
        if (false == ret)
        {
            d_log("key=%s", key);
            DAO_RETURN_IS_OK(sqlite3_finalize(pstmt));
            (void)pthread_mutex_unlock(&ps->mutex);
            return NULL;
        }
        return pstmt;
    }
    /* 返回缓存中的 sqlite3_stmt 结构 */
    return (sqlite3_stmt*)(*(int64_t*)item->data);
}

void prepared_stmt_put(prepared_stmt_t *ps, sqlite3_stmt *pstmt)
{
    if ((NULL == ps)
    || (NULL == pstmt))
    {
        d_log("api misuse");
        return;
    }

    sqlite3 *db = ps->db;
    /* 重置 sqlite3_stmt 结构 */
    DAO_RETURN_IS_OK(sqlite3_reset(pstmt));
    /* 检查 sqlite3_stmt 结构是否繁忙 */
    if (true == sqlite3_stmt_busy(pstmt))
    {
        const char *sql = sqlite3_sql(pstmt);
        d_log("sql_err=busy %s", sql);
        lru_cache_remove(ps->cache, sql);
    }
    (void)pthread_mutex_unlock(&ps->mutex);
    return;
}