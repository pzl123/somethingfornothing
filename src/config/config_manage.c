#include "config_manage.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdint.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>

#include "hv/hbase.h"
#include "utils/utils.h"


typedef struct cb_list
{
    on_config_change cb;
    struct cb_list *prev;
    struct cb_list *next;
} cb_list_t;



typedef struct
{
    char *path;   /* 配置文件路径 */
    char *name;   /* 配置表名 */
    cJSON *value; /* 配置 */
    pthread_rwlock_t value_lock;

    cb_list_t *owners;
    pthread_mutex_t owners_lock;

    bool value_lock_initialized;
    bool owners_lock_initialized;

    UT_hash_handle hh;
} config_manage_t;

static config_manage_t *g_config_manage = NULL;
static config_manage_t *g_default_config_manage = NULL;

static int32_t config_hash_init(char *path, config_manage_t **table);
static bool config_item_free(config_manage_t *item);

static bool read_file_to_memory(const char *target_file, cJSON **cjson_config);
static bool write_memory_to_file(const cJSON *cjson_config, const char *path);

static bool add_config_item(config_manage_t **table, const char *config_name, cJSON *cjson_config, bool path_flag);
static config_manage_t *find_config_item(config_manage_t *dorc, const char *config_name);

static bool notify_owner(const char *config, cJSON *old_value, cJSON *new_value);
static cb_list_t *find_owner(config_manage_t *s, on_config_change cb);
static void delete_all_owners(config_manage_t *s);


static char *name_add_suffix(const char *name);
static char *name_add_prefix(const char *config_name);

#if 0
static void print_format(config_manage_t * table)
{
    if (NULL == table)
    {
        e_log("table is NULL in %s", __FUNCTION__);
        return;
    }

    char *json_str = NULL;

    config_manage_t *current_user;
    config_manage_t *tmp;
    HASH_ITER(hh, table, current_user, tmp)
    {
        json_str = cJSON_Print(current_user->value);
        d_log(
            "\n"
            ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n"
            "path: %s\n"
            "config_name: %s\n"
            "value:\n%s\n"
            "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n"
            , current_user->path, current_user->name, json_str);
        free(json_str);
    }
}
#endif

static bool config_item_free(config_manage_t *item)
{
    if (NULL == item)
    {
        e_log("config item is NULL in %s", __FUNCTION__);
        return false;
    }

    if (NULL != item->path)
    {
        free(item->path);
        item->path = NULL;
    }

    if (NULL != item->name)
    {
        free(item->name);
        item->name = NULL;
    }

    if (NULL != item->value)
    {
        cJSON_Delete(item->value);
        item->value = NULL;
    }

    if (NULL != item->owners)
    {
        delete_all_owners(item);
    }

    if (item->value_lock_initialized)
    {
        (void)pthread_rwlock_destroy(&(item->value_lock));
        item->value_lock_initialized = false;
    }

    if (item->owners_lock_initialized)
    {
        (void)pthread_mutex_destroy(&(item->owners_lock));
        item->owners_lock_initialized = false;
    }

    free(item);
    return true;
}

static config_manage_t *new_config_item(const char *name, const cJSON *config, config_type_e where_e)
{
    config_manage_t *s = (config_manage_t *)malloc(sizeof(config_manage_t));
    if (NULL == s)
    {
        e_log("s malloc error");
        return NULL;
    }
    (void)memset(s, 0, sizeof(config_manage_t));

    s->name =strdup(name);
    if (NULL == s->name)
    {
        w_log("s malloc error");
        config_item_free(s);
        return NULL;
    }

    s->value = cJSON_Duplicate(config, 1);
    if (NULL == s->value)
    {
        w_log("s malloc error");
        config_item_free(s);
        return NULL;
    }

    int32_t len = (where_e == CFG_DEFAULT ? strlen(DEFAULT_CONFIG_PATH):strlen(CONFIG_PATH)) + strlen(s->name) + 2;
    s->path = (char *)malloc(len);
    if (NULL ==  s->path)
    {
        config_item_free(s);
        w_log("s->path malloc error");
        return NULL;
    }
    (void)snprintf(s->path, len, "%s/%s", (where_e == CFG_DEFAULT ? DEFAULT_CONFIG_PATH : CONFIG_PATH), s->name);

    s->owners=NULL;

    if (pthread_rwlock_init(&(s->value_lock), NULL) != 0)
    {
        e_log("Failed to init value_lock");
        config_item_free(s);
        return NULL;
    }
    s->value_lock_initialized = true;

    if (pthread_mutex_init(&(s->owners_lock), NULL) != 0)
    {
        e_log("Failed to init owners_lock");
        config_item_free(s);
        return NULL;
    }
    s->owners_lock_initialized = true;

    return s;
}

