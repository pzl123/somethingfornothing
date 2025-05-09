#include "sqlite_master_dao.h"
#include "database/dao/template.h"
enum {
    COUNT_BY_NAME = 0,
};

static const char *g_sql[] = {
    [COUNT_BY_NAME] = "select count(*) from sqlite_master where type='table' and name=?",
};

bool new_sqlite_master_dao(sqlite_master_dao_t *dao, prepared_stmt_t *ps)
{
    if ((NULL == dao)
    || (NULL == ps))
    {
        d_log("api misuse");
        return false;
    }
    dao->ps = ps;
    return true;
}

void delete_sqlite_master_dao(sqlite_master_dao_t *dao)
{
    if (NULL == dao)
    {
        d_log("api misuse");
        return;
    }
    dao->ps = NULL;
    return;
}

static void count_by_name_bind_cb(sqlite3 *db, sqlite3_stmt *pstmt, void *userp)
{
    const char *name = (const char *)userp;
    DAO_RETURN_IS_OK(sqlite3_bind_text(pstmt, 1, name, -1, NULL));
    return;
}

static void count_by_name_unmarshal_cb(sqlite3_stmt *pstmt, void *userp)
{
    int32_t *count = (int32_t*)userp;
    *count = sqlite3_column_int(pstmt, 0);
    return;
}

int32_t sqlite_master_dao_count_by_name(sqlite_master_dao_t *dao, const char *name, int32_t *count)
{
    if ((NULL == dao)
    || (NULL == name)
    || (NULL == count))
    {
        d_log("api misuse");
        return -1;
    }
    return dao_template_get(dao->ps, g_sql[COUNT_BY_NAME], count_by_name_bind_cb, (void*)name, count_by_name_unmarshal_cb, (void*)count);
}