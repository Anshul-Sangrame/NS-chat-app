#include "connection.hpp"
#include <cstring>
#include <stdexcept>
#include <netdb.h>
using namespace std;

client_connection::client_connection(std::string _to_name , uint16_t _port)
{
    to_name = _to_name;
    port = _port;
    is_SSL = false;
    create_socket();
    establish_conn();
    startSSL();
}

void client_connection::establish_conn()
{
    struct sockaddr_in to_addr;
    to_addr.sin_family = AF_INET;
    to_addr.sin_port = htons(port);
    hostent *lh;
    if ((lh = gethostbyname(to_name.c_str())) == NULL)
    {
        herror("gethostbyname");
        throw runtime_error("");
    }
    to_addr.sin_addr.s_addr = *(in_addr_t *)lh->h_addr_list[0];
    if (connect(sockfd, (struct sockaddr *)&to_addr, sizeof(to_addr)) < 0)
    {
        throw runtime_error(string("socket connection failed: ") + strerror(errno));
    }
    message chat_hello = {
        .hdr = {
            .type = CONTROL,
            .time = time(NULL)
        },
        .body = "CHAT_HELLO"
    };
    send_msg(chat_hello);
    message reply = read_msg();
    if (reply.hdr.type != CONTROL || reply.body != "CHAT_OK_REPLY")
    {
        throw runtime_error(string("socket connection failed: invalid CHAT_OK_REPLY"));
    }
}

void client_connection::startSSL()
{
    message start_ssl = {
        .hdr = {
            .type = CONTROL,
            .time = time(NULL)
        },
        .body = "CHAT_START_SSL"
    };
    send_msg(start_ssl);
    message reply = read_msg();
    if (reply.hdr.type != CONTROL)
    {
        throw runtime_error(string("SSL socket connection failed: invalid response to start_ssl"));
    }
    if (reply.body == "CHAT_START_SSL_NOT_SUPPORTED") return;
    if (reply.body != "CHAT_START_SSL_ACK")
    {
        throw runtime_error(string("SSL socket connection failed: invalid response to start_ssl"));
    }

    prepare_ctx();
    prepare_ssl();

    if (SSL_connect(ssl) != 1)
    {
        ERR_print_errors_fp(stderr);
        throw runtime_error(string("SSL socket connection failed: error in handshake"));
    }

    is_SSL = true;
}