#include "connection.hpp"
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <sstream>
#include <fcntl.h>
#include <poll.h>

using namespace std;

string connection::read_data()
{
    char buffer[BUFF_SIZE];
    int n;

    if (!is_SSL && (n = recv(sockfd, buffer, BUFF_SIZE, 0)) < 0)
    {
        throw runtime_error(string("socket read failed: ") + strerror(errno));
    }
    if (is_SSL && (SSL_read_ex(ssl, buffer, BUFF_SIZE, (size_t *)&n)) <= 0)
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
    if (is_SSL && (SSL_write_ex(ssl, msg.c_str(), msg.size(), &n)) <= 0)
    {
        ERR_print_errors_fp(stderr);
        throw runtime_error("SSL socket send failed");
    }
}

void connection::send_msg(message msg)
{
    send_data(messageToString(msg));
}

void connection::convert_to_blocking()
{
    // Convert socket to blocking
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags < 0)
    {
        throw runtime_error(string("Error getting socket flags: ") + strerror(errno));
    }

    // Remove the non-blocking flag
    flags &= ~O_NONBLOCK;
    if (fcntl(sockfd, F_SETFL, flags) < 0)
    {
        throw runtime_error(string("Error setting socket to blocking mode: ") + strerror(errno));
    }
}

void connection::convert_to_non_blocking()
{
    // Convert socket to non-blocking
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags < 0)
    {
        throw runtime_error(string("Error getting socket flags: ") + strerror(errno));
    }

    // Add the non-blocking flag
    flags |= O_NONBLOCK;
    if (fcntl(sockfd, F_SETFL, flags) < 0)
    {
        throw runtime_error(string("Error setting socket to non-blocking mode: ") + strerror(errno));
    }
}

message connection::send_control(message message)
{
    char buffer[BUFF_SIZE];
    int n;
    int re_tries = 0;
    int ret;

    send_msg(message);
    convert_to_non_blocking();

    while (!is_SSL && (n = recv(sockfd, buffer, BUFF_SIZE, 0)) < 0)
    {
        if (re_tries == 5)
        {
            convert_to_blocking();
            send_msg(message);
            convert_to_non_blocking();
            re_tries = 0;
            continue;
        }
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            re_tries++;
            sleep(1);
            continue;
        }
        throw runtime_error(string("socket read failed: ") + strerror(errno));
    }
    while (is_SSL && (ret = SSL_read_ex(ssl, buffer, BUFF_SIZE, (size_t *)&n)) <= 0)
    {
        if (re_tries == 5)
        {
            convert_to_blocking();
            send_msg(message);
            convert_to_non_blocking();
            re_tries = 0;
            continue;
        }
        if (SSL_get_error(ssl, ret) == SSL_ERROR_WANT_READ)
        {
            re_tries++;
            sleep(1);
            continue;
        }
        ERR_print_errors_fp(stderr);
        throw runtime_error("SSL socket read failed");
    }

    convert_to_blocking();
    return construct_message(string(buffer, n));
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

bool connection::poll_msg()
{
    struct pollfd poll_fd = {
        .fd = sockfd,
        .events = POLLIN,
        .revents = 0
    };

    int res = poll(&poll_fd,1,0);
    if (res < 0)
    {
        throw runtime_error(string("socket poll failed: ") + strerror(errno));
    }

    return (res > 0);
}

connection::~connection()
{
    if (is_SSL)
    {
        int res;
        while ((res = SSL_shutdown(ssl)) != 1)
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
    cout << "closed\n";
}