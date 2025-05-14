#include "retention_policy.h"
#include "utils/utils.h"
#include "uthash/utstring.h"
#include "database/dao/template.h"
#include "utils/utils.h"

#include <stdlib.h>
#include <string.h>


typedef struct
{
    const char *table_name;
    const char *where_field;
    int64_t expire_s;
    prepared_stmt_t *ps;
}rp_by_timestamp_t;

typedef struct
{
    const char *table_name;
    int32_t max_num;
    int32_t percent;
    prepared_stmt_t *ps;
} rp_by_line_t;


enum {
    BY_TIMESTAMP_DELETE,
    BY_LINE_GET_NUM,
    BY_LINE_DELETE,
};

static const char *g_sql[] = {
    [BY_TIMESTAMP_DELETE]   = "delete from %s where %s < ?",
    [BY_LINE_GET_NUM]       = "select count(*) from %s",
    [BY_LINE_DELETE]        = "delete from %s where id < (select id from %s limit 1 offset ?)",
};

static void get_line_num_unmarshal_cb(sqlite3_stmt *pstmt, void *userp)
{
    int32_t *line_num = (int32_t*)userp;
    *line_num = sqlite3_column_int(pstmt, 0);
    return;
}

static int32_t get_line_num(prepared_stmt_t *ps, const char *table, int32_t *line_num)
{
    UT_string *s = NULL;
    utstring_new(s);
    utstring_printf(s, g_sql[BY_LINE_GET_NUM], table);
    int32_t ret = dao_template_get(ps, utstring_body(s), NULL, NULL, get_line_num_unmarshal_cb, line_num);
    if (ret <= 0)
    {
        d_log("sql_err table_name=%s ret=%d", table, ret);
    }
    utstring_free(s);
    return ret;
}

static void by_line_bind_cb(sqlite3 *db, sqlite3_stmt *pstmt, void *userp)
{
    int32_t try_delete_num = (int32_t)(int64_t)userp;
    DAO_RETURN_IS_OK(sqlite3_bind_int(pstmt, 1, try_delete_num));
    return;
}

static bool by_line(retention_policy_t *r)
{
    if (!r)
    {
        d_log("api misuse");
        return false;
    }

    rp_by_line_t *rp_by_line = (rp_by_line_t *)r->policy;
    prepared_stmt_t *ps = rp_by_line->ps;
    const char *table_name = rp_by_line->table_name;

    int32_t cur_line_num = 0;
    int32_t ret = get_line_num(ps, table_name, &cur_line_num);
    if (ret <= 0)
    {
        d_log("line_num error table_name=%s ret=%d", table_name, ret);
        return false;
    }

    if (cur_line_num <= rp_by_line->max_num)
    {
        return true;
    }

    UT_string *s = NULL;
    utstring_new(s);
    utstring_printf(s, g_sql[BY_LINE_DELETE], table_name, table_name);

    int32_t try_delete_num = cur_line_num - (rp_by_line->max_num * rp_by_line->percent / 100);
    int32_t real_delete_num = dao_template_delete(ps, utstring_body(s), by_line_bind_cb, (void*)(int64_t)try_delete_num);
    utstring_free(s);
    if (-1 == real_delete_num)
    {
        d_log("sql_err table_name=%s cur_line_num=%d max_num=%d", table_name, cur_line_num, rp_by_line->max_num);
        return false;
    }
    else if (real_delete_num != try_delete_num)
    {
        d_log("table_name=%s try_delete=%d real_delete=%d cur_line_num=%d max_num=%d",
                    table_name,
                    try_delete_num,
                    real_delete_num,
                    cur_line_num,
                    rp_by_line->max_num);
        return false;
    }
    else
    {
        d_log("table_name=%s cur_line_num=%d real_delete_num=%d", table_name, cur_line_num, real_delete_num);
        return true;
    }
    return true;
}

bool new_retention_policy_by_line(retention_policy_t *r, prepared_stmt_t *ps, const char *table, int32_t max_num, int32_t percent)
{
    if (!r || !ps || !table || ((percent < 0) || (percent > 100)))
    {
        d_log("api misuse");
        return false;
    }

    rp_by_line_t *rp_by_line = (rp_by_line_t *)malloc(sizeof(rp_by_line_t));
    if (!rp_by_line)
    {
        d_log("malloc failed");
        return false;
    }

    rp_by_line->table_name = strdup(table);
    if (!rp_by_line->table_name)
    {
        d_log("malloc failed");
        return false;
    }

    rp_by_line->max_num = max_num;
    rp_by_line->percent = percent;
    rp_by_line->ps = ps;
    r->policy = rp_by_line;
    r->policy_func = by_line;
    return true;
}