static char *name_add_suffix(const char *name)
{
    if (NULL == name)
    {
        e_log("%s error, because name is NULL", __FUNCTION__);
        return NULL;
    }

    int32_t config_len = strlen(name) + strlen(".json") + 1;
    char *config_name = (char *)malloc(config_len);
    if (NULL == config_name)
    {
        w_log("config name malloc error");
        return NULL;
    }
    (void)memset(config_name, 0, config_len);
    (void)snprintf(config_name, config_len, "%s.json", name);
    return config_name;
}

static char *name_add_prefix(const char *config_name)
{
    if (NULL == config_name)
    {
        e_log("%s error, because config_name is NULL", __FUNCTION__);
        return NULL;
    }

    int32_t default_len = strlen(config_name) + strlen("default_") + 1;
    char *default_config_name = (char *)malloc(default_len);
    if (NULL == default_config_name)
    {
        return NULL;
    }
    (void)memset(default_config_name, 0, default_len);
    (void)snprintf(default_config_name, default_len, "default_%s", config_name);
    return default_config_name;
}

bool set_config(const char *name, cJSON *config)
{
    if ((NULL == name) || (NULL == config))
    {
        e_log("set config error, because name or config is NULL in %s", __FUNCTION__);
        return false;
    }
    bool res = false;

    /*name.json*/
    char *config_name = name_add_suffix(name);

    /*default_name.json*/
    char *default_config_name = name_add_prefix(config_name);

    config_manage_t *s = find_config_item(g_config_manage, config_name);
    config_manage_t *default_s = find_config_item(g_default_config_manage, default_config_name);
    free(default_config_name);
    /*g_config_manage和g_default_config_manage中都不存在，无此配置*/
    if ((NULL == s) && (NULL == default_s))
    {
        e_log("no such config %s in path %s and %s",config_name, CONFIG_PATH, DEFAULT_CONFIG_PATH);
        free(config_name);
        return res;
    }
    /*g_default_config_manage中存在，g_config_manage中不存在，需要添加到g_config_manage中,并且在CONFIG_PATH中创建文件name.json*/
    else if ((NULL == s) && (NULL != default_s))
    {
        d_log("no such config %s in path %s, but in path %s", config_name, CONFIG_PATH, DEFAULT_CONFIG_PATH);

        s = new_config_item(config_name, config, CFG_CURRENT);
        if (NULL == s)
        {
            w_log("s malloc error");
            free(config_name);
            return res;
        }
        free(config_name);

        HASH_ADD_STR(g_config_manage, name, s);
        (void)write_memory_to_file(s->value, s->path);
        return !res;
    }
    /*config中存在，default中不存在, 不允许这种情况发生*/
    else if ((NULL != s) && (NULL == default_s))
    {
        e_log("such a situation is not allowed to exist.");
        free(config_name);
        return res;
    }
    /*表已存在，无需重新添加，只要修改*/
    else if ((NULL != s) && (NULL != default_s))
    {
        (void)pthread_rwlock_rdlock(&s->value_lock);
        bool same = cJSON_Compare(s->value, config, true);
        (void)pthread_rwlock_unlock(&s->value_lock);
        if(same)
        {
            free(config_name);
            i_log("same, no need to modify.");
            return !res;
        }
        else
        {
            d_log("config_name: %s %p, s->name:%s %p", config_name, &config_name, s->name, &s->name);
            if(notify_owner(config_name, s->value, config))
            {
                d_log("memory file %s changed", config_name);
            }
            else
            {
                i_log("notify falied, config %s changed", config_name);
            }
            (void)pthread_rwlock_wrlock(&s->value_lock);
            cJSON_Delete(s->value);
            s->value = cJSON_Duplicate(config, 1);
            (void)write_memory_to_file(s->value, s->path);
            (void)pthread_rwlock_unlock(&s->value_lock);
            free(config_name);
            return !res;
        }
    }
    else
    {
        /* 冗余代码, 永远不可达 */
        e_log("set config failed, need to check set config %s", config_name);
        free(config_name);
        return res;
    }
}

