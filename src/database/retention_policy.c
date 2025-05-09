// #include "retention_policy.h"
// #include <time.h>
// #include <string.h>
// #include "uthash/utstring.h"

// #include "utils/utils.h"
// #include "database/dao/template.h"

// typedef struct
// {
//     const char *table_name;
//     const char *where_field;
//     int64_t expire_s;
//     prepared_stmt_t *ps;
// }rp_by_timestamp_t;

// typedef struct
// {
//     const char *table_name;
//     int32_t max_num;
//     int32_t percent;
//     prepared_stmt_t *ps;
// }rp_by_line_t;

// enum {
//     BY_TIMESTAMP_DELETE,
//     BY_LINE_GET_NUM,
//     BY_LINE_DELETE,
// };

// static const char *sql[] = {
//     [BY_TIMESTAMP_DELETE]   = "delete from %s where %s < ?",
//     [BY_LINE_GET_NUM]       = "select count(*) from %s",
//     [BY_LINE_DELETE]        = "delete from %s where id < (select id from %s limit 1 offset ?)",
// };

// static void by_timestamp_bind_cb(sqlite3 *db, sqlite3_stmt *pstmt, void *userp)
// {
//     int64_t when = (int64_t)userp;
//     DAO_RETURN_IS_OK(sqlite3_bind_int64(pstmt, 1, when));
//     return;
// }

// static bool by_timestamp(retention_policy_t *r)
// {
//     if (NULL == r)
//     {
//         d_log("api misuse");
//         return false;
//     }

//     rp_by_timestamp_t *option = (rp_by_timestamp_t*)r->userp;
//     prepared_stmt_t *ps = option->ps;
//     const char *table_name = option->table_name;

//     UT_string *s = NULL;
//     utstring_new(s);
//     utstring_printf(s, sql[BY_TIMESTAMP_DELETE], table_name, option->where_field);

//     int64_t when = (time(0) - option->expire_s);
//     int32_t real_delete_num = dao_template_delete(ps, utstring_body(s), by_timestamp_bind_cb, (void*)when);
//     utstring_free(s);
//     if (-1 == real_delete_num)
//     {
//         d_log("sql_err table_name=%s when=%ld", table_name, when);
//         return false;
//     }
//     else if (0 == real_delete_num)
//     {
//         return true;
//     }
//     else
//     {
//         d_log("table_name=%s real_delete_num=%d", table_name, real_delete_num);
//         return true;
//     }
//     return true;
// }

// static void get_line_num_unmarshal_cb(sqlite3_stmt *pstmt, void *userp)
// {
//     int32_t *line_num = (int32_t*)userp;
//     *line_num = sqlite3_column_int(pstmt, 0);
//     return;
// }

// static int32_t get_line_num(prepared_stmt_t *ps, const char *table_name, int32_t *line_num)
// {
//     UT_string *s = NULL;
//     utstring_new(s);
//     utstring_printf(s, sql[BY_LINE_GET_NUM], table_name);
//     int32_t ret = dao_template_get(ps, utstring_body(s), NULL, NULL, get_line_num_unmarshal_cb, line_num);
//     if (ret <= 0)
//     {
//         d_log("sql_err table_name=%s ret=%d", table_name, ret);
//     }
//     utstring_free(s);
//     return ret;
// }

// static void by_line_bind_cb(sqlite3 *db, sqlite3_stmt *pstmt, void *userp)
// {
//     int32_t try_delete_num = (int32_t)(int64_t)userp;
//     DAO_RETURN_IS_OK(sqlite3_bind_int(pstmt, 1, try_delete_num));
//     return;
// }

// static bool by_line(retention_policy_t *r)
// {
//     if (NULL == r)
//     {
//         d_log("api misuse");
//         return false;
//     }

//     rp_by_line_t *option = (rp_by_line_t*)r->userp;
//     prepared_stmt_t *ps = option->ps;
//     const char *table_name = option->table_name;

//     int32_t cur_line_num = 0;
//     int32_t ret = get_line_num(ps, table_name, &cur_line_num);
//     if (ret <= 0)
//     {
//         d_log("line_num error table_name=%s ret=%d", table_name, ret);
//         return false;
//     }

//     if (cur_line_num <= option->max_num)
//     {
//         return true;
//     }

//     UT_string *s = NULL;
//     utstring_new(s);
//     utstring_printf(s, sql[BY_LINE_DELETE], table_name, table_name);

