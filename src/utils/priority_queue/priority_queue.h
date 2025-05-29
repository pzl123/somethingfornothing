#ifndef __PRIORITY_QUEUE_H__
#define __PRIORITY_QUEUE_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    int _key; /* 优先级 */
    void *_value; /* 存储的值 */
} key_value_t;

typedef struct
{
    key_value_t *elements;         // 存储元素的数组
    int capacity;                  // 最大容量
    int size;                      // 当前元素个数
    int32_t (*compare)(key_value_t* A, key_value_t *b);
} pq_t;

pq_t *pq_init(int32_t capacity, int32_t (*compare)(key_value_t* A, key_value_t *b));
int32_t max_heap_compare(key_value_t *a, key_value_t *b);
int32_t min_heap_compare(key_value_t *a, key_value_t *b);
bool priority_queue_push(pq_t *pq, key_value_t item);
key_value_t pq_top(pq_t *pq);
bool pq_empty(pq_t *pq);
bool priority_queue_pop(pq_t *pq);
#ifdef __cplusplus
}
#endif

#endif /* __PRIORITY_QUEUE_H__ */
