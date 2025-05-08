#pragma once

#include <stdbool.h>

#include "uthash/utarray.h"
#include "database/dao/pcu_relay_cnt/pcu_relay_cnt.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
    基于 uthash 的 LRU 缓存实现
+------------------+
|   lru_cache_t    |
+------------------+
| hash: lru_hash_t*|
| list: lru_list_t*|
| max_key_num      |
| max_mem          |
| used_mem         |
+------------------+

           ↓
+------------------+     +------------------+
|   lru_hash_t     |<--->|   lru_list_t     |
+------------------+     +------------------+
| key              |     | hash_node        |
| key_len          |     | item             |
| list_node        |     | prev             |
| hh               |     | next             |
+------------------+     +------------------+

                             ↓
                     +------------------+
                     |   cache_item_t   |
                     +------------------+
                     | icd              |
                     | data             |
                     +------------------+
 */

/* 哈希表节点，用于快速查找 */
typedef struct lru_hash_t lru_hash_t;
/* 双向链表节点，用于维护访问顺序 */
typedef struct lru_list_t lru_list_t;
/* 缓存管理器，包含哈希表和链表，以及容量控制参数 */
typedef struct lru_cache_t lru_cache_t;

 /* 存储缓存数据本身 */
typedef struct
{
    UT_icd icd;
    void *data; /* 实际数据 */
} cache_item_t;

struct lru_cache_t
{
    lru_hash_t *hash;     /* 指向哈希表头节点，用于快速查找缓存项 */
    lru_list_t *list;     /* 指向双向链表头节点，用于维护访问顺序 */
    int32_t max_key_num;  /* 最多允许存储的键数量（容量限制之一） */
    int64_t max_mem;      /* 缓存最多允许使用的内存总量（以字节为单位） */
    int64_t used_mem;     /* 当前已使用的缓存内存大小 */
};

/**
 * @brief 创建一个cache
 *
 * @param cache 操作的目标缓存对象
 * @param max_key_num 最大缓存key数量
 * @param max_mem 最大缓存的内存
 * @return true
 * @return false
 */
bool new_lru_cache(lru_cache_t *cache, int32_t max_key_num, int64_t max_mem);

/**
 * @brief 删除指定cache
 *
 * @param cache 操作的目标缓存对象
 */
void delete_lru_cache(lru_cache_t *cache);

/**
 * @brief 从cache中删除指定key 的item
 *
 * @param cache 操作的目标缓存对象
 * @param key
 */
void lru_cache_remove(lru_cache_t *cache, const char *key);

/**
 * @brief 从cache中获取指定key的item
 *
 * @param cache 操作的目标缓存对象
 * @param key
 * @return cache_item_t* 获取的指定item的地址
 */
cache_item_t* lru_cache_get(lru_cache_t *cache, const char *key);

/**
 * @brief 向缓存中插入一个 int64_t 类型的值
 *
 * 将给定的 64 位整数值与指定键关联并存入缓存中。
 * 若缓存已满，则根据 LRU 策略移除最近最少使用的项后再插入新项。
 *
 * @param cache 操作的目标缓存对象
 * @param key   键字符串，用于唯一标识该缓存项
 * @param i64   要插入的 64 位整数值
 * @return      成功插入返回 true；若内存不足或键无效则返回 false
 */
bool lru_cache_put_int64(lru_cache_t *cache, const char *key, int64_t i64);

/**
 * @brief 向缓存中插入一个字符串类型的值
 *
 * 该函数会将字符串拷贝进缓存内部内存中，原字符串在函数调用后可被安全释放。
 * 若缓存已满，则根据 LRU 策略移除最近最少使用的项后再插入新项。
 *
 * @param cache 操作的目标缓存对象
 * @param key   键字符串，用于唯一标识该缓存项
 * @param str   要插入的字符串值(NUL 终止)
 * @return      成功插入返回 true；若内存不足、键无效或字符串为空则返回 false
 */
bool lru_cache_put_str(lru_cache_t *cache, const char *key, const char *str);

/**
 * @brief 向缓存中插入一个结构体类型的数据
 *
 * 将指定大小的结构体数据拷贝到缓存中，并与给定键关联。
 * 若缓存已满，则根据 LRU 策略移除最近最少使用的项后再插入新项。
 *
 * @param cache 操作的目标缓存对象
 * @param key   键字符串，用于唯一标识该缓存项
 * @param sz    结构体的字节大小
 * @param data  指向结构体数据的指针(不能为 NULL)
 * @return      成功插入返回 true；若内存不足、参数无效或拷贝失败则返回 false
 */
bool lru_cache_put_struct(lru_cache_t *cache, const char *key, int32_t sz, const void *data);

#ifdef __cplusplus
}
#endif /* __cplusplus */