//     int32_t try_delete_num = cur_line_num - (option->max_num * option->percent / 100);
//     int32_t real_delete_num = dao_template_delete(ps, utstring_body(s), by_line_bind_cb, (void*)(int64_t)try_delete_num);
//     utstring_free(s);
//     if (-1 == real_delete_num)
//     {
//         d_log("sql_err table_name=%s cur_line_num=%d max_num=%d", table_name, cur_line_num, option->max_num);
//         return false;
//     }
//     else if (real_delete_num != try_delete_num)
//     {
//         d_log("table_name=%s try_delete=%d real_delete=%d cur_line_num=%d max_num=%d",
//                     table_name,
//                     try_delete_num,
//                     real_delete_num,
//                     cur_line_num,
//                     option->max_num);
//         return false;
//     }
//     else
//     {
//         d_log("table_name=%s cur_line_num=%d real_delete_num=%d", table_name, cur_line_num, real_delete_num);
//         return true;
//     }
//     return true;
// }

// static bool always(retention_policy_t *r)
// {
//     (void)r;
//     return true;
// }

// bool new_retention_policy_by_line(retention_policy_t *r, prepared_stmt_t *ps, const char *table_name, int32_t max_num, int32_t percent)
// {
//     if ((NULL == r)
//     || (NULL == ps)
//     || (NULL == table_name))
//     {
//         d_log("api misuse");
//         return false;
//     }

//     if ((percent < 0)
//     || (percent > 100))
//     {
//         d_log("api misuse");
//         return false;
//     }

//     rp_by_line_t *userp = (rp_by_line_t*)malloc(sizeof(rp_by_line_t));
//     if (NULL == userp)
//     {
//         d_log("malloc error");
//         return false;
//     }

//     userp->table_name = strdup(table_name);
//     if (NULL == userp->table_name)
//     {
//         d_log("malloc error");
//         free(userp);
//         return false;
//     }

//     userp->max_num = max_num;
//     userp->percent = percent;
//     userp->ps = ps;
//     r->userp = userp;
//     r->run = by_line;
//     return true;
// }

// void delete_retention_policy_by_line(retention_policy_t *r)
// {
//     if (NULL == r)
//     {
//         d_log("api misuse");
//         return;
//     }

//     if (NULL != r->userp)
//     {
//         rp_by_line_t *option = (rp_by_line_t*)r->userp;
//         if (NULL != option->table_name)
//         {
//             free((void*)option->table_name);
//             option->table_name = NULL;
//         }

//         free(r->userp);
//         r->userp = NULL;
//     }
//     r->run = NULL;
//     return;
// }

// bool new_retention_policy_by_timestamp(retention_policy_t *r, prepared_stmt_t *ps, const char *table_name, const char *where_field, int64_t expire_s)
// {
//     if ((NULL == r)
//     || (NULL == ps)
//     || (NULL == table_name)
//     || (NULL == where_field))
//     {
//         d_log("api misuse");
//         return false;
//     }

//     rp_by_timestamp_t *userp = (rp_by_timestamp_t*)malloc(sizeof(rp_by_timestamp_t));
//     if (NULL == userp)
//     {
//         d_log("malloc error");
//         return false;
//     }

//     userp->table_name = strdup(table_name);
//     if (NULL == userp->table_name)
//     {
//         d_log("malloc error");
//         free(userp);
//         return false;
//     }

//     userp->where_field = strdup(where_field);
//     if (NULL == userp->table_name)
//     {
//         d_log("malloc error");
//         free((void*)userp->table_name);
//         free(userp);
//         return false;
//     }

//     userp->expire_s = expire_s;
//     userp->ps = ps;
//     r->userp = userp;
//     r->run = by_timestamp;
//     return true;
// }

// void delete_retention_policy_by_timestamp(retention_policy_t *r)
// {
//     if (NULL == r)
//     {
//         d_log("api misuse");
//         return;
//     }

//     if (NULL != r->userp)
//     {
//         rp_by_timestamp_t *option = (rp_by_timestamp_t*)r->userp;
//         if (NULL != option->table_name)
//         {
//             free((void*)option->table_name);
//             option->table_name = NULL;
//         }

//         if (NULL != option->where_field)
//         {
//             free((void*)option->where_field);
//             option->where_field = NULL;
//         }

//         free(r->userp);
//         r->userp = NULL;
//     }
//     r->run = NULL;
//     return;
// }

// bool new_retention_policy_always(retention_policy_t *r)
// {
//     if (NULL == r)
//     {
//         d_log("api misuse");
//         return false;
//     }

//     r->userp = NULL;
//     r->run = always;
//     return true;
// }

// void delete_retention_policy_always(retention_policy_t *r)
// {
//     if (NULL == r)
//     {
//         d_log("api misuse");
//         return;
//     }
//     r->run = NULL;
//     return;
// }