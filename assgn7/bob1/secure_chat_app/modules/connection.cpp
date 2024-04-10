#include "connection.hpp"
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <sstream>
#include <fcntl.h>
#include "cert_locations.hpp"
using namespace std;

string connection::read_data()
{
    char buffer[BUFF_SIZE];
    int n;
    int ret;

    if (!is_SSL && (n = recv(sockfd, buffer, BUFF_SIZE, 0)) < 0)
    {
        throw runtime_error(string("socket read failed: ") + strerror(errno));
    }
    if (is_SSL && (ret = SSL_read_ex(ssl, buffer, BUFF_SIZE, (size_t *)&n)) <= 0)
    {
        if (SSL_get_error(ssl, ret) == SSL_ERROR_ZERO_RETURN)
        {
            throw runtime_error("Connection ended by peer");
        }
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

int verify_callback(int preverify_ok, X509_STORE_CTX *ctx)
{

    // X509_STORE* store = SSL_CTX_get_cert_store(ctx);
    // SSL *ssl = static_cast<SSL *>(X509_STORE_CTX_get_ex_data(ctx, SSL_get_ex_data_X509_STORE_CTX_idx()));
    // SSL_CTX *ssl_ctx = SSL_get_SSL_CTX(ssl);
    // X509 *cert = X509_STORE_CTX_get0_cert(ctx);

    if (X509_STORE_CTX_get_error_depth(ctx) == 0)
    {
        return 1;
    }

    return preverify_ok;
}

void connection::prepare_ctx()
{
    ctx = SSL_CTX_new(DTLS_method());
    SSL_CTX_set_min_proto_version(ctx, DTLS1_2_VERSION);

    const char *cipher_list = "ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:TLS_AES_256_GCM_SHA384:AES256-GCM-SHA384:AES128-SHA256:AES128-SHA";

    if (SSL_CTX_set_cipher_list(ctx, cipher_list) == 0)
    {
        ERR_print_errors_fp(stderr);
        throw runtime_error("Unable to set cipher suites");
    }

    if (SSL_CTX_use_certificate_file(ctx, CERT_LOC, SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        throw runtime_error("Can't load certificate");
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, PRIVATE_KEY_LOC, SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        throw runtime_error("Can't load private key");
    }

    if (!SSL_CTX_load_verify_locations(ctx, CERT_CHAIN_LOC, NULL))
    {
        ERR_print_errors_fp(stderr);
        throw runtime_error("Error in loading certificate");
    }

    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, verify_callback);
    SSL_CTX_set_verify_depth(ctx, 4);

    session_handler();
    // SSL_CTX_set_session_ticket_cb(ctx, NULL, , NULL);
}

void connection::prepare_ssl()
{
    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sockfd);
}

void connection::stop()
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
    }
    close(sockfd);
}

connection::~connection()
{
    stop();
    if (is_SSL)
    {
        // int res;
        // while ((res = SSL_shutdown(ssl)) != 1)
        // {
        //     if (res < 0)
        //     {
        //         ERR_print_errors_fp(stderr);
        //         break;
        //     }
        // }
        SSL_free(ssl);
        SSL_CTX_free(ctx);
    }
    close(sockfd);
}