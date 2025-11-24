#include "../../include/websocket/libwebsockets.h"
#include <iostream>

#include <cstring>

typedef struct
{
    int session_id;
    time_t last_active;
} chat_session_t;

static int chat_cb(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
    switch (reason)
    {
    case LWS_CALLBACK_ESTABLISHED:
        std::cout << "LWS_CALLBACK_ESTABLISHED" << std::endl;
        break;
    case LWS_CALLBACK_RECEIVE:
    {
        std::cout << "LWS_CALLBACK_RECEIVE" << std::endl;
        std::cout << "Received from client: ";
        std::cout.write(static_cast<char*>(in), len);
        std::cout << std::endl;

        // Echo back
        unsigned char buf[LWS_PRE + 4096];
        if (len > 4096) len = 4096; // 防止溢出

        (void)memcpy(&buf[LWS_PRE], in, len);
        int m = lws_write(wsi, &buf[LWS_PRE], len, LWS_WRITE_TEXT);
        if (m < 0)
        {
            std::cerr << "Echo write failed" << std::endl;
            return -1;
        }
        break;
    }
    case LWS_CALLBACK_CLOSED:
        std::cout << "LWS_CALLBACK_CLOSED" << std::endl;
        break;
    default:
        break;
    }
    return 0;
}

static struct lws_protocols protocols[] =
{
    {
        .name = "chat-protocols",
        .callback = &chat_cb,
        .per_session_data_size = sizeof(chat_session_t),
        .rx_buffer_size = 0,
        .id = 0,
        .user = NULL,
        .tx_packet_size = 0
    },

    {NULL, NULL, 0, 0, 0, NULL, 0}
};

int main(int argc, char **argv)
{
    struct lws_context_creation_info info;
    struct lws_context *context;

    std::string address = "127.0.0.1";
    int port = 9002;

    memset(&info, 0, sizeof(info));
    info.port = port;
    info.iface = NULL;
    info.protocols = protocols;
    info.gid = -1;
    info.uid = -1;

    context = lws_create_context(&info);
    if (!context)
    {
        std::cout << "Failed to create libwebsockets context" << std::endl;
        return -1;
    }

    std::cout << "WebSocket server listening on ws://" << address << ":" << port << std::endl;

    while (1)
    {
        lws_service(context, 50);
    }

    lws_context_destroy(context);
    std::cout << "Server stopped" << std::endl;

    return 0;
}


/* g++ -g -I/home/zlgmcu/project/learnC++/include/websocket websocket_server.cpp -o server -L/home/zlgmcu/project/learnC++/lib -lwebsockets -lssl -lcrypto -lpthread -lm -ldl */