cJSON *get_config(const char *name)
{
    if (NULL == name)
    {
        e_log("name or config is NULL, get config failed in %s", __FUNCTION__);
        return NULL;
    }

    char *config_name = name_add_suffix(name);
    config_manage_t *s = find_config_item(g_config_manage, config_name);
    if (NULL == s)
    {
        e_log("no such default config %s in path %s", config_name, CONFIG_PATH);
        free(config_name);
        return NULL;
    }
    else
    {
        (void)pthread_rwlock_rdlock(&s->value_lock);
        cJSON *config = cJSON_Duplicate(s->value, 1);
        (void)pthread_rwlock_unlock(&s->value_lock);

        free(config_name);
        return config;
    }
}

bool replace_object(const char *name, cJSON *config, config_type_e config_type)
{
    if ((NULL == name) || (NULL == config))
    {
        e_log("api error");
        return false;
    }
    char *config_name = NULL;
    if (CFG_DEFAULT == config_type)
    {
        char *default_name = name_add_prefix(name);
        config_name= name_add_suffix(default_name);
        free(default_name);
    }
    else if (CFG_CURRENT == config_type)
    {
        config_name = name_add_suffix(name);
    }
    config_manage_t *s = find_config_item(CFG_DEFAULT == config_type ? g_default_config_manage : g_config_manage, config_name);
    if (NULL == s)
    {
        e_log("no such config %s in path %s", config_name, CFG_DEFAULT == config_type ? DEFAULT_CONFIG_PATH : CONFIG_PATH);
        cJSON_Delete(config);
        return false;
    }
    else
    {
        (void)pthread_rwlock_wrlock(&s->value_lock);
        cJSON *old_value = s->value;
        s->value = config;
        (void)write_memory_to_file(s->value, s->path);
        (void)pthread_rwlock_unlock(&s->value_lock);
        cJSON_Delete(old_value);
    }
    free(config_name);
    return true;
}

char *get_config_str(const char *name)
{
    cJSON *config = get_config(name);
    char *json2str = cJSON_PrintUnformatted(config);
    cJSON_Delete(config);
    return json2str;
}

bool set_default_config(const char *name, cJSON *config)
{
    if ((NULL == name) || (NULL == config))
    {
        e_log("name or config is NULL, set default config failed in %s", __FUNCTION__);
        return false;
    }
    bool res = false;
    char *default_config_name_tmp = name_add_prefix(name);
    char *default_config_name = name_add_suffix(default_config_name_tmp);
    free(default_config_name_tmp);

    config_manage_t *s = find_config_item(g_default_config_manage, default_config_name);
    /*default 不存在则创建*/
    if (NULL == s)
    {
        d_log("default config %s not exist, create it", default_config_name);
        s = new_config_item(default_config_name, config, CFG_DEFAULT);
        if (NULL == s)
        {
            w_log("s malloc error");
            free(default_config_name);
            return res;
        }
        free(default_config_name);

        HASH_ADD_STR(g_default_config_manage, name, s);
        (void)write_memory_to_file(s->value,s->path);
        (void)set_config(name, config);
        return !res;
    }
    else
    {
        free(default_config_name);
        (void)pthread_rwlock_rdlock(&s->value_lock);
        bool same = cJSON_Compare(s->value, config, true);
        (void)pthread_rwlock_unlock(&s->value_lock);
        if (same)
        {
            char *cur_name = name_add_suffix(name);
            config_manage_t *cur_s = find_config_item(g_config_manage, cur_name);
            if (NULL == cur_s)
            {
                i_log("%s not exist", cur_name);
                (void)set_config(name, config);
                i_log("%s created", cur_name);
            }
            free(cur_name);
            (void)pthread_rwlock_unlock(&s->value_lock);
            i_log("%s same default, no need to modify.", s->name);
            return !res;
        }
        else
        {
            (void)pthread_rwlock_wrlock(&s->value_lock);
            cJSON_Delete(s->value);
            s->value = cJSON_Duplicate(config, 1);
            (void)write_memory_to_file(s->value, s->path);
            (void)pthread_rwlock_unlock(&s->value_lock);
            i_log("%s configuration mismatch, modify current configuration", s->name);
            (void)set_config(name, config);
            return !res;
        }
    }
}



