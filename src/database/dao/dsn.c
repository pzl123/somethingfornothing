#include "dsn.h"


static const char *map[] = {
    [DSN_MAIN]          = "../../db/main.db",
};

const char* dsn_to_string(db_dsn_e dsn)
{
    return map[dsn];
}
