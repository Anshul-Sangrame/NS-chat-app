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
    is_SSL = false;
    ssl = NULL;
    ctx = NULL;
}

string connection::read_raw_data()
{
    char buffer[BUFF_SIZE + 1];
    int n;

    if (!is_SSL)
        n = recvfrom(sockfd, (char *)buffer, BUFF_SIZE, MSG_WAITALL, (struct sockaddr *)&to_addr, &len);
    buffer[n] = '\0';

    string res;
    res.append(buffer, n + 1);

    return res;
}

message connection::construct_message(std::string msg)
{
    message_header hdr;

    memcpy(&hdr, msg.c_str(), HEADER_SIZE);

    string body(msg.c_str() + HEADER_SIZE);

    return {.hdr = hdr, .body = body};
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
    if (!is_SSL)
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
    if (connect(sockfd, (struct sockaddr *)&to_addr, len) != 0)
    {
        throw runtime_error(string("connect failed: ") + strerror(errno));
    }
    ctx = SSL_CTX_new(DTLS_client_method());
    SSL_CTX_set_min_proto_version(ctx, DTLS1_2_VERSION);

    if (!SSL_CTX_load_verify_locations(ctx, "root-cert.crt", NULL))
    {
        ERR_print_errors_fp(stderr);
        throw runtime_error("");
    }

    const char* cipher_list = "ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:TLS_AES_256_GCM_SHA384:AES256-GCM-SHA384:AES128-SHA256:AES128-SHA";

    if(SSL_CTX_set_cipher_list(ctx, cipher_list) == 0) {
        ERR_print_errors_fp(stderr);
        throw runtime_error("Unable to set cipher suites");
    }

    ssl = SSL_new(ctx);
    bio = BIO_new_dgram(sockfd, BIO_CLOSE);
    BIO_ctrl(bio, BIO_CTRL_DGRAM_SET_CONNECTED, 0, &to_addr);
    SSL_set_bio(ssl, bio, bio);

    SSL_set_tlsext_host_name(ssl, "duckduckgo.com");

    if (SSL_connect(ssl) != 1)
    {
        ERR_print_errors_fp(stderr);
        throw runtime_error("Error in handshake");
    }
    cout << "SSL success\n";
}

client_connection::~client_connection()
{
    if (ssl)
    {
        SSL_shutdown(ssl);
        SSL_free(ssl);
    }
    else
    {
        close(sockfd);
    }
    if (ctx)
        SSL_CTX_free(ctx);
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
    is_SSL = true;
    ctx = SSL_CTX_new(DTLS_server_method());
    SSL_CTX_set_min_proto_version(ctx, DTLS1_2_VERSION);
    if (SSL_CTX_use_certificate_file(ctx, "root-cert.crt", SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        throw runtime_error("");
    }
    if (SSL_CTX_use_PrivateKey_file(ctx, "root-key.pem", SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        throw runtime_error("");
    }

    const char* cipher_list = "ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:TLS_AES_256_GCM_SHA384:AES256-GCM-SHA384:AES128-SHA256:AES128-SHA";

    if(SSL_CTX_set_cipher_list(ctx, cipher_list) == 0) {
        ERR_print_errors_fp(stderr);
        throw runtime_error("Unable to set cipher suites");
    }

    ssl = SSL_new(ctx);
    bio = BIO_new_dgram(sockfd, BIO_NOCLOSE);
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    BIO_ctrl(bio, BIO_CTRL_DGRAM_SET_RECV_TIMEOUT, 0, &timeout);
    SSL_set_bio(ssl, bio, bio);
    // SSL_set_fd(ssl, sockfd);

    // if (SSL_do_handshake(ssl) != 1)
    // if (DTLSv1_listen(ssl,(BIO_ADDR *)&to_addr) < 1)
    if (SSL_accept(ssl) != 1)
    {
        ERR_print_errors_fp(stderr);
        throw runtime_error("Error in handshake");
    }

    cout << "SSL success\n";
}

server_connection::~server_connection()
{
    if (ssl)
    {
        SSL_shutdown(ssl);
        SSL_free(ssl);
    }
    if (ctx)
        SSL_CTX_free(ctx);
}
