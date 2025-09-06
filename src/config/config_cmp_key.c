#include "config_cmp_key.h"
#include "utils/utils.h"

#include "cjsonx/cJSON.h"

#include"config_manage.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define PCU_CONFING_NAME "pcu"                /* pcu 配置名 */
static bool pcu_config_init(void)
{
    cJSON *config = cJSON_CreateObject();
    (void)cJSON_AddNumberToObject(config, "pcu_id", 1);
    (void)cJSON_AddNumberToObject(config, "branch_system", 2);
    (void)cJSON_AddNumberToObject(config, "ccu_num", 4);
    (void)cJSON_AddNumberToObject(config, "dc_dc_model", 0x01);
    (void)cJSON_AddNumberToObject(config, "meter_model", 0x01);
    (void)cJSON_AddNumberToObject(config, "device_control_mode", 0x01);
    (void)cJSON_AddNumberToObject(config, "dcdc_alarm_mode", 0x01);
    (void)cJSON_AddNumberToObject(config, "pdu_alarm_mode", 0x01);
    (void)cJSON_AddNumberToObject(config, "meter_alarm_mode", 0x01);
    (void)cJSON_AddNumberToObject(config, "high_vol_input_mode", 0x01);
    (void)cJSON_AddBoolToObject(config, "spd_enable", true);
    (void)cJSON_AddBoolToObject(config, "door_access_enable", true);
    (void)cJSON_AddBoolToObject(config, "waterlogging_enable", true);
    (void)cJSON_AddBoolToObject(config, "topple_enable", true);
    (void)cJSON_AddNumberToObject(config, "fan_start_temp", 40);
    (void)cJSON_AddNumberToObject(config, "work_altitude", 1000);

    if (!set_default_config(PCU_CONFING_NAME, config))
    {
        e_log("%s set_default_config failed", PCU_CONFING_NAME);

        cJSON_Delete(config);
        return false;
    }
    cJSON_Delete(config);

    return true;
}

void traverse_all_keys_recursive(const cJSON *object, const char *parent_key)
{
    if (!object || !cJSON_IsObject(object)) {
        return;
    }

    cJSON *child = NULL;
    cJSON_ArrayForEach(child, object) {
        char current_key[256] = {0};

        if (parent_key && strlen(parent_key) > 0) {
            snprintf(current_key, sizeof(current_key), "%s.%s", parent_key, child->string);
        } else {
            snprintf(current_key, sizeof(current_key), "%s", child->string);
        }

        d_log("Found key: %s\n", current_key);

        if (cJSON_IsObject(child))
        {
            traverse_all_keys_recursive(child, current_key);
        }

        else if (cJSON_IsArray(child))
        {
            cJSON *item = NULL;
            int idx = 0;
            cJSON_ArrayForEach(item, child)
            {
                if (cJSON_IsObject(item))
                {
                    char array_key[1256];
                    snprintf(array_key, sizeof(array_key), "%s[%d]", current_key, idx);
                    traverse_all_keys_recursive(item, array_key);
                }
                idx++;
            }
        }
    }
}


/* static */ void print_cjson_object(const cJSON *object)
{
    char *str = cJSON_Print(object);
    d_log("%s", str);
    free(str);
}

static int cf_case_insensitive_strcmp(const unsigned char *string1, const unsigned char *string2)
{
    if ((string1 == NULL) || (string2 == NULL))
    {
        return 1;
    }

    if (string1 == string2)
    {
        return 0;
    }

    for(; tolower(*string1) == tolower(*string2); (void)string1++, string2++)
    {
        if (*string1 == '\0')
        {
            return 0;
        }
    }

    return tolower(*string1) - tolower(*string2);
}

static cJSON *cf_get_object_item(const cJSON * const object, const char * const name, const cJSON_bool case_sensitive)
{
    cJSON *current_element = NULL;

    if ((object == NULL) || (name == NULL))
    {
        return NULL;
    }

    current_element = object->child;
    if (case_sensitive)
    {
        while ((current_element != NULL) && (current_element->string != NULL) && (strcmp(name, current_element->string) != 0))
        {
            current_element = current_element->next;
        }
    }
    else
    {
        while ((current_element != NULL) && (cf_case_insensitive_strcmp((const unsigned char*)name, (const unsigned char*)(current_element->string)) != 0))
        {
            current_element = current_element->next;
        }
    }

    if ((current_element == NULL) || (current_element->string == NULL)) {
        return NULL;
    }

    return current_element;
}

