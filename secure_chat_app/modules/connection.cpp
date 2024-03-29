#include "connection.hpp"
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <sstream>
using namespace std;

string connection::read_data()
{
    char buffer[BUFF_SIZE];
    int n;

    if (!is_SSL && (n = recv(sockfd, buffer, BUFF_SIZE, 0)) < 0)
    {
        throw runtime_error(string("socket read failed: ") + strerror(errno));
    }
    if (is_SSL && (SSL_read_ex(ssl, buffer, BUFF_SIZE, (size_t *)&n)) < 0)
    {
        ERR_print_errors_fp(stderr);
        throw runtime_error("SSL socket read failed");
    }

    string res(buffer, n);

    return res;
}

message connection::construct_message(std::string msg)
{
    message_header hdr;

    memcpy(&hdr, msg.c_str(), HEADER_SIZE);

    string body(msg.c_str() + HEADER_SIZE, msg.size() - HEADER_SIZE);

    return {.hdr = hdr, .body = body};
}

message connection::read_msg()
{
    return construct_message(read_data());
}

string connection::messageToString(message msg)
{
    char header[HEADER_SIZE];
    memcpy(header, &(msg.hdr), HEADER_SIZE);
    string res(header, HEADER_SIZE);
    res.append(msg.body);
    return res;
}

void connection::send_data(string msg)
{
    size_t n;
    if (!is_SSL && send(sockfd, msg.c_str(), msg.size(), MSG_CONFIRM) < 0)
    {
        throw runtime_error(string("socket send failed: ") + strerror(errno));
    }
    if (is_SSL && (SSL_write_ex(ssl, msg.c_str(), msg.size(), &n)) < 0)
    {
        ERR_print_errors_fp(stderr);
        throw runtime_error("SSL socket send failed");
    }
}

void connection::send_msg(message msg)
{
    send_data(messageToString(msg));
}

void connection::create_socket()
{
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        throw runtime_error(string("socket creation failed: ") + strerror(errno));
    }
}

void connection::prepare_ssl()
{
    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sockfd);
}

connection::~connection()
{
    if (is_SSL)
    {
        int res;
        while((res = SSL_shutdown(ssl)) != 1)
        {
            if (res < 0)
            {
                ERR_print_errors_fp(stderr);
                break;
            }
        }
        SSL_free(ssl);
        SSL_CTX_free(ctx);
    }
    close(sockfd);
}