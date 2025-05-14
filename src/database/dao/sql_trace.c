#include "sql_trace.h"
#include <stdio.h>
#include "utils/utils.h"
static int32_t trace_profile_cb(uint32_t mask, void *userp, void *p, void *elapse_ptr)
{
    (void)mask;
    sqlite3_stmt *pstmt = (sqlite3_stmt*)p;
    const char *sql = sqlite3_sql(pstmt);
    const char *label = (const char*)userp;
    int32_t elapse = (*(int64_t*)elapse_ptr / 1000000);
    d_log("label=%s elapse=%d sql=%s", label, elapse, sql);
    return 0;
}

void add_sql_trace(sqlite3 *db, const char *label)
{
    sqlite3_trace_v2(db, SQLITE_TRACE_PROFILE, trace_profile_cb, (void*)label);
    return;
}

static void global_error_cb(void *userp, int32_t code, const char *msg)
{
    (void)userp;
    switch (code)
    {
    case SQLITE_SCHEMA:
        d_log("code=%d sql_err=%s", code, msg);
        break;
    default:
        d_log("code=%d sql_err=%s", code, msg);
        break;
    }
    return;
}

int32_t sql_global_error_init(void)
{
    return sqlite3_config(SQLITE_CONFIG_LOG, global_error_cb);
}

