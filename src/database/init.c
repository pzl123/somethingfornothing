#include "init.h"
#include "dal.h"

static dal_t dal = {0};

dao_t* dao_init(void)
{
    (void)new_dal(&dal);
    return &dal.dao;
}

void dao_clear(void)
{
    delete_dal(&dal);
    return;
}

dao_t *get_dao_prt(void)
{
    return &dal.dao;
}
