#pragma once
#include "message.hpp"
#include <string>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <unistd.h>

class connection
{
protected:
    /* INPUT strats */
    std::string hostname;
    uint16_t port;
    /* INPUT ends */
    int sockfd;
    bool is_SSL;
    SSL_CTX *ctx;
    SSL *ssl;

    void create_socket();
    virtual void establish_conn() = 0;
    virtual void prepare_ctx() = 0;
    void prepare_ssl();

    message construct_message(std::string);
    std::string messageToString(message);
    std::string read_data();
    void send_data(std::string);

public:
    message read_msg();
    void send_msg(message);
    message send_control(message);
    virtual void startSSL() = 0;
    ~connection();
};

class client_connection : public connection
{
private:
    void establish_conn();
    void prepare_ctx();

public:
    client_connection(std::string _hostname, uint16_t _port);
    void startSSL();
};

class server_connection : public connection
{
private:
    void bind_sock();
    void establish_conn();
    void prepare_ctx();

public:
    server_connection(uint16_t _port);
    void startSSL();
};