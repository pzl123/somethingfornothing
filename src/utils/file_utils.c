#include "file_utils.h"
#include "utils.h"

#include <unistd.h>

// #include "easylogger/elog.h"

// #include "common/public_def.h"
#include "uthash/utstring.h"
#include "hv/hbase.h"

uint32_t file_size(const char *file_path)
{
    if (NULL == file_path)
    {
        return 0;
    }

    struct stat st = {0};
    if (stat(file_path, &st) == 0)
    {
        return st.st_size;
    }

    return 0;
}

/**
 * @brief get the file data object
 *
 * @param file_path 绝对路径
 * @param data 数据（动态内存，外部释放）
 * @param len 数据长度
 * @return int32_t 成功返回 PCU_ERR_SUCCESS，失败返回 PCU_ERR
 */
int32_t get_file_data(const char *file_path, uint8_t **data, uint32_t *len)
{
    if ((NULL == file_path) || (NULL == data) || (NULL == len))
    {
        return PCU_ERR;
    }

    FILE *fp = fopen(file_path, "r");
    if (fp == NULL)
    {
        e_log("fopen failed, file_path: %s", file_path);
        return PCU_ERR;
    }

    int32_t ret = PCU_ERR;
    uint8_t *buf = NULL;
    if (fseek(fp, 0, SEEK_END) != 0)
    {
        e_log("fseek failed!");
        goto end;
    }

    /* 获取文件大小 */
    uint32_t fSize = ftell(fp);
    if (fSize <= 0)
    {
        e_log("ftell failed!");
        goto end;
    }

    /* 调整文件指针到初始位置 */
    rewind(fp);

    buf = (uint8_t *)malloc(fSize);
    if (buf == NULL)
    {
        e_log("malloc failed!");
        goto end;
    }
    (void)memset(buf, 0x00, fSize);

    uint32_t readSize = fread(buf, sizeof(char), fSize, fp);
    if (readSize <= 0)
    {
        e_log("fread failed!");
        goto end;
    }
    *data = buf;
    *len = readSize;

    ret = PCU_ERR_SUCCESS;

end:
    (void)fclose(fp);
    if (ret != PCU_ERR_SUCCESS)
    {
        free(buf);
    }

    return ret;
}

bool file_copy(const char *dst, const char *src)
{
    UT_string *cmd = NULL;
    utstring_new(cmd);
    utstring_printf(cmd, "cp %s %s", src, dst);
    FILE *fp = popen(utstring_body(cmd), "r");
    if (NULL == fp)
    {
        e_log("cmd=%s msg=%s", utstring_body(cmd), strerror(errno));
        utstring_free(cmd);
        return false;
    }

    utstring_free(cmd);
    (void)pclose(fp);
    return true;
}

bool file_move(const char *dst, const char *src)
{
    UT_string *cmd = NULL;
    utstring_new(cmd);
    utstring_printf(cmd, "mv %s %s", src, dst);
    FILE *fp = popen(utstring_body(cmd), "r");
    if (NULL == fp)
    {
        e_log("cmd=%s msg=%s", utstring_body(cmd), strerror(errno));
        utstring_free(cmd);
        return false;
    }

    utstring_free(cmd);
    (void)pclose(fp);
    return true;
}

bool file_remove(const char *dst)
{
    UT_string *cmd = NULL;
    utstring_new(cmd);
    utstring_printf(cmd, "rm %s", dst);
    FILE *fp = popen(utstring_body(cmd), "r");
    if (NULL == fp)
    {
        e_log("cmd=%s msg=%s", utstring_body(cmd), strerror(errno));
        utstring_free(cmd);
        return false;
    }

    utstring_free(cmd);
    (void)pclose(fp);
    return true;
}

bool file_exist(const char *path)
{
    return (access(path, F_OK) == 0) ? true : false;
}


int64_t dir_size(const char *path)
{
    char cmd[512] = {0};
    (void)snprintf(cmd, sizeof(cmd), "du -s %s | awk '{print $1}'", path);
    FILE *fp = popen(cmd, "r");
    if (NULL == fp)
    {
        e_log("cmd=%s", cmd);
        return -1;
    }

    char buf[128] = {0};
    size_t bytes = fread(buf, 1, sizeof(buf), fp);
    fclose(fp);
    if (bytes == 0)
    {
        e_log("fread error cmd=%s", cmd);
        return -1;
    }

    int64_t size = 0;
    if (1 != sscanf(buf, "%ld", &size))
    {
        e_log("buf=%s", buf);
        return -1;
    }
    return size;
}

bool is_directory(const char *path)
{
    struct stat path_stat = {0};
    if (stat(path, &path_stat) != 0)
    {
        return false;
    }

    return S_ISDIR(path_stat.st_mode);
}

char *extract_directory(const char *path)
{
    if (!path)
    {
        return NULL;
    }

    size_t len = strlen(path);
    if (len == 0)
    {
        return strdup(".");
    }

    const char *last_slash = NULL;
    for (size_t i = len - 1; i > 0; i--)
    {
        if (path[i] == '/' || path[i] == '\\')
        {
            last_slash = &path[i];
            break;
        }
    }

    if (!last_slash)
    {
        return strdup(".");
    }

    if (last_slash == path)
    {
        char *result = malloc(2);
        strcpy(result, "/");
        return result;
    }

    size_t dir_len = last_slash - path;
    char *result = malloc(dir_len + 1);
    strncpy(result, path, dir_len);
    result[dir_len] = '\0';
    return result;
}