#include "dao_option.h"

UT_array* new_dao_option_array(void)
{
    UT_array *array = NULL;
    static UT_icd icd = {sizeof(dao_option_t), NULL, NULL, NULL};
    utarray_new(array, &icd);
    return array;
}