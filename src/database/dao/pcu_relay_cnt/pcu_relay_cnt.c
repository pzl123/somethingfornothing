#include "pcu_relay_cnt.h"
#include "database/dao/template.h"
#include "database/model/pcu_relay_cnt_model.h"

enum {
    CREATE = 0,
    GET_BY_RELAY_ID,
    UPDATE_BY_RELAY_ID,
};

static const char *sql[] = {
    [CREATE]                = "insert into pcu_relay_cnt(relay_id,close_cnt) values(?,?)",
    [GET_BY_RELAY_ID]       = "select id,relay_id,close_cnt from pcu_relay_cnt where relay_id=?",
    [UPDATE_BY_RELAY_ID]    = "update pcu_relay_cnt set close_cnt=? where relay_id=?",
};

bool new_pcu_relay_cnt_dao(pcu_relay_cnt_dao_t *dao, prepared_stmt_t *ps)
{
    if ((NULL == dao)
    || (NULL == ps))
    {
        d_log("api misuse");
        return false;
    }

    dao->ps = ps;
    // (void)new_retention_policy_always(&dao->rp);
    return true;
}

void delete_pcu_relay_cnt_dao(pcu_relay_cnt_dao_t *dao)
{
    if (NULL == dao)
    {
        d_log("api misuse");
        return;
    }

    dao->ps = NULL;
    // delete_retention_policy_always(&dao->rp);
    return;
}

static void create_bind_cb(sqlite3 *db, sqlite3_stmt *pstmt, void *userp)
{
    const pcu_relay_cnt_t *p = (const pcu_relay_cnt_t*)userp;
    int32_t i = 1;
    DAO_RETURN_IS_OK(sqlite3_bind_int64(pstmt, i++, p->relay_id));
    DAO_RETURN_IS_OK(sqlite3_bind_int(pstmt, i++, p->close_cnt));
    return;
}

int32_t pcu_relay_cnt_dao_create(pcu_relay_cnt_dao_t *dao, const pcu_relay_cnt_t *p)
{
    if ((NULL == dao)
    || (NULL == p))
    {
        d_log("api misuse");
        return -1;
    }

    return dao_template_create(dao->ps, sql[CREATE], create_bind_cb, (void*)p);
}

static void get_bind_cb(sqlite3 *db, sqlite3_stmt *pstmt, void *userp)
{
    pcu_do_branch_relay_e relay_id = (pcu_do_branch_relay_e)(int64_t)userp;
    int32_t i = 1;
    DAO_RETURN_IS_OK(sqlite3_bind_int64(pstmt, i++, relay_id));
    return;
}

static void get_unmarshal_cb(sqlite3_stmt *pstmt, void *userp)
{
    pcu_relay_cnt_t *res = (pcu_relay_cnt_t*)userp;
    int32_t i = 0;
    res->id = sqlite3_column_int(pstmt, i++);
    res->relay_id = (pcu_do_branch_relay_e)sqlite3_column_int64(pstmt, i++);
    res->close_cnt = sqlite3_column_int(pstmt, i++);
    return;
}

int32_t pcu_relay_cnt_dao_get_by_relay_id(pcu_relay_cnt_dao_t *dao, pcu_do_branch_relay_e relay_id, pcu_relay_cnt_t *res)
{
    if ((NULL == dao)
    || (NULL == res))
    {
        d_log("api misuse");
        return -1;
    }

    return dao_template_get(dao->ps, sql[GET_BY_RELAY_ID], get_bind_cb, (void*)(int64_t)relay_id, get_unmarshal_cb, res);
}

typedef struct
{
    pcu_do_branch_relay_e relay_id;
    const pcu_relay_cnt_t *p;
}update_bind_cb_param;

static void update_bind_cb(sqlite3 *db, sqlite3_stmt *pstmt, void *userp)
{
    const update_bind_cb_param *param = (const update_bind_cb_param*)userp;
    int32_t i = 1;
    DAO_RETURN_IS_OK(sqlite3_bind_int(pstmt, i++, param->p->close_cnt));
    DAO_RETURN_IS_OK(sqlite3_bind_int64(pstmt, i++, param->relay_id));
    return;
}

int32_t pcu_relay_cnt_dao_update_by_relay_id(pcu_relay_cnt_dao_t *dao, pcu_do_branch_relay_e relay_id, const pcu_relay_cnt_t *p)
{
    if ((NULL == dao)
    || (NULL == p))
    {
        d_log("api misuse");
        return -1;
    }

    update_bind_cb_param param = {
        .relay_id = relay_id,
        .p = p,
    };
    return dao_template_update(dao->ps, sql[UPDATE_BY_RELAY_ID], update_bind_cb, &param);
}