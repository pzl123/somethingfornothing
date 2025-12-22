#include <iostream>
#include <memory>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <map>
#include <queue>
#include <unistd.h>

#include "utils/cache/lru.h"
#include "database/init.h"
#include "database/dao/pcu_relay_cnt/pcu_relay_cnt.h"
#include "fcgi/fcgi.h"
#include "relay/relay.h"
#include "utils/priority_queue/priority_queue.h"
#include "can/can.h"
#include "peventloop/monotonic.h"
#include "utils/timer.h"
#include "ocpp/client/websocketclient/websock_client.h"
#include "ocpp/config/configManager.h"

#include "mode/singleton.h"
#include "mode/composite.h"
#include "mode/template.h"

#include "tcp/ccu.h"
#include "config/config_cmp_key.h"

#include "utils/file_utils.h"

static void my_log_emit(int level, const char *line)
{
    if (level == LLL_ERR)
    {
        e_log("LWS ERROR: %s", line);
    }
    else if (level == LLL_WARN)
    {
        w_log("LWS WARN: %s", line);
    }
    else if (level == LLL_NOTICE)
    {
        i_log("LWS NOTIC: %s", line);
    }
    else
    {
        d_log("LWS INFO OR DEBUG: %s", line);
    }
}

int main(void)
{
    core_dump_file(true);
    lws_set_log_level(LLL_ERR | LLL_WARN | LLL_NOTICE, my_log_emit); /* 设置websocket全局日志信息 */
    web_socket_client_test();
    return 0;
}