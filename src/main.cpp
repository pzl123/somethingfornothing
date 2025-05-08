#include <iostream>

#include "database/sqlite3_base.h"
#include "utils/cache/lru.h"
#include "database/dal.h"
#include "utils/utils.h"



int main(void)
{
    lru_cache_t cache = {0};
    new_lru_cache(&cache, 10, 10 * 1000);

    lru_cache_put_int64(&cache, "key1", 64);

    lru_cache_put_str(&cache, "key2", "hello lru");

    struct tmpa
    {
        int32_t aa;
        char name[24];
    };
    struct tmpa ss = {32, "aaa"};

    lru_cache_put_struct(&cache, "key3", sizeof(ss), (void *)&ss);

    lru_cache_remove(&cache, "key1");

    delete_lru_cache(&cache);
    return 0;
}