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
#include "ocpp/config/ConfigManager.h"

#include "mode/singleton.h"
#include "mode/composite.h"
#include "mode/template.h"

#include "tcp/ccu.h"
#include "config/config_cmp_key.h"

#include "utils/file_utils.h"

int main(void)
{
    // core_dump_file(true);
    web_socket_client_test();
    // can_test();

    return 0;
}