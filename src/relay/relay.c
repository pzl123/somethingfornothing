#include "relay.h"


pcu_relay_cnt_t g_relay_cnt[6] = {
    {1, DO_DC_INPUT1_POS, 20},
    {2, DO_DC_INPUT1_NEG, 21},
    {3, DO_DC_INPUT1_PRE, 22},
    {4, DO_DC_INPUT2_POS, 23},
    {5, DO_DC_INPUT2_NEG, 24},
    {6, DO_DC_INPUT2_PRE, 25}
};

pcu_relay_cnt_t* get_relay_cnt(void)
{
    return g_relay_cnt;
}