cJSON *get_default_config(const char *name)
{
    if (NULL == name)
    {
        e_log("name or config is NULL, get default config failed in %s", __FUNCTION__);
        return NULL;
    }
    char *default_name = name_add_prefix(name);
    char * config_name= name_add_suffix(default_name);
    free(default_name);
    config_manage_t *s = find_config_item(g_default_config_manage, config_name);
    if (NULL == s)
    {
        e_log("no such default config %s in path %s", config_name, DEFAULT_CONFIG_PATH);
        free(config_name);
        return NULL;
    }
    else
    {
        (void)pthread_rwlock_rdlock(&s->value_lock);
        cJSON *config = cJSON_Duplicate(s->value, 1);
        (void)pthread_rwlock_unlock(&s->value_lock);

        free(config_name);
        return config;
    }
}

char *get_default_config_str(const char *name)
{
    cJSON *config = get_default_config(name);
    char *json2str = cJSON_PrintUnformatted(config);
    cJSON_Delete(config);
    return json2str;
}

static bool add_config_item(config_manage_t **table, const char *config_name, cJSON *cjson_config, bool path_flag)
{
    if ((NULL == table) || (NULL == config_name) || (NULL == cjson_config))
    {
        e_log("table or config_name or cjson_config is NULL in %s", __FUNCTION__);
        return false;
    }

    config_manage_t *s = NULL;
    HASH_FIND_STR(*table, config_name, s);
    if (NULL == s)
    {
        s = new_config_item(config_name, cjson_config, path_flag?CFG_DEFAULT:CFG_CURRENT);
        if (NULL == s)
        {
            w_log("s malloc error");
            return false;
        }
        HASH_ADD_STR(*table, name, s);
        return true;
    }
    else
    {
        w_log("file already exits in hash table, update it");
        (void)pthread_rwlock_wrlock(&s->value_lock);
        cJSON_Delete(s->value);
        s->value = cJSON_Duplicate(cjson_config, 1);
        (void)pthread_rwlock_unlock(&s->value_lock);
        return true;
    }

}

bool config_manage_init(void)
{
    bool ret = true;
    (void)make_dir_recursive(DEFAULT_CONFIG_PATH);
    int32_t err = config_hash_init(DEFAULT_CONFIG_PATH, &g_default_config_manage);
    if (0 != err)
    {
        ret = false;
    }
    switch (err)
    {
    case -2:
        e_log("default Input error");
        return ret;
    case -1:
        e_log("default config initialize failed");
        return ret;
    case 0:
        i_log("default config initialize successed");
        break;
    default:
        e_log("default unknown error");
        return ret;
    }
    (void)make_dir_recursive(CONFIG_PATH);
    err = config_hash_init(CONFIG_PATH, &g_config_manage);
    if (0 != err)
    {
        ret = false;
    }
    switch (err)
    {
    case -2:
        e_log("Input error");
        return ret;
    case -1:
        e_log("config initialize failed");
        return ret;
    case 0:
        i_log("config initialize successed");
        return ret;
    default:
        e_log("unknown error");
        return ret;
    }

}

static int32_t config_hash_init(char *path, config_manage_t **table)
{
    if ((NULL == table) || (NULL == path))
    {
        e_log("table or path is NULL on %s", __FUNCTION__);
        return -2;
    }

    struct dirent *file  = NULL;
    int32_t files_found = 0;

    DIR *dp = opendir(path);
    if(NULL == dp)
    {
        e_log("failed to open directory %s", path);
        return -1;
    }

    while (NULL != (file = readdir(dp)))
    {
        if (strncmp(file->d_name, ".", 1) == 0)
        {
            continue;
        }
        int32_t len = strlen(path) + strlen(file->d_name) + 2;
        char *dest_path = (char *)malloc(len);
        if (NULL == dest_path)
        {
            w_log("dest_path malloc error");
            (void)closedir(dp);
            return -1;
        }
        (void)snprintf(dest_path, len, "%s/%s", path, file->d_name);

        bool flag = false;
        if(strstr(dest_path,"default"))
        {
            d_log("DEFAULT CONFIG");
            flag = true;
        }
        else
        {
            d_log("CONFIG");
            flag = false;
        }
        struct stat buf;
        (void)memset(&buf, 0, sizeof(buf));
        if (0 != stat(dest_path, &buf))
        {
            e_log("stat %s error", dest_path);
            free(dest_path);
            continue;
        }

        if (S_ISREG(buf.st_mode))
        {
            cJSON *cjson_config = NULL;
            if (read_file_to_memory(dest_path, &cjson_config))
            {
                if (!(add_config_item(table, file->d_name, cjson_config, flag)))
                {
                    e_log("failed to load JSON from %s", dest_path);
                    cJSON_Delete(cjson_config);
                    free(dest_path);
                    continue;
                }
                files_found++;
                cJSON_Delete(cjson_config);
            }
            else
            {
                e_log("failed to load or parse JSON from %s", dest_path);
                free(dest_path);
                continue;
            }
        }
        free(dest_path);
    }
    (void)closedir(dp);

    if (files_found == 0)
    {
        w_log("no files in the target directory %s", path);
    }

    return 0;
}

