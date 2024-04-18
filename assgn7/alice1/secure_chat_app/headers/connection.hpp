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
    uint16_t port;
    /* INPUT ends */
    int sockfd;
    bool is_SSL;
    SSL_CTX *ctx;
    SSL *ssl;
    SSL_SESSION* session;

    void create_socket();
    virtual void establish_conn() = 0;
    void prepare_ctx();
    void prepare_ssl();

    message construct_message(std::string);
    std::string messageToString(message);
    std::string read_data();
    void convert_to_non_blocking();
    void convert_to_blocking();
    void send_data(std::string);
    void stop();
    virtual void startSSL() = 0;

public:
    std::string to_name;
    message read_msg();
    void send_msg(message);
    bool poll_msg();
    message send_control(message);
    ~connection();
};

class client_connection : public connection
{
private:
    void establish_conn();
    void startSSL();
    // void session_handler();
    void session_get_from_file();
    void session_store_in_file();

public:
    client_connection(std::string _hostname, uint16_t _port);
};

class server_connection : public connection
{
private:
    void bind_sock();
    void establish_conn();
    void startSSL();
    void session_handler();

public:
    server_connection(uint16_t _port);
};