#pragma once

#include <string>
#include <arpa/inet.h>
#define BUFF_SIZE 4096 + 1

#define DATA 0
#define CONTROL 1

struct message
{
    int type;
    time_t time;
    std::string body;
};

std::string messageToString(message);
message stringToMessage(std::string);

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
    bool SSL;
    socklen_t len;

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