static bool read_file_to_memory(const char *target_file, cJSON **cjson_config)
{
    if ((NULL == target_file) || (NULL == cjson_config))
    {
        e_log("target_file or cjson_config is NULL in %s", __FUNCTION__);
        return false;
    }
    bool res = false;

    FILE *fp = fopen(target_file, "r");
    if (NULL == fp)
    {
        e_log("open file %s error", target_file);
        return res;
    }

    (void)fseek(fp, 0, SEEK_END);
    int32_t file_len = ftell(fp);
    (void)fseek(fp, 0, SEEK_SET);
    if (file_len <= 0)
    {
        (void)fclose(fp);
        w_log("%s file is NULL", target_file);
        return res;
    }

    char *readbuf = (char *)malloc(file_len + 1);
    if (NULL == readbuf)
    {
        w_log("readbuf malloc error");
        fclose(fp);
        return res;
    }

    size_t bytes_read = fread(readbuf, 1, file_len, fp);
    (void)fclose(fp);
    if ((int32_t)bytes_read != file_len)
    {
        e_log("unable to read full file %s", target_file);
        free(readbuf);
        return res;
    }

    readbuf[file_len] = '\0';

    *cjson_config = cJSON_Parse(readbuf);
    free(readbuf);
    return !res;
}

bool write_memory_to_file(const cJSON *cjson_config, const char *path)
{
    if ((NULL == cjson_config) || (NULL == path))
    {
        e_log("config_name or cjson_config or path is NULL in %s", __FUNCTION__);
        return false;
    }

    bool res = false;
    char *json_string = cJSON_Print(cjson_config);
    if (NULL != json_string)
    {
        FILE *target_file = fopen(path , "w");
        if (NULL == target_file)
        {
            e_log("failed to open the target file %s", path);
            free(json_string);
            return res;
        }
        (void)fwrite(json_string, sizeof(char), strlen(json_string), target_file);
        free(json_string);
        (void)fflush(target_file);
        int32_t fd = fileno(target_file);
        if (fd != -1)
        {
            if (fsync(fd) != 0)
            {
                e_log("fsync failed for %s", path);
                (void)fclose(target_file);
                return res;
            }
        }
        else
        {
            e_log("failed to get file %s descriptor", path);
            (void)fclose(target_file);
            return res;
        }
        (void)fclose(target_file);
        return !res;
    }
    else
    {
        e_log("failed to serialize %s cJSON object to string.", path);
        return res;
    }
}

config_manage_t *find_config_item(config_manage_t * table, const char *config_name)
{
    if ((NULL == table) || (NULL == config_name))
    {
        e_log("table or config_name is NULL in %s", __FUNCTION__);
        return NULL;
    }

    config_manage_t *s = NULL;
    HASH_FIND_STR(table, config_name, s);
    return s;
}

void clear_hash_table(config_manage_t *table)
{
    if (NULL == table)
    {
        e_log("table is NULL in %s", __FUNCTION__);
        return;
    }

    config_manage_t *current_user;
    config_manage_t *tmp;
    HASH_ITER(hh, table, current_user, tmp)
    {
        HASH_DEL(table, current_user);
        config_item_free(current_user);
    }
}



void config_manage_clear(void)
{
    clear_hash_table(g_config_manage);
    clear_hash_table(g_default_config_manage);
}



