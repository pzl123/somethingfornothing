#include <iostream>
#include <cstring>

#include "../../include/websocket/libwebsockets.h"
static bool new_message = false;
static std::string message_to_send;

static int client_cb(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
    switch (reason)
    {
    case LWS_CALLBACK_CLIENT_ESTABLISHED:
        std::cout << "Connected to server, sending message..." << std::endl;
        lws_callback_on_writable(wsi);// 负责触发写回调
        break;

    case LWS_CALLBACK_CLIENT_WRITEABLE:
    {
        // if (!new_message) break;

        message_to_send = "hello world websocket";
        unsigned char buf[LWS_PRE + 256];
        size_t msg_len = message_to_send.length();
        if (msg_len > 256) msg_len = 256;

        memcpy(&buf[LWS_PRE], message_to_send.c_str(), msg_len);
        int m = lws_write(wsi, &buf[LWS_PRE], msg_len, LWS_WRITE_TEXT);
        if (m < 0) {
            std::cerr << "Write error" << std::endl;
            return -1;
        }
        // new_message = false;
        break;
    }

    case LWS_CALLBACK_CLIENT_RECEIVE:
        std::cout << "echo from server: ";
        std::cout.write(static_cast<char*>(in), len);
        std::cout << std::endl;
        break;

    case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
        std::cout << "LWS_CALLBACK_CLIENT_CONNECTION_ERROR" << std::endl;
        break;

    default:
        break;
    }
    return 0;
}

static struct lws_protocols protocols[] =
{
    {.name = "chat-protocols",
     .callback = &client_cb,
     .per_session_data_size = 0,
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
    struct lws_client_connect_info i;
    struct lws_context * context;
    struct lws *wsi;

    (void)memset(&info, 0, sizeof(info));
    info.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
    info.port = CONTEXT_PORT_NO_LISTEN;  // 客户端不需要监听
    info.protocols = protocols;

    context = lws_create_context(&info);
    if (!context)
    {
        std::cout << "Failed to create context" << std::endl;
        return -1;
    }

    (void)memset(&i, 0, sizeof(i));
    i.context = context;
    i.address = "127.0.0.1";
    i.port = 9002;
    i.path = "/";
    i.host = i.address;
    i.origin = i.address;
    i.protocol = "chat-protocols"; // 必须与服务的协议名一致
    i.ssl_connection = 0; // 不使用TLS

    wsi = lws_client_connect_via_info(&i);
    if (!wsi)
    {
        std::cout << "Failed to connect to server" << std::endl;
    }

    std::cout << "Connecting to ws://localhost:9002..." << std::endl;

    while (1)
    {
        lws_service(context, 50);
    }

    lws_context_destroy(context);
    return 0;
}

/* g++ -g -I/home/zlgmcu/project/learnC++/include/websocket websocket_client.cpp -o client -L/home/zlgmcu/project/learnC++/lib -lwebsockets -lssl -lcrypto -lpthread -lm -ldl */