#ifndef CONFIG_MANAGE_H
#define CONFIG_MANAGE_H

#include <stdbool.h>

#include "uthash/uthash.h"
#include "cjsonx/cJSONx.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* 配置文件路径 */
#define CONFIG_PATH "/home/zlgmcu/project/learnC++/src/config/config_path/current"
/* 默认配置文件路径 */
#define DEFAULT_CONFIG_PATH "/home/zlgmcu/project/learnC++/src/config/config_path/default"

typedef enum
{
    CFG_DEFAULT = 0,
    CFG_CURRENT
} config_type_e;

typedef void (*on_config_change)(cJSON *old_value, cJSON *new_value);

/**
 * @brief 初始化配置模块
 *
 * @return bool
 */
bool config_manage_init(void);

/**
 * @brief 清理配置模块
 */
void config_manage_clear(void);


/**
 * @brief 设置默认配置
 *
 * @param name 配置表名
 * @param config 默认配置
 * @return bool
 */
bool set_default_config(const char *name, cJSON *config);

/**
 * @brief 获取默认配置,外部管理内存释放
 *
 * @param name 配置表名
 * @return 正确返回，是一个cJSON对象，错误返回NULL
 */
cJSON *get_default_config(const char *name);

/**
 * @brief Get the default config str object
 *
 * @param name config table name
 * @return char*
 */
char *get_default_config_str(const char *name);

/**
 * @brief 设置配置
 *
 * @param name 配置表名
 * @param config 配置
 * @return bool
 */
bool set_config(const char *name, cJSON *config);

/**
 * @brief 获取配置,外部管理内存释放
 *
 * @param name 配置表名
 * @return 正确返回，是一个cJSON对象，错误返回NULL
 */
cJSON *get_config(const char *name);

/**
 * @brief 替换调config hash表中 name 的json对象
 *
 * @param name 被替换者
 * @param config 替换对象
 * @param config_type 替换对象所属范围
 * @return true
 * @return false
 */
bool replace_object(const char *name, cJSON *config, config_type_e config_type);

/**
 * @brief Get the config str object
 *
 * @param name config table name
 * @return char*
 */
char *get_config_str(const char *name);

/**
 * @brief 注册配置变更回调
 *
 * @param name 配置表名
 * @param cb 回调函数
 */
bool config_attach(const char *name, on_config_change cb);

/**
 * @brief 取消注册配置变更回调
 *
 * @param name 配置表名
 * @param cb 回调函数
 */
bool config_detach(const char *name, on_config_change cb);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _CONFIG_MANAGE_H_ */
