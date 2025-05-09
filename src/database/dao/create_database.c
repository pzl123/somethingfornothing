#include "create_database.h"
#include "utils/utils.h"
#include "database/dao/template.h"


bool create_table(sqlite_master_dao_t *dao, const table_t *table)
{
    int32_t count = 0;
    int32_t line_num = sqlite_master_dao_count_by_name(dao, table->table_name, &count);
    if (1 != line_num)
    {
        d_log("label=create_db line_num=%d table_name=%s count=%d", line_num, table->table_name, count);
        return false;
    }

    if (count > 0)
    {
        d_log("label=create_db msg=%s exist", table->table_name);
        return true;
    }

    sqlite3 *db = dao->ps->db;
    dao_begin(db, __func__);
    if (SQLITE_OK != sqlite3_exec(db, table->create_table_stmt, NULL, NULL, NULL))
    {
        d_log("label=create_db sql_err=%s", sqlite3_errmsg(db));
        dao_rollback(db, __func__);
        return false;
    }
    dao_commit(db, __func__);
    return true;
}

bool create_database(sqlite_master_dao_t *dao, const table_t *tables, int32_t len)
{
    if ((NULL == tables)
    || (NULL == dao))
    {
        d_log("api misuse");
        return false;
    }

    for (int32_t i = 0; i < len; i++)
    {
        if (tables[i].dsn == dao->ps->dsn)
        {
            if (false == create_table(dao, &tables[i]))
            {
                d_log("label=create_db msg=create_error table_name=%s dsn=%d ", tables[i].table_name, tables[i].dsn);
            }
        }
    }
    return true;
}