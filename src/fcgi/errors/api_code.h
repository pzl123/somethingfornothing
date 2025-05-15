#ifndef __API_CODE_H__
#define __API_CODE_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    API_ERR_OK = 0,
    API_ERR_POINTER,                    /* 指针错误 */
    API_ERR_DB,                         /* 数据库错误 */
    API_ERR_INTERNAL,                   /* 内部错误 */
    API_ERR_METHOD,                     /* 请求方法错误 */
    API_ERR_URI,                        /* uri 错误 */
    API_ERR_JSON,                       /* json 格式错误 */
    API_ERR_NULL_DATA,                  /* 请求体为空 */
    API_ERR_TOKEN_INVALID,              /* 无效token */
    API_ERR_TOKEN_EXPIRED,              /* 过期token */
    API_ERR_PERMISSION ,                /* 权限不足 */
    API_ERR_CONTENT_TYPE,               /* content_type 错误 */
    API_ERR_CONTENT_LENGTH,             /* content_length 错误 */
    API_ERR_BOUNDARY,                   /* boundary 错误 */
    API_ERR_FILE_NAME,                  /* 文件名错误 */
    API_ERR_JSON_TYPE,                  /* json 类型错误(来自 cjsonx) */
    API_ERR_MISSING_FIELD,              /* 缺少字段(来自 cjsonx) */
    API_ERR_ARGS,                       /* args 错误(来自 cjsonx) */
    API_ERR_FIELD_OVERFLOW,             /* 字段的值溢出(来自 cjsonx) */
    API_ERR_TOO_MANY_FIELDS,            /* 字段太多 */
    API_ERR_SYS_REBOOT_OR_APP_REBOOT,   /* sys_reboot 字段或者 app_reboot 字段错误 */
    API_ERR_BUSY,                       /* 升级任务忙碌 */
    API_ERR_UPGRADE_CREATE,             /* 升级任务创建失败 */
    API_ERR_CHARGING,                   /* 充电中 */
    API_ERR_APP_REBOOT,                 /* 成功并且 app 重启 */
    API_ERR_SYS_REBOOT,                 /* 成功并且系统重启 */
    API_ERR_NOT_DEBUG_MODE,             /* 当前不是调试模式 */
    API_ERR_NOT_FAULT_INJECTION_MODE,   /* 当前不是故障注入模式 */
    API_ERR_DEVICE_OFFLINE,             /* 设备不在线 */
    API_ERR_EXPORTING,                  /* 其他任务导出中 */
    API_ERR_NOT_RELEASE_MODE,           /* 当前不是出厂模式 */

    /* 以下是字段错误 */
    API_ERR_FIELD_ROLES,
    API_ERR_FIELD_USERNAME,
    API_ERR_FIELD_PASSWORD,
    API_ERR_FIELD_ID,
    API_ERR_FIELD_LEVEL,
    API_ERR_FIELD_CCU_ID,
    API_ERR_FIELD_CONN_ID,
    API_ERR_FIELD_CODE,
    API_ERR_FIELD_NAME,
    API_ERR_FIELD_DCDC_ID,
    API_ERR_FIELD_IP_ADDR,
    API_ERR_FIELD_PDU_ID,
    API_ERR_FIELD_RELAY_ID,
    API_ERR_FIELD_RELAY_CONTROL,
    API_ERR_FIELD_HANDSHAKE_RELAY_ID,
    API_ERR_FIELD_DEVICE_INFO_TYPE,
    API_ERR_FIELD_TRANSACTION_ID,
    API_ERR_FIELD_DEVICE_ID,
    API_ERR_FIELD_FAULT,
    API_ERR_FIELD_DEVICE_TYPE,
    API_ERR_FIELD_TYPE,
    API_ERR_FIELD_TASK_ID,
    API_ERR_FIELD_FILE_PATH,
    API_ERR_FIELD_DATE,
    API_ERR_END,
} api_code_e;
typedef struct FCGX_Request FCGX_Request;
extern void api_print(FCGX_Request *req, api_code_e code, const char* data);

api_code_e api_code_from_cjsonx(int32_t err);
#ifdef __cplusplus
}
#endif

#endif /* __API_CODE_H__ */
