#include "lru.h"
#include "uthash/utstring.h"
#include "uthash/uthash.h"
#include "uthash/utlist.h"
#include "utils/utils.h"

struct lru_list_t
{
    lru_hash_t *hash_node;
    cache_item_t *item;
    lru_list_t *prev, *next;
};

struct lru_hash_t
{
    const char *key;
    int32_t key_len;
    lru_list_t *list_node;
    UT_hash_handle hh;
};

static cache_item_t* new_lru_cache_item(const UT_icd *icd)
{
    cache_item_t *item = (cache_item_t*)malloc(sizeof(cache_item_t));
    if (NULL == item)
    {
        d_log("malloc error");
        return NULL;
    }

    item->data = malloc(icd->sz);
    if (NULL == item->data)
    {
        d_log("malloc error sz=%ld", icd->sz);
        free(item);
        return NULL;
    }

    item->icd = *icd;
    if (NULL != icd->init)
    {
        icd->init(item->data);
    }
    else
    {
        (void)memset(item->data, 0, icd->sz);
    }
    return item;
}



static void copy_lru_cache_item(cache_item_t *i, const void *src)
{
    if (NULL != i->icd.copy)
    {
        i->icd.copy(i->data, src);
    }
    else
    {
        (void)memcpy(i->data, src, i->icd.sz);
    }
    return;
}

static bool replace_lru_cache_item(cache_item_t *i, const UT_icd *icd, const void *src)
{
    void *dst = malloc(icd->sz);
    if (NULL == dst)
    {
        d_log("malloc error sz=%ld", icd->sz);
        return false;
    }

    if (NULL != icd->init)
    {
        icd->init(dst);
    }
    else
    {
        (void)memset(dst, 0, icd->sz);
    }

    if (NULL != icd->copy)
    {
        icd->copy(dst, src);
    }
    else
    {
        (void)memcpy(dst, src, icd->sz);
    }

    if (NULL != i->icd.dtor)
    {
        i->icd.dtor(i->data);
    }
    else
    {
        free(i->data);
    }

    i->data = dst;
    i->icd = *icd;
    return true;
}


static void print_cache(lru_cache_t *cache)
{
    lru_list_t *elt = NULL;
    int32_t cnt = 0;
    DL_COUNT(cache->list, elt, cnt);
    d_log("used=%ld byte:%0.3f%%  key_count=%d", cache->used_mem, (float32_t)((float32_t)cache->used_mem/(float32_t)cache->max_mem), cnt);

    {
        lru_list_t *tmp = NULL;
        UT_string *s = NULL;
        utstring_new(s);
        DL_FOREACH_SAFE(cache->list, elt, tmp) {
            utstring_printf(s, "[%s]->", elt->hash_node->key);
        }
        d_log("%s", utstring_body(s));
        utstring_free(s);
    }

    return;
}

static void move_to_head(lru_list_t **head, lru_list_t **elt)
{
    DL_DELETE((*head), (*elt));
    DL_PREPEND((*head), (*elt));
    return;
}

cache_item_t* lru_cache_get(lru_cache_t *cache, const char *key)
{
    if ((NULL == cache)
    || (NULL == key))
    {
        d_log("api misuse");
        return NULL;
    }

    lru_hash_t *elt = NULL;
    HASH_FIND_STR(cache->hash, key, elt);
    if (NULL == elt)
    {
        return NULL;
    }

    move_to_head(&cache->list, &elt->list_node);
    print_cache(cache);
    return elt->list_node->item;
}

static bool is_full(lru_cache_t *cache)
{
    if (cache->used_mem >= cache->max_mem)
    {
        d_log("used_mem=%ld max_mem=%ld", cache->used_mem, cache->max_mem);
        return true;
    }

    lru_list_t *elt = NULL;
    int32_t cnt = 0;
    DL_COUNT(cache->list, elt, cnt);

    if (cnt >= cache->max_key_num)
    {
        return true;
    }
    return false;
}

static void delete_lru_cache_item(cache_item_t *i)
{
    if (NULL != i->icd.dtor)
    {
        i->icd.dtor(i->data);
    }
    else
    {
        free(i->data);
    }
    free(i);
    return;
}

static void delete_data(lru_cache_t *cache, lru_list_t *dst)
{
    DL_DELETE(cache->list, dst);
    HASH_DEL(cache->hash, dst->hash_node);

    cache->used_mem -= sizeof(lru_list_t) + sizeof(lru_hash_t) + (dst->hash_node->key_len + 1) + sizeof(cache_item_t) + dst->item->icd.sz;

    free((void*)dst->hash_node->key);
    free(dst->hash_node);

    delete_lru_cache_item(dst->item);
    free(dst);
    return;
}

