#ifndef CONFIG_CMP_KEY
#define CONFIG_CMP_KEY

#include "cjsonx/cJSON.h"

#ifdef __cplusplus
extern "C" {
#endif

void test_config_cmp_key(void);

/**
 * @brief 以 a 为模板，创建新对象 a1，保留 b 中相同 key 的值
 * @param a 模板对象
 * @param b 值来源对象
 * @param case_sensitive 是否大小写敏感
 * @return 新创建的对象 a1，结构与 a 一致，值部分来自 b
 */
CJSON_PUBLIC(cJSON*) cJSON_MergeWithTemplate(const cJSON *a, const cJSON *b, cJSON_bool case_sensitive);

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_CMP_KEY */

