#include "relay_cnt.h"

void query_relay_cnt(RelayData_t *relay_data, FCGX_Request *req)
{
    FCGX_FPrintF(req->out,
        "Content-Type: application/json\r\n"
        "\r\n");

    // 输出 JSON 格式的表格数据
    FCGX_FPrintF(req->out, "{\"data\":[");
    for (int i = 0; i < 6; i++) {
        if (i > 0) FCGX_FPrintF(req->out, ",");
        FCGX_FPrintF(req->out,"{\"name\":\"%s\",\"id\":%d,\"relay_id\":\"%s\",\"action_cnt\":%d}",
            relay_data[i].name, relay_data[i].id, relay_data[i].relay_id, relay_data[i].action_cnt);
    }
    FCGX_FPrintF(req->out, "]}");

    FCGX_Finish_r(req);
}