static bool MergeNode(const cJSON *a, const cJSON *b, cJSON *a1, cJSON_bool case_sensitive)
{
    if ((a == NULL) || (b == NULL) || (NULL == a1))
    {
        return false;
    }

    switch (a->type & 0xFF)
    {
        case cJSON_False:
        case cJSON_True:
        case cJSON_NULL:
            a1->type = b->type & 0xFF;
            break;

        case cJSON_Number:
            cJSON_SetNumberValue(a1, b->valuedouble);
            a1->valueint = b->valueint;
            a1->type = cJSON_Number;
            break;

        case cJSON_String:
        case cJSON_Raw:
            if (!cJSON_SetValuestring(a1, b->valuestring)) {
                return false;
            }
            a1->type = b->type & 0xFF;
            break;
        case cJSON_Object:
        {
            cJSON *a_child = NULL;
            cJSON *b_child = NULL;

            cJSON_ArrayForEach(a_child, a)
            {
                if (a_child->string == NULL)
                {
                    continue;
                }
                b_child = cJSON_GetObjectItemCaseSensitive(b, a_child->string);

                if (b_child != NULL)
                {
                    cJSON *b_copy = cJSON_Duplicate(b_child, true);
                    if (NULL == b_copy)
                    {
                        e_log("error duplicate b");
                        return false;
                    }
                    else
                    {
                        cJSON *a1_existing = cJSON_GetObjectItemCaseSensitive(a1, a_child->string);
                        cJSON_ReplaceItemViaPointer(a1, a1_existing, b_copy);
                    }
                }

                cJSON *a1_child = cJSON_GetObjectItemCaseSensitive(a1, a_child->string);
                cJSON *b1_child = cJSON_GetObjectItemCaseSensitive(b, a_child->string);
                if (a1_child != NULL && b1_child != NULL)
                {
                    MergeNode(a_child, b1_child, a1_child, case_sensitive);
                }
            }
            break;
        }

        case cJSON_Array:
        {
            cJSON *a_child = a->child;
            cJSON *b_child = b->child;
            cJSON *a1_child = a1->child;

            while (a_child && b_child && a1_child)
            {
                MergeNode(a_child, b_child, a1_child, case_sensitive);
                a_child = a_child->next;
                b_child = b_child->next;
                a1_child = a1_child->next;
            }
            break;
        }

        default:
            break;
    }
    return true;
}

static bool is_valid_cjson_type(int type)
{
    type &= 0xFF;
    switch (type)
    {
        case cJSON_False:
        case cJSON_True:
        case cJSON_NULL:
        case cJSON_Number:
        case cJSON_String:
        case cJSON_Raw:
        case cJSON_Array:
        case cJSON_Object:
            return true;
        default:
            return false;
    }
}

CJSON_PUBLIC(cJSON*) cJSON_MergeWithTemplate(const cJSON *a, const cJSON *b, cJSON_bool case_sensitive)
{
    if ((NULL == a) || (NULL == b))
    {
        e_log("a or b is NULL");
        return NULL;
    }

    if ((false == is_valid_cjson_type(a->type)) || (false == is_valid_cjson_type(b->type)))
    {
        e_log("error type: %d, %d", a->type, b->type);
        return NULL;
    }

    cJSON *a1 = cJSON_Duplicate(a, true);
    if (NULL == a1)
    {
        e_log("error cJSON_Duplicate a");
        return NULL;
    }

    MergeNode(a, b, a1, case_sensitive);
    return a1;
}

void test_config_cmp_key(void)
{
    config_manage_init();
    pcu_config_init();

    cJSON *default_pcu_json = get_default_config(PCU_CONFING_NAME);
    d_log("original default_pcu_json");
    print_cjson_object(default_pcu_json);
    cJSON *pcu_json = get_config(PCU_CONFING_NAME);
    d_log("original pcu_json");
    print_cjson_object(pcu_json);

    (void)cJSON_AddNumberToObject(default_pcu_json, "xxxx_id", 1);
    cJSON *new_pcu_json = cJSON_MergeWithTemplate(default_pcu_json, pcu_json, true);
    replace_object(PCU_CONFING_NAME, new_pcu_json, CFG_CURRENT);
    cJSON *updated_pcu_json = get_config(PCU_CONFING_NAME);
    d_log("pcu_json after default_pcu_json add xxxx_id");
    print_cjson_object(updated_pcu_json);

    cJSON_DeleteItemFromObjectCaseSensitive(default_pcu_json, "work_altitude");
    cJSON *new_pcu_json1 = cJSON_MergeWithTemplate(default_pcu_json, pcu_json, true);
    replace_object(PCU_CONFING_NAME, new_pcu_json1, CFG_CURRENT);
    cJSON *updated_pcu_json1 = get_config(PCU_CONFING_NAME);
    d_log("pcu_json after default_pcu_json delete work_altitude");
    print_cjson_object(updated_pcu_json1);

    cJSON *json1 = cJSON_CreateNumber(10);
    cJSON *json2 = cJSON_CreateNumber(20);
    cJSON *json3 = cJSON_CreateFalse();
    cJSON *json4 = cJSON_CreateTrue();
    cJSON *json7 = cJSON_CreateString("aaaaa");
    cJSON *json8 = cJSON_CreateString("bbbbb");
    cJSON *json5 = cJSON_MergeWithTemplate(json1, json2, true);
    cJSON *json6 = cJSON_MergeWithTemplate(json3, json4, true);
    cJSON *json9 = cJSON_MergeWithTemplate(json7, json8, true);
    d_log("template:%s, value:%s, target:%s", cJSON_Print(json1), cJSON_Print(json2), cJSON_Print(json5));
    d_log("template:%s, value:%s, target:%s", cJSON_Print(json3), cJSON_Print(json4), cJSON_Print(json6));
    d_log("template:%s, value:%s, target:%s", cJSON_Print(json7), cJSON_Print(json8), cJSON_Print(json9));
}