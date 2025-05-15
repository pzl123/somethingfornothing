#include "fcgi.h"
#include "fcgi/fcgi_stdio.h"
#include "fcgi/errors/api_code.h"

#include "fcgi/controller/relay_cnt.h"

#define MAX_API_HANDLER_LEN (10)

enum {
    METHOD_ERR = 0,
    METHOD_GET,
    METHOD_POST,
};


typedef struct
{
    const char *path;
    void (*func)(RelayData_t *data, FCGX_Request *req);
} api_router_t;

static const api_router_t get_router[] = {
{"/relay_cnt", query_relay_cnt}
};
static const api_router_t post_router[] = {
};
typedef struct
{
    const api_router_t *router;
    int32_t len;
    int32_t method;
} api_router_group_t;

static const api_router_group_t router_group[] = {
    {.router = get_router,     .len = (sizeof(get_router)/sizeof(get_router[0])),    .method = METHOD_GET},
    {.router = post_router,    .len = (sizeof(post_router)/sizeof(post_router[0])),  .method = METHOD_POST},
};



static int32_t method_string_to_enum(const char *str)
{
    if (NULL == str)
    {
        return METHOD_ERR;
    }
    if (0 == strcmp(str, "GET"))
    {
        return METHOD_GET;
    }
    else if (0 == strcmp(str, "POST"))
    {
        return METHOD_POST;
    }
    else
    {
        return METHOD_ERR;
    }
}

static const api_router_t* find_router(int32_t mehtod, const char *path_info)
{
    int32_t router_len = (sizeof(router_group)/sizeof(router_group[0]));
    for (int32_t i = 0; i < router_len; i++)
    {
        if (mehtod == router_group[i].method)
        {
            for (int32_t j = 0; j < router_group[i].len; j++)
            {
                if (0 == strcmp(router_group[i].router[j].path, path_info))
                {
                    return &(router_group[i].router[j]);
                }
            }
        }
    }
    return NULL;
}


RelayData_t relay_data[] = {
    {1, "DO_DC_INPUT1_POS", "R1", 5},
    {2, "DO_DC_INPUT1_NEG", "R2", 3},
    {3, "DO_DC_INPUT1_PRE", "R3", 7},
    {4, "DO_DC_INPUT2_POS", "R4", 2},
    {5, "DO_DC_INPUT2_NEG", "R5", 4},
    {6, "DO_DC_INPUT2_PRE", "R6", 6}
};

void fcgi_main(dao_t *dao)
{
    (void)dao;
    FCGX_Request req;
    FCGX_Init();
    FCGX_InitRequest(&req, 0, 0);

    while (FCGX_Accept_r(&req) == 0)
    {
        const char *request_method = FCGX_GetParam("REQUEST_METHOD", req.envp);
        int32_t mehtod = method_string_to_enum(request_method);
        if (METHOD_ERR == mehtod)
        {
            api_print(&req, API_ERR_METHOD, NULL);
            continue;
        }

        const char *path_info = FCGX_GetParam("PATH_INFO", req.envp);
        if (NULL == path_info)
        {
            api_print(&req, API_ERR_URI, NULL);
            continue;
        }

        const api_router_t *router = find_router(mehtod, path_info);
        if (NULL == router)
        {
            api_print(&req, API_ERR_URI, NULL);
            continue;
        }
        router->func(relay_data, &req);
    }
}

