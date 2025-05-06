#ifndef __SQL_TRACE_H__
#define __SQL_TRACE_H__

#include <stdint.h>
#include "sqlite3/sqlite3.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief 向db注册一个trace hook；
 * @details 每当db执行完sql语句后，打印这条sql和它的耗时
 */
void add_sql_trace(sqlite3 *db, const char *label);

/**
 * @brief 初始化全局错误记录，每个进程只能有一个
 * @details 以下是可能出现的部分列表:
 * 任何时候编译 SQL 语句（使用sqlite3_prepare_v2()或其兄弟）或运行 SQL 语句（使用sqlite3_step()）时都会记录该错误
 * 当发生需要重新分析和重新准备准备好的语句的模式更改时，该事件将被记录为错误代码 SQLITE_SCHEMA
 * 每当必须恢复数据库时，都会记录 SQLITE_NOTICE 消息，因为先前的编写器在未完成其事务的情况下崩溃
 * 当数据库文件以可能导致数据库损坏的方式重命名或别名时，将记录 SQLITE_WARNING 消息
 * 内存不足 (OOM) 错误情况会生成带有 SQLITE_NOMEM 错误代码的错误日志记录事件和一条消息，说明失败的分配请求了多少字节的内存
 * 操作系统接口中的 I/O 错误会生成错误记录事件。这些事件的消息给出了源代码中错误产生的行号，以及在存在相应文件时与事件关联的文件名
 * 当检测到数据库损坏时，将调用 SQLITE_CORRUPT 错误记录器回调。对于 I/O 错误，错误消息文本包含原始源代码中首次检测到错误的行号
 * SQLITE_MISUSE 错误调用错误记录器回调
 */
int32_t sql_global_error_init(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif