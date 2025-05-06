#include "dsn.h"


static const char *map[] = {
    [DSN_MAIN]          = "../../db/main.db",
    [DSN_TRANSACTION]   = "../../db/transaction.db",
    [DSN_LC_INFO]       = "../../db/lc_info.db",
};

const char* dsn_to_string(db_dsn_e dsn)
{
    return map[dsn];
}
