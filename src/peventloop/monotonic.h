#ifndef __MONOTONIC_H__
#define __MONOTONIC_H__
/* 单调时钟（monotonic clock）是一个**始终递增**的时钟源。它与实际的“一天中的时间”无关，**只能用于相对时间的测量**。
单调时钟也不能保证在时间上是完全精确的；可能会存在轻微的偏差或偏移。
根据系统架构的不同，通过使用 CPU 的指令计数器（如 x86 架构下的 RDTSC 指令），获取单调时间的速度可能比普通时钟源快得多。
 */
#include <stdint.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif
/* 这是一个以微秒为单位的计数器。'monotime' 类型用于表示那些保存**单调时间（monotonic time）**的变量。
这个类型的引入有助于明确和文档化：该变量与“单调时钟”相关，不应与其他类型的时间值混淆 */
typedef uint64_t monotime;

/* 获取相对于某个任意时间点的微秒计数器 */
extern monotime (*getMonotonicUs)(void);

typedef enum monotonic_clock_type {
    MONOTONIC_CLOCK_POSIX,
    MONOTONIC_CLOCK_HW,
} monotonic_clock_e;

/* 在程序启动时调用一次，用于初始化单调时钟（monotonic clock）
虽然这个函数只需要被调用一次，但也可以多次调用而不产生影响
它返回一个可打印的字符串，表示已初始化的时钟类型。返回的字符串是静态的，不需要手动释放 */
const char *monotonicInit(void);

/* 返回一个字符串，指示所使用的单调时钟的类型 */
const char *monotonicInfoString(void);

/* 返回正在使用的单调时钟的类型 */
monotonic_clock_e monotonicGetType(void);

/* Functions to measure elapsed time.  Example:
 *     monotime myTimer;
 *     elapsedStart(&myTimer);
 *     while (elapsedMs(myTimer) < 10) {} // loops for 10ms
 */
static inline void elapsedStart(monotime *start_time)
{
    *start_time = getMonotonicUs();
}

static inline uint64_t elaspedUs(monotime start_time)
{
    return getMonotonicUs() - start_time;
}

static inline uint64_t elapsedMS(monotime start_time)
{
    return elaspedUs(start_time) / 1000;
}

#ifdef __cplusplus
}
#endif

#endif /* __MONOTONIC_H__ */
