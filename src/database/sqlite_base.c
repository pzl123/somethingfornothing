#include "sqlite3_base.h"
#include "sqlite3/sqlite3.h"
#include "database/dao/dsn.h"
#include "database/dao/sql_trace.h"
#include "database/dao/template.h"

#include <stdbool.h>


bool new_dataserver(void)
{
    (void)sql_global_error_init();
    sqlite3 *db = NULL;
    const char *path = dsn_to_string(DSN_MAIN);
    if (SQLITE_OK != sqlite3_open(path, &db))
    {
        printf("open %s error", path);
        return false;
    }
    add_sql_trace(db, path);
    DAO_RETURN_IS_OK(sqlite3_busy_timeout(db, 10 * 1000));
    return true;
}