static bool create_data(lru_cache_t *cache, const char *key, const UT_icd *icd, const void *data)
{
    lru_list_t *list_elt = (lru_list_t*)malloc(sizeof(lru_list_t));
    if (NULL == list_elt)
    {
        d_log("malloc error key=%s", key);
        return false;
    }

    lru_hash_t *hash_elt = (lru_hash_t*)malloc(sizeof(lru_hash_t));
    if (NULL == hash_elt)
    {
        d_log("malloc error key=%s", key);
        free(list_elt);
        return false;
    }

    int32_t key_len = strlen(key);
    char *key_copy = (char*)malloc(key_len + 1);
    if (NULL == key_copy)
    {
        d_log("malloc error key=%s", key);
        free(list_elt);
        free(hash_elt);
        return false;
    }

    cache_item_t *item = new_lru_cache_item(icd);
    if (NULL == item)
    {
        d_log("malloc error key=%s", key);
        free(list_elt);
        free(hash_elt);
        free(key_copy);
        return false;
    }

    copy_lru_cache_item(item, data);

    (void)memcpy(key_copy, key, key_len);
    key_copy[key_len] = '\0';

    hash_elt->list_node = list_elt;
    hash_elt->key = key_copy;
    hash_elt->key_len = key_len;
    HASH_ADD_STR(cache->hash, key, hash_elt);

    list_elt->hash_node = hash_elt;
    list_elt->item = item;
    DL_PREPEND(cache->list, list_elt);

    cache->used_mem += sizeof(lru_list_t) + sizeof(lru_hash_t) + (key_len + 1) + sizeof(cache_item_t) + icd->sz;
    return true;
}

static bool update_data(lru_cache_t *cache, cache_item_t *item, const UT_icd *icd, const void *src)
{
    if (0 == memcmp(&item->icd, icd, sizeof(UT_icd)))
    {
        if (0 == memcmp(item->data, src, icd->sz))
        {
            d_log("not charge");
            return true;
        }
    }

    int32_t charge = (icd->sz - item->icd.sz);
    bool ret = replace_lru_cache_item(item, icd, src);
    if (false == ret)
    {
        return false;
    }

    cache->used_mem += charge;
    return true;
}

static void delete_tail_data(lru_cache_t *cache)
{
    lru_list_t *tail = cache->list->prev; /* 双向链表, head->prev 指向 tail 若只有一个 指向自己 */
    delete_data(cache, tail);
    return;
}

bool lru_cache_put(lru_cache_t *cache, const char *key, const UT_icd *icd, const void *data)
{
    if ((NULL == cache)
    || (NULL == key)
    || (NULL == data)
    || (NULL == icd))
    {
        d_log("api misuse");
        return false;
    }

    if (icd->sz <= 0)
    {
        d_log("api misuse");
        return false;
    }

    lru_hash_t *elt = NULL;
    HASH_FIND_STR(cache->hash, key, elt);
    if (NULL != elt)
    {
        if (false == update_data(cache, elt->list_node->item, icd, data))
        {
            d_log("update error key=%s", key);
            return false;
        }

        move_to_head(&cache->list, &elt->list_node);
        return true;
    }

    if (true == is_full(cache))
    {
        delete_tail_data(cache);
    }

    bool ret = create_data(cache, key, icd, data);
    print_cache(cache);
    return ret;
}

bool lru_cache_put_int64(lru_cache_t *cache, const char *key, int64_t i64)
{
    UT_icd icd = {0};
    icd.sz = sizeof(int64_t);
    return lru_cache_put(cache, key, &icd, &i64);
}

bool lru_cache_put_str(lru_cache_t *cache, const char *key, const char *str)
{
    int32_t len = strlen(str);
    UT_icd icd = {0};
    icd.sz = len + 1;
    return lru_cache_put(cache, key, &icd, str);
}

bool lru_cache_put_struct(lru_cache_t *cache, const char *key, int32_t sz, const void *data)
{
    UT_icd icd = {0};
    icd.sz = sz;
    return lru_cache_put(cache, key, &icd, data);
}

void lru_cache_remove(lru_cache_t *cache, const char *key)
{
    if ((NULL == cache)
    || (NULL == key))
    {
        d_log("api misuse");
        return;
    }

    lru_hash_t *elt = NULL;
    HASH_FIND_STR(cache->hash, key, elt);
    if (NULL == elt)
    {
        d_log("key=%s", key);
        return;
    }

    delete_data(cache, elt->list_node);
    print_cache(cache);
    return;
}


void delete_lru_cache(lru_cache_t *cache)
{
    if (NULL == cache)
    {
        d_log("api misuse");
        return;
    }

    {
        lru_list_t *elt = NULL, *tmp = NULL;
        DL_FOREACH_SAFE(cache->list, elt, tmp)
        {
            DL_DELETE(cache->list, elt);
            delete_lru_cache_item(elt->item);
            free(elt);
        }
    }

    {
        lru_hash_t *elt = NULL, *tmp = NULL;
        HASH_ITER(hh, cache->hash, elt, tmp)
        {
            HASH_DEL(cache->hash, elt);
            free((void*)elt->key);
            free(elt);
        }
    }

    cache->hash = NULL;
    cache->list = NULL;
    return;
}

bool new_lru_cache(lru_cache_t *cache, int32_t max_key_num, int64_t max_mem)
{
    if ((NULL == cache)
    || (max_key_num <= 0)
    || (max_mem <= 0))
    {
        d_log("api misuse");
        return false;
    }

    cache->hash = NULL;
    cache->list = NULL;
    cache->max_key_num = max_key_num;
    cache->max_mem = max_mem;
    cache->used_mem = 0;
    return true;
}
