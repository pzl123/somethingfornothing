#include "lru.h"
#include "database/dao/pcu_relay_cnt/pcu_relay_cnt.h"
#include <stdio.h>

void hash_add_or_update(relay_dao_t **user, const UT_icd *icd, const char *key, int value)
{
    relay_dao_t *s = NULL;
    HASH_FIND_STR(*user, key, s);
    if (s)
    {
        s->cnt_value = value;
        printf("Updated key: %s -> %d\n", key, value);
    }
    else
    {
        s = (relay_dao_t *)malloc(icd->sz);
        icd->init(s);
        s->cnt_value = value;
        s->key = strdup(key);

        HASH_ADD_KEYPTR(hh, *user, s->key, strlen(s->key), s);
        printf("Added new key: %s -> %d\n", key, value);
    }
}

int32_t hash_find(relay_dao_t **user, const char *key)
{
    relay_dao_t *s = NULL;
    HASH_FIND_STR(*user, key, s);
    if (s)
    {
        printf("Found key: %s -> %d\n", key, s->cnt_value);
        return s->cnt_value;
    }
    else
    {
        printf("Key not found: %s\n", key);
        return -1;
    }
}

void hash_delete(relay_dao_t **user, const UT_icd *icd, const char *key)
{
    relay_dao_t *s = NULL;
    HASH_FIND_STR(*user, key, s);
    if (s)
    {
        HASH_DEL(*user, s);
        icd->dtor(s);
        free(s);
        printf("Deleted key: %s\n", key);
    }
    else
    {
        printf("Key not found for deletion: %s\n", key);
    }
}


void hash_print_all(relay_dao_t **user)
{
    relay_dao_t *current, *tmp;
    printf("Current hash table:\n");
    HASH_ITER(hh, *user, current, tmp)
    {
        printf("Key: %s, Value: %d\n", current->key, current->cnt_value);
    }
    printf("\n");
}

void hash_clear_all(relay_dao_t **user,  const UT_icd *icd)
{
    relay_dao_t *current, *tmp;
    HASH_ITER(hh, *user, current, tmp)
    {
        HASH_DEL(*user, current);
        icd->dtor(current);
        free(current);
    }
    printf("All entries cleared.\n");
}
