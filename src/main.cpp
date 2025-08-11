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

#include "mode/singleton.h"
#include "mode/composite.h"
#include "mode/template.h"

#include "tcp/ccu.h"

int main(void)
{
    pthread_t tid;
    (void)pthread_create(&tid, NULL, &ccu_server_start_internal, NULL);
    (void)pthread_detach(tid);
    while (1)
    {
        sleep(1);
    }
    return 0;
}
