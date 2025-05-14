#ifndef __DSN_H__
#define __DSN_H__




#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* 数据源名称(data source name)，包含了连接到数据库所需的所有必要信息 */

typedef enum
{
    DSN_MAIN = 0,
    DSN_END,
}db_dsn_e;

const char* dsn_to_string(db_dsn_e dsn);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __DSN_H__ */
