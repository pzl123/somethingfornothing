#include "template.h"
#include "uthash/utstring.h"
#include "database/dao/dao_option.h"
#include "hv/hplatform.h"

int32_t dao_template_create_pk_backfill(prepared_stmt_t *ps, const char *sql, stmt_bind_cb cb, void *userp, int32_t *pk)
{
    if ((NULL == ps)
    || (NULL == sql)
    || (NULL == pk))
    {
        d_log("api misuse");
        return -1;
    }

    sqlite3_stmt *pstmt = prepared_stmt_get(ps, sql);
    if (NULL == pstmt)
    {
        d_log("sql=%s", sql);
        return -1;
    }

    sqlite3 *db = sqlite3_db_handle(pstmt);
    if (NULL != cb)
    {
        cb(db, pstmt, userp);
    }

    int32_t rc = sqlite3_step(pstmt);
    int32_t change = sqlite3_changes(db);
    if (SQLITE_DONE != rc)
    {
        d_log("sql_err=%s sql=%s", sqlite3_errmsg(db), sql);
        change = -1;
    }
    else
    {
        *pk = (int32_t)sqlite3_last_insert_rowid(db);
    }

    prepared_stmt_put(ps, pstmt);
    return change;
}

static int32_t dao_template_base(prepared_stmt_t *ps, const char *sql, stmt_bind_cb cb, void *userp)
{
    if ((NULL == ps)
    || (NULL == sql))
    {
        d_log("api misuse");
        return -1;
    }

    sqlite3_stmt *pstmt = prepared_stmt_get(ps, sql);
    if (NULL == pstmt)
    {
        d_log("sql=%s", sql);
        return -1;
    }

    sqlite3 *db = sqlite3_db_handle(pstmt);
    if (NULL != cb)
    {
        cb(db, pstmt, userp);
    }

    int32_t rc = sqlite3_step(pstmt);
    int32_t change = sqlite3_changes(db);
    if (SQLITE_DONE != rc)
    {
        d_log("sql_err=%s sql=%s", sqlite3_errmsg(db), sql);
        change = -1;
    }

    prepared_stmt_put(ps, pstmt);
    return change;
}

int32_t dao_template_select(prepared_stmt_t *ps, const char *sql, stmt_bind_cb cb, void *userp)
{
    if ((NULL == ps)
    || (NULL == sql)
    || (NULL == userp))
    {
        d_log("api misuse");
        return -1;
    }
    sqlite3_stmt *pstmt = prepared_stmt_get(ps, sql);
    if (NULL == pstmt)
    {
        d_log("sql=%s", sql);
        return -1;
    }

    sqlite3 *db = sqlite3_db_handle(pstmt);
    if (NULL != cb)
    {
        cb(db, pstmt, userp);
    }

    int32_t line = 0;
    while (SQLITE_ROW == sqlite3_step(pstmt))
    {
        line++;
    }
    prepared_stmt_put(ps, pstmt);
    return line;
}

int32_t dao_template_create(prepared_stmt_t *ps, const char *sql, stmt_bind_cb cb, void *userp)
{
    return dao_template_base(ps, sql, cb, userp);
}

int32_t dao_template_delete(prepared_stmt_t *ps, const char *sql, stmt_bind_cb cb, void *userp)
{
    return dao_template_base(ps, sql, cb, userp);
}

int32_t dao_template_update(prepared_stmt_t *ps, const char *sql, stmt_bind_cb cb, void *userp)
{
    return dao_template_base(ps, sql, cb, userp);
}

bool dao_template_drop(prepared_stmt_t *ps, const char *sql)
{
    int32_t ret = dao_template_base(ps, sql, NULL, NULL);
    if (-1 == ret)
    {
        return false;
    }
    return true;
}

