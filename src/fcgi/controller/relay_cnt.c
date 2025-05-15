#include "relay_cnt.h"
#include "database/model/pcu_relay_cnt_model.h"
#include "cjsonx/cJSONx.h"
#include "relay/relay.h"
#include "utils/utils.h"
#include "database/dao/pcu_relay_cnt/pcu_relay_cnt.h"
#include "database/init.h"

#include <string.h>

typedef struct
{
    pcu_relay_cnt_t list[6]; // 指向数组的指针
    size_t list_len;            // 数组大小
} pcu_relay_cnt_array_t;

static const cjsonx_reflect_t get_relay_cnt_reflection[] =
{
    __cjsonx_int(pcu_relay_cnt_t, id),
    __cjsonx_int(pcu_relay_cnt_t, relay_id),
    __cjsonx_int(pcu_relay_cnt_t, close_cnt),
    __cjsonx_end()
};


static const cjsonx_reflect_t get_relay_cnt_array_reflection[] =
{
    __cjsonx_array_object(pcu_relay_cnt_array_t, list, list_len, get_relay_cnt_reflection), // 处理数组字段
    __cjsonx_end()
};

void query_relay_cnt(FCGX_Request *req)
{
    pcu_relay_cnt_t *relay_arr = get_relay_cnt();
    pcu_relay_cnt_array_t response;
    memset(&response, 0, sizeof(response));
    for (uint8_t i = 0; i < 6; i++)
    {
        response.list[i] = relay_arr[i];
    }
    response.list_len = 6;
    char json_str[2048] = {0};
    int32_t ret = cjsonx_struct2str_preallocated(json_str, sizeof(json_str), &response, get_relay_cnt_array_reflection);
    if (ERR_CJSONX_NONE != ret)
    {
        api_print(req, api_code_from_cjsonx(ret), NULL);
        return;
    }
    api_print(req, API_ERR_OK, json_str);
}

void set_relay_cnt(FCGX_Request *req)
{
    pcu_relay_cnt_t *relay_arr = get_relay_cnt();
    pcu_relay_cnt_array_t response;
    memset(&response, 0, sizeof(response));

    char json_str[2048];
    int len = FCGX_GetStr(json_str, sizeof(json_str) - 1, req->in);
    if (len < 0) {
        api_print(req, API_ERR_JSON_TYPE, "Failed to read input");
        return;
    }
    json_str[len] = '\0'; // 确保字符串结束
    d_log("json:%s", json_str);
    // 使用 cJSON 解析 JSON 字符串
    cJSON *root = cJSON_Parse(json_str);
    if (!root) {
        api_print(req, API_ERR_JSON_TYPE, "Failed to parse JSON");
        return;
    }

    // 获取 data 对象
    cJSON *data_obj = cJSON_GetObjectItemCaseSensitive(root, "data");
    if (!data_obj) {
        cJSON_Delete(root);
        api_print(req, API_ERR_JSON_TYPE, "Missing 'data' field in JSON");
        return;
    }

    // 获取 list 数组
    cJSON *list_array = cJSON_GetObjectItemCaseSensitive(data_obj, "list");
    if (!cJSON_IsArray(list_array)) {
        cJSON_Delete(root);
        api_print(req, API_ERR_JSON_TYPE, "Invalid 'list' field in JSON");
        return;
    }

    // 假设数组长度为6
    size_t array_size = cJSON_GetArraySize(list_array);
    if (array_size != 6)
    {
        cJSON_Delete(root);
        api_print(req, API_ERR_JSON_TYPE, "Relay count must be exactly 6 items");
        return;
    }

    // 将 JSON 数组解析到 response.list 中
    for (uint8_t i = 0; i < 6; i++) {
        cJSON *item = cJSON_GetArrayItem(list_array, i);
        response.list[i].id = cJSON_GetObjectItem(item, "id")->valueint;
        response.list[i].relay_id = cJSON_GetObjectItem(item, "relay_id")->valueint;
        response.list[i].close_cnt = cJSON_GetObjectItem(item, "close_cnt")->valueint;
    }

    cJSON_Delete(root); // 清理 cJSON 对象

    dao_t *dao = get_dao_prt();
    for (uint8_t i = 0; i < 6; i++)
    {
        relay_arr[i] = response.list[i];
        d_log("relay_arr[%d]:cnt:%d    response[%d].close_cnt:%d", i, relay_arr[i].close_cnt, i, response.list[i].close_cnt);
        pcu_relay_cnt_dao_update_by_relay_id(&dao->pcu_relay_cnt_dao, relay_arr[i].relay_id, &relay_arr[i]);
    }

    // 返回 OK 响应
    api_print(req, API_ERR_OK, "{\"status\": \"success\"}");
}