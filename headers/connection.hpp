#pragma once

#include <string>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#define BODY_SIZE 4096

#define DATA 0
#define CONTROL 1

struct message_header
{
    uint64_t type;
    uint64_t size;
    int64_t time;
};

#define HEADER_SIZE sizeof(message_header)
#define BUFF_SIZE (HEADER_SIZE + BODY_SIZE)

struct message
{
    struct message_header hdr;
    std::string body;
};

// Can be used in priority queue or sorting
bool operator<(const message &a, const message &b);
bool operator>(const message &a, const message &b);
bool operator<=(const message &a, const message &b);
bool operator>=(const message &a, const message &b);

class connection
{
protected:
    int sockfd;
    struct sockaddr_in to_addr;
    bool is_SSL;
    socklen_t len;
    SSL_CTX *ctx;
    SSL *ssl;
    
    message construct_message(std::string);
    std::string read_raw_data();
    std::string messageToString(message);
    void send_data(std::string);

public:
    connection();
    message read();
    void send(message);
    virtual void startSSL() = 0;
    virtual ~connection();
};

class client_connection : public connection
{
private:

public:
    client_connection(uint16_t port);
    // message read();
    // void send(message);
    void startSSL();
    ~client_connection();
};

class server_connection : public connection
{
private:
    struct sockaddr_in my_addr;

public:
    server_connection(uint16_t port);
    // message read();
    // void send(message);
    void startSSL();
    ~server_connection();
};