bool config_attach(const char *name, on_config_change cb)
{
    if ((NULL == name) || (NULL == cb))
    {
        e_log("name or cb is NULL in %s", __FUNCTION__);
        return false;
    }
    char *config_name = name_add_suffix(name);

    config_manage_t *s = NULL;
    HASH_FIND_STR(g_config_manage, config_name, s);
    free(config_name);
    if (s == NULL)
    {
        return false;
    }
    else
    {
        cb_list_t *new_owner = (cb_list_t *)malloc(sizeof(cb_list_t));
        if (NULL == new_owner)
        {
            w_log("new_owner malloc error");
            return false;
        }
        new_owner->cb = cb;
        new_owner->next = NULL;
        new_owner->prev = NULL;

        (void)pthread_mutex_lock(&s->owners_lock);
        if (NULL == s->owners)
        {
            s->owners = new_owner;
        }
        else
        {
            cb_list_t *head = s->owners;
            while(head->next != NULL)
            {
                head = head->next;
            }
            head->next = new_owner;
            new_owner->prev = head;
        }
        (void)pthread_mutex_unlock(&s->owners_lock);
        return true;
    }
}

static cb_list_t *find_owner(config_manage_t *table, on_config_change cb)
{
    if ((NULL == table) || (NULL == cb))
    {
        e_log("table or cb is NULL in %s", __FUNCTION__);
        return NULL;
    }

    (void)pthread_mutex_lock(&table->owners_lock);
    cb_list_t *head = table->owners;
    while (head != NULL)
    {
        if (head->cb == cb)
        {
            (void)pthread_mutex_unlock(&table->owners_lock);
            return head;
        }
        head = head->next;
    }
    (void)pthread_mutex_unlock(&table->owners_lock);
    return NULL;
}


bool config_detach(const char *name, on_config_change cb)
{
    if ((NULL == name) || (NULL == cb))
    {
        e_log("name or cb is NULL in %s", __FUNCTION__);
        return false;
    }
    char *config_name = name_add_suffix(name);

    config_manage_t *s = NULL;
    HASH_FIND_STR(g_config_manage, config_name, s);
    free(config_name);
    if (s == NULL)
    {
        e_log("%s in config manages is NULL in config_detach", name);
        return false;
    }
    else
    {
        cb_list_t *finded_owner = find_owner(s, cb);
        if (finded_owner == NULL)
        {
            e_log("%s finded_owner is NULL in config_detach", name);
            return false;
        }
        (void)pthread_mutex_lock(&s->owners_lock);

        /* 如果是链表的第一个元素 */
        if (finded_owner == s->owners)
        {
            s->owners = finded_owner->next;
            if (s->owners != NULL)
            {
                s->owners->prev = NULL;
            }
        }
        /* 如果是链表中间或最后一个元素 */
        else if (finded_owner->prev != NULL)
        {
            finded_owner->prev->next = finded_owner->next;
            if (finded_owner->next != NULL)
            {
                finded_owner->next->prev = finded_owner->prev;
            }
        }
        (void)pthread_mutex_unlock(&s->owners_lock);
        free(finded_owner);
        return true;
    }
}


static bool notify_owner(const char *config, cJSON *old_value, cJSON *new_value)
{
    if ((NULL == config) || (NULL == old_value) || (NULL == new_value))
    {
        e_log("config or old_value or new_value is NULL in %s", __FUNCTION__);
        return false;
    }
    config_manage_t *s = NULL;
    HASH_FIND_STR(g_config_manage, config, s);
    if (s == NULL)
    {
        e_log("file %s not exist, can't notify", config);
        return  false;
    }
    else
    {
        (void)pthread_mutex_lock(&s->owners_lock);
        cb_list_t *head = s->owners;
        while (head != NULL)
        {
            head->cb(old_value, new_value);
            head = head->next;
        }
        (void)pthread_mutex_unlock(&s->owners_lock);
        return true;
    }
}

static void delete_all_owners(config_manage_t *s)
{
    if (s == NULL)
    {
        e_log("error: s is NULL in %s", __FUNCTION__);
        return;
    }
    (void)pthread_mutex_lock(&s->owners_lock);
    cb_list_t *head = s->owners;
    while (head != NULL)
    {
        cb_list_t *temp = head;
        head = head->next;
        if(NULL != temp)
        {
            free(temp);
        }
    }
    s->owners = NULL;
    (void)pthread_mutex_unlock(&s->owners_lock);
}

