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
struct sockaddr_in to_addr;
struct sockaddr_in my_addr;
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

void send_data(const string &msg)
{
    size_t n;
    if (!is_SSL && send(sockfd, msg.c_str(), msg.size(), MSG_CONFIRM) < 0)
    {
        throw runtime_error(string("socket send failed: ") + strerror(errno));
    }
    if (is_SSL && (SSL_write_ex(ssl,msg.c_str(),msg.size(),&n)) < 0)
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
    if (is_SSL && (SSL_read_ex(ssl,buff,4096,(size_t *)&n)) < 0)
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
    if (connect(sockfd, (struct sockaddr *)&to_addr, sizeof(to_addr)) < 0)
    {
        throw runtime_error(string("socket connection failed: ") + strerror(errno));
    }
    send_data("chat_hello");
    recv_data();
}

void prepare_ctx()
{
    ctx = SSL_CTX_new(DTLS_client_method());
    if (!SSL_CTX_load_verify_locations(ctx, "server-certificate.pem", NULL))
    {
        ERR_print_errors_fp(stderr);
        throw runtime_error("Error in loading certificate");
    }
    const char *cipher_list = "ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:TLS_AES_256_GCM_SHA384:AES256-GCM-SHA384:AES128-SHA256:AES128-SHA";

    if(SSL_CTX_set_cipher_list(ctx, cipher_list) == 0) {
        ERR_print_errors_fp(stderr);
        throw runtime_error("Unable to set cipher suites");
    }
}

void prepare_ssl()
{
    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sockfd);
}

void startSSL()
{
    send_data("Start SSL");
    recv_data();
    is_SSL = true;
    prepare_ctx();
    prepare_ssl();

    if (SSL_connect(ssl) != 1)
    {
        ERR_print_errors_fp(stderr);
        throw runtime_error("Error in handshake");
    }
    cout << "SSL success\n";
}

int main(int argc, char *argv[])
{
    try
    {
        /* Cnstructor starts */
        to_addr.sin_family = AF_INET;
        to_addr.sin_port = htons(port);
        to_addr.sin_addr.s_addr = INADDR_ANY;
        create_socket();
        establish_conn();
        is_SSL = false;
        ssl = NULL;
        ctx = NULL;
        /* constructor ends */

        startSSL();

        recv_data();
        send_data("hello back\n");
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
    if (ctx) SSL_CTX_free(ctx);
    /* DESTRUCTOR ends */

    return 0;
}