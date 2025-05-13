#ifndef __PCURELAY_CNT_H__
#define __PCURELAY_CNT_H__


#include <stdbool.h>

#include "database/dao/prepared_stmt.h"
#include "database/model/pcu_relay_cnt_model.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct
{
    // retention_policy_t rp;
    prepared_stmt_t *ps;
} pcu_relay_cnt_dao_t;

bool new_pcu_relay_cnt_dao(pcu_relay_cnt_dao_t *dao, prepared_stmt_t *ps);

void delete_pcu_relay_cnt_dao(pcu_relay_cnt_dao_t *dao);

/**
 * @brief 添加一条 pcu 主回路继电器动作次数
 * @param dao
 * @param p 数据源
 * @return 变更的行数
 */
int32_t pcu_relay_cnt_dao_create(pcu_relay_cnt_dao_t *dao, const pcu_relay_cnt_t *p);

/**
 * @brief 获取 pcu 主回路继电器动作次数(通过继电器 id)
 * @param dao
 * @param relay_id 继电器 id
 * @param res 返回结果
 * @return 查询到的行数
 */
int32_t pcu_relay_cnt_dao_get_by_relay_id(pcu_relay_cnt_dao_t *dao, pcu_do_branch_relay_e relay_id, pcu_relay_cnt_t *res);

/**
 * @brief 更新 pcu 主回路继电器动作次数(通过继电器 id)
 * @param dao
 * @param relay_id 继电器 id
 * @param p 数据源
 * @return 变更的行数
 */
int32_t pcu_relay_cnt_dao_update_by_relay_id(pcu_relay_cnt_dao_t *dao, pcu_do_branch_relay_e relay_id, const pcu_relay_cnt_t *p);


int32_t pcu_relay_cnt_dao_delete_by_relay_id(pcu_relay_cnt_dao_t *dao, pcu_do_branch_relay_e relay_id);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __PCURELAY_CNT_H__ */