int32_t dao_template_get(prepared_stmt_t *ps, const char *sql, stmt_bind_cb bind_cb, void *bp, stmt_unmarshal_cb unmarshal_cb, void *up)
{
    if ((NULL == ps)
    || (NULL == sql)
    || (NULL == unmarshal_cb)
    || (NULL == up))
    {
        d_log("api misuse");
        return -1;
    }

    sqlite3_stmt *pstmt = prepared_stmt_get(ps, sql);
    if (NULL == pstmt)
    {
        d_log("sql=%s", sql);
        return -1;
    }

    sqlite3 *db = sqlite3_db_handle(pstmt);
    if (NULL != bind_cb)
    {
        bind_cb(db, pstmt, bp);
    }

    int32_t line_num = 0;
    while (SQLITE_ROW == sqlite3_step(pstmt))
    {
        line_num++;
        unmarshal_cb(pstmt, up);
    }

    prepared_stmt_put(ps, pstmt);
    return line_num;
}

static int32_t dao_template_batch_base(prepared_stmt_t *ps, const char *sql, int32_t batch_size, stmt_batch_bind_cb cb, void *userp)
{
    if ((NULL == ps)
    || (NULL == sql))
    {
        d_log("api misuse");
        return -1;
    }

    sqlite3_stmt *pstmt = prepared_stmt_get(ps, sql);
    if (NULL == pstmt)
    {
        d_log("ps=%p sql=%s batch_size=%d cb=%p userp=%p", ps, sql, batch_size, cb, userp);
        return -1;
    }

    sqlite3 *db = sqlite3_db_handle(pstmt);
    dao_begin(db, __func__);

    int32_t line_num = 0;
    for (int32_t i = 0; i < batch_size; i++)
    {
        if (NULL != cb)
        {
            cb(db, pstmt, i, userp);
        }

        int32_t rc = sqlite3_step(pstmt);
        if (SQLITE_DONE != rc)
        {
            d_log("sql_err=%s sql=%s batch_size=%d cb=%p userp=%p", sqlite3_errmsg(db), sql, batch_size, cb, userp);
        }
        else
        {
            line_num++;
        }

        DAO_RETURN_IS_OK(sqlite3_reset(pstmt));
        if (true == sqlite3_stmt_busy(pstmt))
        {
            d_log("sql_err=busy sql=%s batch_size=%d cb=%p userp=%p i=%d", sql, batch_size, cb, userp, i);
        }
    }

    dao_commit(db, __func__);
    prepared_stmt_put(ps, pstmt);
    return line_num;
}

int32_t dao_template_batch_create(prepared_stmt_t *ps, const char *sql, int32_t batch_size, stmt_batch_bind_cb cb, void *userp)
{
    return dao_template_batch_base(ps, sql, batch_size, cb, userp);
}

static int32_t dao_template_base_with_option(prepared_stmt_t *ps, const char *sql_part, const UT_array *options, stmt_unmarshal_cb unmarshal_cb, void *up)
{
    if ((NULL == ps)
    || (NULL == sql_part)
    || (NULL == options))
    {
        d_log("api misuse");
        return -1;
    }

    UT_string *sql = NULL;
    utstring_new(sql);
    utstring_printf(sql, sql_part);
    const dao_option_t *p = (const dao_option_t*)utarray_front(options);
    for (; p != NULL; p = (const dao_option_t*)utarray_next(options, p))
    {
        utstring_printf(sql, p->sql_part);
    }

    sqlite3_stmt *pstmt = prepared_stmt_get(ps, utstring_body(sql));
    if (NULL == pstmt)
    {
        d_log("sql=%s", utstring_body(sql));
        return -1;
    }

    utstring_free(sql);
    sqlite3 *db = sqlite3_db_handle(pstmt);

    int32_t i = 1;
    p = (const dao_option_t*)utarray_front(options);
    for (; p != NULL; p = (const dao_option_t*)utarray_next(options, p))
    {
        switch (p->type)
        {
        case DAO_TYPE_INT32:
            DAO_RETURN_IS_OK(sqlite3_bind_int(pstmt, i++, (int32_t)(int64_t)p->userp));
            break;
        case DAO_TYPE_INT64:
            DAO_RETURN_IS_OK(sqlite3_bind_int64(pstmt, i++, (int64_t)p->userp));
            break;
        case DAO_TYPE_FLOAT32:
            DAO_RETURN_IS_OK(sqlite3_bind_double(pstmt, i++, (float32_t)(int64_t)p->userp));
            break;
        case DAO_TYPE_FLOAT64:
            DAO_RETURN_IS_OK(sqlite3_bind_double(pstmt, i++, (float64_t)(int64_t)p->userp));
            break;
        case DAO_TYPE_TEXT:
            DAO_RETURN_IS_OK(sqlite3_bind_text(pstmt, i++, (const char*)p->userp, -1, NULL));
            break;
        case DAO_TYPE_NULL:
            break;
        default:
            d_log("dao_option_t error type=%d sql_part=%s", p->type, p->sql_part);
            break;
        }
    }

    int32_t line_num = 0;
    while (SQLITE_ROW == sqlite3_step(pstmt))
    {
        line_num++;
        if (NULL != unmarshal_cb)
        {
            unmarshal_cb(pstmt, up);
        }
    }

    if (0 == line_num)
    {
        line_num = sqlite3_changes(db);
    }

    prepared_stmt_put(ps, pstmt);
    return line_num;
}

