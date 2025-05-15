#include "api_code.h"
#include "fcgi/fcgiapp.h"
#include "utils/utils.h"

#include <stdio.h>
#include <stdint.h>

static const char *unknown = "unknown error code";

typedef struct
{
    int32_t code;
    const char *message;
}api_result_t;
static const api_result_t map[] = {
    [API_ERR_OK]                            = {200,       "success"},
    [API_ERR_POINTER]                       = {500,       "pointer error"},
    [API_ERR_DB]                            = {500,       "database error"},
    [API_ERR_INTERNAL]                      = {500,       "internal error"},
    [API_ERR_METHOD]                        = {401,       "method error"},
    [API_ERR_URI]                           = {401,       "uri error"},
    [API_ERR_JSON]                          = {401,       "json format error"},
    [API_ERR_NULL_DATA]                     = {401,       "null data"},
    [API_ERR_CONTENT_TYPE]                  = {401,       "content_type error"},
    [API_ERR_CONTENT_LENGTH]                = {401,       "content_length error"},
    [API_ERR_BOUNDARY]                      = {401,       "boundary error"},
    [API_ERR_FILE_NAME]                     = {401,       "file_name error"},
    [API_ERR_TOKEN_EXPIRED]                 = {40001,     "token expired"},
    [API_ERR_TOKEN_INVALID]                 = {40002,     "token invalid"},
    [API_ERR_PERMISSION]                    = {40003,     "permission denied"},
    [API_ERR_JSON_TYPE]                     = {401,       "json type error"},
    [API_ERR_MISSING_FIELD]                 = {401,       "missing field"},
    [API_ERR_ARGS]                          = {401,       "args error"},
    [API_ERR_FIELD_OVERFLOW]                = {401,       "value overflow"},
    [API_ERR_TOO_MANY_FIELDS]               = {401,       "too many fields"},
    [API_ERR_SYS_REBOOT_OR_APP_REBOOT]      = {401,       "sys_reboot or app_reboot error"},
    [API_ERR_BUSY]                          = {401,       "upgrade task is busy"},
    [API_ERR_UPGRADE_CREATE]                = {401,       "failed create upgrade task"},
    [API_ERR_CHARGING]                      = {401,       "charging"},
    [API_ERR_APP_REBOOT]                    = {200,       "success and app reboot"},
    [API_ERR_SYS_REBOOT]                    = {200,       "success and system reboot"},
    [API_ERR_NOT_DEBUG_MODE]                = {401,       "not currently in debug mode"},
    [API_ERR_NOT_FAULT_INJECTION_MODE]      = {401,       "not currently in fault injection mode"},
    [API_ERR_DEVICE_OFFLINE]                = {401,       "device offline"},
    [API_ERR_EXPORTING]                     = {401,       "exporting other tasks"},
    [API_ERR_NOT_RELEASE_MODE]              = {401,       "not currently in release mode"},

    [API_ERR_FIELD_ROLES]                   = {401,       "field roles error"},
    [API_ERR_FIELD_USERNAME]                = {401,       "field username error"},
    [API_ERR_FIELD_PASSWORD]                = {401,       "field password error"},
    [API_ERR_FIELD_ID]                      = {401,       "field id error"},
    [API_ERR_FIELD_LEVEL]                   = {401,       "field level error"},
    [API_ERR_FIELD_CCU_ID]                  = {401,       "field ccu_id error"},
    [API_ERR_FIELD_CONN_ID]                 = {401,       "field conn_id error"},
    [API_ERR_FIELD_CODE]                    = {401,       "field code error"},
    [API_ERR_FIELD_NAME]                    = {401,       "field name error"},
    [API_ERR_FIELD_DCDC_ID]                 = {401,       "field dcdc_id error"},
    [API_ERR_FIELD_IP_ADDR]                 = {401,       "field ip_addr error"},
    [API_ERR_FIELD_PDU_ID]                  = {401,       "field pdu_id error"},
    [API_ERR_FIELD_RELAY_ID]                = {401,       "field relay_id error"},
    [API_ERR_FIELD_RELAY_CONTROL]           = {401,       "field relay_control error"},
    [API_ERR_FIELD_HANDSHAKE_RELAY_ID]      = {401,       "field handshake_relay_id error"},
    [API_ERR_FIELD_DEVICE_INFO_TYPE]        = {401,       "field device_info_type error"},
    [API_ERR_FIELD_TRANSACTION_ID]          = {401,       "field transaction_id error"},
    [API_ERR_FIELD_DEVICE_ID]               = {401,       "field device_id error"},
    [API_ERR_FIELD_FAULT]                   = {401,       "field fault error"},
    [API_ERR_FIELD_DEVICE_TYPE]             = {401,       "field device_type error"},
    [API_ERR_FIELD_TYPE]                    = {401,       "field type error"},
    [API_ERR_FIELD_TASK_ID]                 = {401,       "field task_id error"},
    [API_ERR_FIELD_FILE_PATH]               = {401,       "field file_path error"},
    [API_ERR_FIELD_DATE]                    = {401,       "field date error"},
};

void api_print(FCGX_Request *req, api_code_e code, const char* data)
{
    if (NULL == req)
    {
        e_log("null pointer");
        return;
    }

    FCGX_FPrintF(req->out, "Content-Type: application/json\r\n\r\n");
    if ((code < API_ERR_OK)
    || (code >= API_ERR_END))
    {
        FCGX_FPrintF(req->out, "{ \"code\": %d, \"message\": \"%s\", \"data\": {} }\n", code, unknown);
        return;
    }

    if(data == NULL)
    {
        FCGX_FPrintF(req->out, "{ \"code\": %d, \"message\": \"%s\", \"data\": {} }\n", map[code].code, map[code].message);
    }
    else
    {
        FCGX_FPrintF(req->out, "{ \"code\": %d, \"message\": \"%s\", \"data\": %s }\n", map[code].code, map[code].message, data);
    }
    return;
}
