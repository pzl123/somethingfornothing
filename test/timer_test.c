#include "utils/timer.h"
#include "utils/utils.h"

#include <string.h>
#include <unistd.h>


static void my_callback(void *userdata)
{
    d_log("Timer1 fired! Data: %s", (char*)userdata);
}

static void my_callback1(void *userdata)
{
    d_log("Timer2 fired! Data: %s", (char*)userdata);
}


static void test_timer(void)
{
    int quit = 0;
    p_timer_init(&quit);
    char *data = strdup("Hello Timer");
    Timer_handle_t t1 = p_timer_add(1000, 3, &my_callback, data);
    Timer_handle_t t2 = p_timer_add(1000, 3, &my_callback1, data);
    // // 主线程做其他工作
    while (1)
    {
        sleep(5);
    }
    quit = 1;
    p_timer_del(&t1);
    p_timer_del(&t2);
}
