#include <iostream>
#include <memory>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <map>
#include <queue>

#include "utils/cache/lru.h"
#include "database/init.h"
#include "database/dao/pcu_relay_cnt/pcu_relay_cnt.h"
#include "fcgi/fcgi.h"
#include "relay/relay.h"
#include "utils/priority_queue/priority_queue.h"
#include "can/can.h"
#include "peventloop/monotonic.h"
#include "utils/timer.h"
#include "ocpp/websock_client.h"

#include "mode/singleton.h"
#include "mode/composite.h"
#include "mode/template.h"

#include "tcp/ccu.h"
#include "config/config_cmp_key.h"

int main(void)
{
    // test_config_cmp_key();
    websocket_test();
    return 0;
}