void delete_retention_policy_by_line(retention_policy_t *r)
{
    if (NULL == r)
    {
        d_log("api misuse");
        return;
    }

    if (NULL != r->policy)
    {
        rp_by_line_t *policy = (rp_by_line_t*)r->policy;
        if (NULL != policy->table_name)
        {
            free((void*)policy->table_name);
            policy->table_name = NULL;
        }

        free(r->policy);
        r->policy = NULL;
    }
    r->policy_func = NULL;
    return;
}

static void by_timestamp_bind_cb(sqlite3 *db, sqlite3_stmt *pstmt, void *userp)
{
    int64_t when = (int64_t)userp;
    DAO_RETURN_IS_OK(sqlite3_bind_int64(pstmt, 1, when));
    return;
}

static bool by_timestamp(retention_policy_t *r)
{
    if (NULL == r)
    {
        d_log("api misuse");
        return false;
    }

    rp_by_timestamp_t *policy = (rp_by_timestamp_t*)r->policy;
    prepared_stmt_t *ps = policy->ps;
    const char *table_name = policy->table_name;

    UT_string *s = NULL;
    utstring_new(s);
    utstring_printf(s, g_sql[BY_TIMESTAMP_DELETE], table_name, policy->where_field);

    int64_t when = (time(0) - policy->expire_s);
    int32_t real_delete_num = dao_template_delete(ps, utstring_body(s), by_timestamp_bind_cb, (void*)when);
    utstring_free(s);
    if (-1 == real_delete_num)
    {
        d_log("sql_err table_name=%s when=%ld", table_name, when);
        return false;
    }
    else if (0 == real_delete_num)
    {
        return true;
    }
    else
    {
        d_log("table_name=%s real_delete_num=%d", table_name, real_delete_num);
        return true;
    }
    return true;
}

bool new_retention_policy_by_timestamp(retention_policy_t *r, prepared_stmt_t *ps, const char *table_name, const char *where_field, int64_t expire_s)
{
    if ((!r)
    || (!ps)
    || (!table_name)
    || (!where_field))
    {
        d_log("api misuse");
        return false;
    }
    rp_by_timestamp_t *policy = (rp_by_timestamp_t*)malloc(sizeof(rp_by_timestamp_t));
    if (!policy)
    {
        d_log("malloc error");
        return false;
    }

    policy->table_name = strdup(table_name);
    if (!policy->table_name)
    {
        d_log("strdup error");
        free(policy);
        return false;
    }

    policy->where_field = strdup(where_field);
    if (NULL == policy->table_name)
    {
        d_log("malloc error");
        free((void*)policy->table_name);
        free(policy);
        return false;
    }

    policy->expire_s = expire_s;
    policy->ps = ps;
    r->policy = policy;
    r->policy_func = by_timestamp;
    return true;
}

void delete_retention_policy_by_timestamp(retention_policy_t *r)
{
    if (NULL == r)
    {
        d_log("api misuse");
        return;
    }

    if (NULL != r->policy)
    {
        rp_by_timestamp_t *option = (rp_by_timestamp_t*)r->policy;
        if (NULL != option->table_name)
        {
            free((void*)option->table_name);
            option->table_name = NULL;
        }

        if (NULL != option->where_field)
        {
            free((void*)option->where_field);
            option->where_field = NULL;
        }

        free(r->policy);
        r->policy = NULL;
    }
    r->policy_func = NULL;
    return;
}

static bool always(retention_policy_t *r)
{
    (void)r;
    return true;
}

bool new_retention_policy_always(retention_policy_t *r)
{
    if (NULL == r)
    {
        d_log("api misuse");
        return false;
    }

    r->policy = NULL;
    r->policy_func = always;
    return true;
}

void delete_retention_policy_always(retention_policy_t *r)
{
    if (NULL == r)
    {
        d_log("api misuse");
        return;
    }
    r->policy_func = NULL;
    return;
}
