#include "dal.h"
#include "database/dao/dao.h"
#include "database/dao/sql_trace.h"
#include "database/dao/template.h"
#include "database/model/table.h"
#include "database/dao/create_database.h"

#include <stdbool.h>
#include <stdlib.h>
#include <sys/prctl.h>

static void init_pstmts(prepared_stmt_t *pstmts)
{
    for (int32_t i = 0; i < DSN_END; i++)
    {
        sqlite3 *db = NULL;
        const char *path = dsn_to_string(i);
        if (SQLITE_OK != sqlite3_open(path, &db))
        {
            d_log("open %s error", path);
            return;
        }
        add_sql_trace(db, path);
        DAO_RETURN_IS_OK(sqlite3_busy_timeout(db, 10 * 1000));

        bool ret = new_prepared_stmt(&pstmts[i], db, i);
        if (false == ret)
        {
            d_log("new pstmt error path=%s i=%d db=%p", path, i, db);
        }
    }
    return;
}

static void init_sqlite_master_dao_array(sqlite_master_dao_t *dao_array, prepared_stmt_t *pstmts)
{
    for (int32_t i = 0; i < DSN_END; i++)
    {
        (void)new_sqlite_master_dao(&dao_array[i], &pstmts[i]);
    }
    return;
}

static void init_database(sqlite_master_dao_t *dao_array)
{
    const table_t *tables = NULL;
    int32_t len = 0;
    (void)get_tables(&tables, &len);

    for (int32_t i = 0; i < DSN_END; i++)
    {
        (void)create_database(&dao_array[i], tables, len);
    }
    return;
}

static void* dao_hloop_thread(void *arg)
{
    (void)prctl(PR_SET_NAME, __FUNCTION__);
    hloop_t *loop = (hloop_t *)arg;
    (void)hloop_run(loop);
    return NULL;
}

bool new_dal(dal_t *dal)
{
    if (!dal)
    {
        d_log("api misuse");
        return false;
    }

    (void)sql_global_error_init(); /* sqlite 库全局错误打印回调 */
    init_pstmts(dal->pstmts); /* 为每个数据库创建一个cahce, 缓存调用该库的stmt */
    init_sqlite_master_dao_array(dal->sqlite_master_dao_array, dal->pstmts);
    init_database(dal->sqlite_master_dao_array);

    dal->loop = hloop_new(HLOOP_FLAG_AUTO_FREE);
    (void)new_dao(&dal->dao, dal->pstmts, dal->loop, dal->sqlite_master_dao_array);
    (void)new_dao_deleter(&dal->deleter, dal->loop, new_retention_policy_array(&dal->dao), DAO_DELETER_TIMEOUT_10MIN);

    pthread_t tid;
    (void)pthread_create(&tid, NULL, dao_hloop_thread, dal->loop);
    (void)pthread_detach(tid);
    return true;
}

void delete_dal(dal_t *dal)
{
    if (NULL == dal)
    {
        d_log("api misuse");
        return;
    }

    (void)hloop_stop(dal->loop);
    delete_dao_deleter(&dal->deleter);
    delete_dao(&dal->dao);

    for (int32_t i = 0; i < DSN_END; i++)
    {
        sqlite3 *db = dal->pstmts[i].db;
        delete_sqlite_master_dao(&dal->sqlite_master_dao_array[i]);
        delete_prepared_stmt(&dal->pstmts[i]);
        dao_close(db);
    }
    return;
}