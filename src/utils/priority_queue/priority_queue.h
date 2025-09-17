#ifndef __PRIORITY_QUEUE_H__
#define __PRIORITY_QUEUE_H__

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct
{
    int32_t _priority; /* 原始优先级 */
    void *_value; /* 存储的值 */
    uint64_t timestamp; /* 入队时间 */
} pv_t;

typedef struct
{
    pv_t *elements; // 存储元素的数组
    int capacity;   // 最大容量
    int size;       // 当前元素个数
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int32_t (*compare)(pv_t* A, pv_t *b);
} pq_t;

pq_t *pq_init(int32_t capacity, int32_t (*compare)(pv_t* A, pv_t *b));
bool pq_delete(pq_t *pq);
int32_t max_heap_compare(pv_t *a, pv_t *b);
int32_t min_heap_compare(pv_t *a, pv_t *b);
int32_t aging_compare(pv_t *a, pv_t *b);
bool priority_queue_push(pq_t *pq, pv_t item);
pv_t pq_top(pq_t *pq);
bool pq_empty(pq_t *pq);
bool pq_full(pq_t *pq);
bool priority_queue_pop(pq_t *pq, pv_t *item);
bool priority_queue_try_pop(pq_t *pq, pv_t *item);



#ifdef __cplusplus
}
#endif

#endif /* __PRIORITY_QUEUE_H__ */
