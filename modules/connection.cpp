#include "connection.hpp"
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <sstream>
using namespace std;
std::string to_text[] = {"DATA", "CONTROL"};

bool operator<(const message &a, const message &b)
{
    return (a.hdr.time < b.hdr.time);
}
bool operator>(const message &a, const message &b)
{
    return (a.hdr.time > b.hdr.time);
}
bool operator<=(const message &a, const message &b)
{
    return (a.hdr.time <= b.hdr.time);
}
bool operator>=(const message &a, const message &b)
{
    return (a.hdr.time >= b.hdr.time);
}

connection::connection()
{
    SSL = false;
}

string connection::read_raw_data()
{
    char buffer[BUFF_SIZE + 1];
    int n;

    if (!SSL)
        n = recvfrom(sockfd, (char *)buffer, BUFF_SIZE, MSG_WAITALL , (struct sockaddr *)&to_addr, &len);
    buffer[n] = '\0';
    
    string res;
    res.append(buffer,n+1);

    return res;
}

message connection::construct_message(std::string msg)
{
    message_header hdr;

    memcpy(&hdr,msg.c_str(),HEADER_SIZE);

    string body(msg.c_str() + HEADER_SIZE);

    return {.hdr = hdr,.body = body};
}

message connection::read()
{
    return construct_message(read_raw_data());
}

string connection::messageToString(message msg)
{
    char header[HEADER_SIZE];
    header[HEADER_SIZE] = '\0';
    memcpy(header, &(msg.hdr), HEADER_SIZE);
    string res;
    res.append(header, HEADER_SIZE);
    res.append(msg.body);
    return res;
}

void connection::send_data(string msg)
{
    if (!SSL)
        sendto(sockfd, msg.c_str(), msg.size(), MSG_CONFIRM, (const struct sockaddr *)&to_addr, len);
}

void connection::send(message msg)
{
    send_data(messageToString(msg));
}

connection::~connection()
{
}

client_connection::client_connection(uint16_t port)
{
    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        throw runtime_error(string("socket creation failed: ") + strerror(errno));
    }
    memset(&to_addr, 0, sizeof(to_addr));

    // Filling server information
    to_addr.sin_family = AF_INET;
    to_addr.sin_port = htons(port);
    to_addr.sin_addr.s_addr = INADDR_ANY;

    len = sizeof(to_addr);
}

void client_connection::startSSL()
{
    // Implement SSL stuff
}

client_connection::~client_connection()
{
    close(sockfd);
}

server_connection::server_connection(uint16_t port)
{
    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        throw runtime_error(string("socket creation failed: ") + strerror(errno));
    }

    memset(&my_addr, 0, sizeof(my_addr));
    memset(&to_addr, 0, sizeof(to_addr));

    // Filling server information
    my_addr.sin_family = AF_INET; // IPv4
    my_addr.sin_addr.s_addr = INADDR_ANY;
    my_addr.sin_port = htons(port);

    // Bind the socket with the server address
    if (bind(sockfd, (const struct sockaddr *)&my_addr, sizeof(my_addr)) < 0)
    {
        throw runtime_error(string("bind failed: ") + strerror(errno));
    }

    len = sizeof(to_addr);
}

void server_connection::startSSL()
{
    // Implement SSL stuff
}

server_connection::~server_connection()
{
    // Destroy
}
