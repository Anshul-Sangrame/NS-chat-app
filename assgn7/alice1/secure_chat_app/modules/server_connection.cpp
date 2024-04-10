#include "connection.hpp"
#include <cstring>
#include <stdexcept>
#include <netdb.h>
using namespace std;

server_connection::server_connection(uint16_t _port)
{
    is_SSL = false;
    port = _port;
    create_socket();
    bind_sock();
    establish_conn();
    startSSL();
}

void server_connection::bind_sock()
{
    struct sockaddr_in my_addr;
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sockfd, (sockaddr *)&my_addr, sizeof(my_addr)) < 0)
    {
        throw runtime_error(string("socket bind failed: ") + strerror(errno));
    }
}

void server_connection::establish_conn()
{
    struct sockaddr_in to_addr;
    char buff[BUFF_SIZE];
    socklen_t len = sizeof(to_addr);
    int n;

    if ((n = recvfrom(sockfd, buff, BUFF_SIZE, 0, (struct sockaddr *)&to_addr, &len)) < 0)
    {
        throw runtime_error(string("socket connection failed: ") + strerror(errno));
    }

    struct hostent *host = gethostbyaddr(&to_addr.sin_addr, sizeof(to_addr.sin_addr), AF_INET);
    if (host == NULL) {
        herror("gethostbyaddr");
        throw runtime_error("");
    }
    to_name = host->h_name;

    message msg = construct_message(string(buff,n));

    if (msg.hdr.type != CONTROL || msg.body != "CHAT_HELLO")
    {
        throw runtime_error(string("socket connection failed: invalid CHAT_HELLO"));
    }

    if (connect(sockfd, (struct sockaddr *)&to_addr, sizeof(to_addr)) < 0)
    {
        throw runtime_error(string("socket connection failed: ") + strerror(errno));
    }

    message reply = {
        .hdr = {
            .type = CONTROL,
            .time = time(NULL)
        },
        .body = "CHAT_OK_REPLY"
    };
    send_msg(reply);
}

void server_connection::session_handler()
{
    // handles server side session
}

void server_connection::startSSL()
{
    message msg = read_msg();
    if (msg.hdr.type != CONTROL)
    {
        throw runtime_error(string("SSL socket connection failed: Non control message recieved"));
    }
    
    if (msg.body == "CHAT_NO_SSL")
    {
        message reply = {
        .hdr = {
            .type = CONTROL,
            .time = time(NULL)
        },
        .body = "CHAT_NO_SSL_ACK"
        };
        send_msg(reply);
        return;
    }

    if (msg.body != "CHAT_START_SSL")
    {
        throw runtime_error(string("SSL socket connection failed: invalid start_ssl"));
    }

    message reply = {
        .hdr = {
            .type = CONTROL,
            .time = time(NULL)
        },
        .body = "CHAT_START_SSL_ACK"
    };
    send_msg(reply);

    prepare_ctx();
    prepare_ssl();

    if (SSL_accept(ssl) != 1)
    {
        ERR_print_errors_fp(stderr);
        throw runtime_error("Error in handshake");
    }

    is_SSL = true;
}