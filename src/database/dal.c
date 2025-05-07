#include "dal.h"
#include "database/dao/dao.h"

#include <stdbool.h>
#include <stdlib.h>

void init_dal(dal_t *dal)
{
    init_dao(&dal->dao);
}