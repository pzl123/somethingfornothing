#include "table.h"

#include <stdio.h>

static const table_t table_pcu_relay_cnt =
{
    .table_name = "pcu_relay_cnt",
    .dsn = DSN_MAIN,
    .create_table_stmt =
        "CREATE TABLE IF NOT EXISTS pcu_relay_cnt ("
        "relay_id INTEGER PRIMARY KEY,"
        "close_cnt INTEGER"
        ");",
};

static const table_t tables[] = {
    table_pcu_relay_cnt,
};

bool get_tables(const table_t **head, int32_t *len)
{
    if ((NULL == head) || (NULL == len))
    {
        printf("api misuse");
        return false;
    }
    *head = tables;
    *len = sizeof(tables)/sizeof(tables[0]);
    return true;
}