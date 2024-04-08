#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <unistd.h>
#include <stdexcept>
#include <iostream>
#include <cstring>
#include <openssl/ssl.h>
#include <openssl/err.h>
using namespace std;

/* MEMBER starts */
/* INPUT strats */
const char *hostname = "localhost";
const short int port = 8080;
/* INPUT ends */
int sockfd;
SSL_CTX *ctx;
SSL *ssl;
bool is_SSL;
/* MEMBER ends */

void create_socket()
{
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        throw runtime_error(string("socket creation failed: ") + strerror(errno));
    }
}

void bind_sock()
{
    struct sockaddr_in my_addr;
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sockfd, (sockaddr *)&my_addr, sizeof(my_addr)) < 0)
    {
        throw runtime_error(string("socket creation failed: ") + strerror(errno));
    }
}

void send_data(const string &msg)
{
    size_t n;
    if (!is_SSL && send(sockfd, msg.c_str(), msg.size(), MSG_CONFIRM) < 0)
    {
        throw runtime_error(string("socket send failed: ") + strerror(errno));
    }
    if (is_SSL && (SSL_write_ex(ssl, msg.c_str(), msg.size(), &n)) == 0)
    {
        ERR_print_errors_fp(stderr);
        throw runtime_error("SSL socket recv failed");
    }
    cout << "SENT: " << msg << "\n";
}

string recv_data()
{
    char buff[4096 + 1];
    int n;
    if (!is_SSL && (n = recv(sockfd, buff, 4096, 0)) < 0)
    {
        throw runtime_error(string("socket recv failed: ") + strerror(errno));
    }
    if (is_SSL && (SSL_read_ex(ssl, buff, 4096, (size_t *)&n)) == 0)
    {
        ERR_print_errors_fp(stderr);
        throw runtime_error("SSL socket recv failed");
    }
    buff[n] = '\0';
    cout << "RECIEVED: " << buff << "\n";
    return buff;
}

void establish_conn()
{
    struct sockaddr_in to_addr;
    char buff[4097];
    socklen_t len = sizeof(to_addr);
    int n;

    if ((n = recvfrom(sockfd, buff, 4096, MSG_WAITALL, (struct sockaddr *)&to_addr, &len)) < 0)
    {
        throw runtime_error(string("socket connection failed: ") + strerror(errno));
    }
    buff[n] = '\0';

    cout << "RECIEVED: " << buff << "\n";
    cout << "RECIEVED PORT: " << ntohs(to_addr.sin_port) << "\n";

    if (connect(sockfd, (struct sockaddr *)&to_addr, sizeof(to_addr)) < 0)
    {
        throw runtime_error(string("socket connection failed: ") + strerror(errno));
    }
    send_data("chat_hello ack");
}



void prepare_ctx()
{
    ctx = SSL_CTX_new(DTLS_server_method());
    SSL_CTX_set_min_proto_version(ctx, DTLS1_2_VERSION);

    const char *cipher_list = "ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:TLS_AES_256_GCM_SHA384:AES256-GCM-SHA384:AES128-SHA256:AES128-SHA";

    if (SSL_CTX_set_cipher_list(ctx, cipher_list) == 0)
    {
        ERR_print_errors_fp(stderr);
        throw runtime_error("Unable to set cipher suites");
    }
    
     if (SSL_CTX_use_certificate_file(ctx, "bob-cert.crt", SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        throw runtime_error("could not use bob-cert.crt\n");
    }
    if (SSL_CTX_use_PrivateKey_file(ctx, "bob-key.pem", SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        throw runtime_error("could not use bob-key.pem\n");
    }

    if (!SSL_CTX_load_verify_file(ctx, "certs/cert-chain.crt"))
    {
        ERR_print_errors_fp(stderr);
        throw runtime_error("Error in loading certificate");
    }
}

void prepare_ssl()
{
    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sockfd);
}

void startSSL()
{
    recv_data();
    send_data("start SSL ack");
    prepare_ctx();
    prepare_ssl();

    if (SSL_accept(ssl) != 1)
    {
        ERR_print_errors_fp(stderr);
        throw runtime_error("Error in handshake");
    }

    is_SSL = true;
    cout << "SSL success\n";
}

int main(int argc, char *argv[])
{
    try
    {
        /* constructor starts */
        is_SSL = false;
        ssl = NULL;
        ctx = NULL;
        create_socket();
        bind_sock();
        establish_conn();
        /* constructor ends */

        startSSL();

        send_data("hello");
        recv_data();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }

    /* DESTRUCTOR starts */
    close(sockfd);
    if (ssl)
    {
        SSL_shutdown(ssl);
        SSL_free(ssl);
    }
    if (ctx)
        SSL_CTX_free(ctx);
    /* DESTRUCTOR ends */

    return 0;
}