int32_t dao_template_get_with_option(prepared_stmt_t *ps, const char *sql, const UT_array *options, stmt_unmarshal_cb unmarshal_cb, void *up)
{
    if ((NULL == ps)
    || (NULL == sql)
    || (NULL == options)
    || (NULL == unmarshal_cb))
    {
        d_log("api misuse");
        return -1;
    }

    return dao_template_base_with_option(ps, sql, options, unmarshal_cb, up);
}

int32_t dao_template_delete_with_option(prepared_stmt_t *ps, const char *sql, const UT_array *options)
{
    if ((NULL == ps)
    || (NULL == sql)
    || (NULL == options))
    {
        d_log("api misuse");
        return -1;
    }

    return dao_template_base_with_option(ps, sql, options, NULL, NULL);
}

int32_t dao_template_update_with_option(prepared_stmt_t *ps, const char *sql, const UT_array *options)
{
    if ((NULL == ps)
    || (NULL == sql)
    || (NULL == options))
    {
        d_log("api misuse");
        return -1;
    }

    return dao_template_base_with_option(ps, sql, options, NULL, NULL);
}

void dao_begin(sqlite3 *db, const char *func_name)
{
    if (NULL == db)
    {
        d_log("func_name=%s api misuse", func_name);
        return;
    }
    if (SQLITE_OK != sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, NULL))
    {
        const char *errmsg = sqlite3_errmsg(db);
        d_log("func_name=%s sql_err=%s", func_name, errmsg);

        const char *msg = "cannot start a transaction within a transaction";
        if (0 == strcmp(msg, errmsg))
        {
            dao_commit(db, func_name);
        }
    }
    return;
}

void dao_rollback(sqlite3 *db, const char *func_name)
{
    if (NULL == db)
    {
        d_log("func_name=%s api misuse", func_name);
        return;
    }
    if (SQLITE_OK != sqlite3_exec(db, "ROLLBACK", NULL, NULL, NULL))
    {
        d_log("func_name=%s sql_err=%s", func_name, sqlite3_errmsg(db));
    }
    return;
}

void dao_commit(sqlite3 *db, const char *func_name)
{
    if (NULL == db)
    {
        d_log("func_name=%s api misuse", func_name);
        return;
    }
    if (SQLITE_OK != sqlite3_exec(db, "COMMIT", NULL, NULL, NULL))
    {
        d_log("func_name=%s sql_err=%s", func_name, sqlite3_errmsg(db));
    }
    return;
}

void dao_close(sqlite3 *db)
{
    if (NULL == db)
    {
        d_log("api misuse");
        return;
    }

    int32_t code = sqlite3_close(db);
    while (SQLITE_BUSY == code)
    {
        code = SQLITE_OK;
        sqlite3_stmt *pstmt = sqlite3_next_stmt(db, NULL);
        if (NULL == pstmt)
        {
            break;
        }

        code = sqlite3_finalize(pstmt);
        if (SQLITE_OK == code)
        {
            code = sqlite3_close(db);
        }
        else
        {
            d_log("sql_err=%s", sqlite3_errmsg(db));
        }
    }
    return;
}
