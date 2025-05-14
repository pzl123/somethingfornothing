#ifndef __PCU_RELAY_CNT_MODEL_H__
#define __PCU_RELAY_CNT_MODEL_H__

#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum
{
    DO_DC_INPUT1_POS = 427, /* DC直流输入1，正级主接触器合分 */
    DO_DC_INPUT1_NEG = 428, /* DC直流输入1，负级主接触器合分 */
    DO_DC_INPUT1_PRE = 411, /* DC直流输入1，预充接触器合分 */
    DO_DC_INPUT2_POS = 455, /* DC直流输入2，正级主接触器合分 */
    DO_DC_INPUT2_NEG = 454, /* DC直流输入2，负级主接触器合分 */
    DO_DC_INPUT2_PRE = 412, /* DC直流输入2，预充接触器合分 */
} pcu_do_branch_relay_e;

typedef struct
{
    int32_t id;
    pcu_do_branch_relay_e relay_id;
    int32_t close_cnt;
} pcu_relay_cnt_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif