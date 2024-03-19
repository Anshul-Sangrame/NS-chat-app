#include <arpa/inet.h> // inet_addr()
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> // bzero()
#include <sys/socket.h>
#include <unistd.h> // read(), write(), close()
#include <openssl/ssl.h>
#include <openssl/err.h>
#define MAX 80
#define PORT 8080
#define SA struct sockaddr
void func(int sockfd)
{
	char buff[MAX];
	int n;
	for (;;) {
		bzero(buff, sizeof(buff));
		printf("Enter the string : ");
		n = 0;
		while ((buff[n++] = getchar()) != '\n')
			;
		write(sockfd, buff, sizeof(buff));
		bzero(buff, sizeof(buff));
		read(sockfd, buff, sizeof(buff));
		printf("From Server : %s", buff);
		if ((strncmp(buff, "exit", 4)) == 0) {
			printf("Client Exit...\n");
			break;
		}
	}
}

void ssl_info_callback(const SSL *ssl, int where, int ret) {
    if (where & SSL_CB_HANDSHAKE_START) {
        printf("SSL/TLS handshake started");
        
        const SSL_CIPHER *cipher = SSL_get_current_cipher(ssl);
        printf("Cipher suite being negotiated: %s", SSL_CIPHER_get_name(cipher));

    }
    if (where & SSL_CB_HANDSHAKE_DONE) {
        printf("SSL/TLS handshake completed successfully");
    }
}

int create_socket(){

    int sockfd;
    struct sockaddr_in servaddr, cli;

	// socket create and verification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		printf("socket creation failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully created..\n");

    return sockfd;
}



SSL_CTX *create_context() {
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    method = DTLS_client_method();

    ctx = SSL_CTX_new(method);
    if(!ctx){
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}

void configure_context(SSL_CTX *ctx){
    /* set the key and cert*/
    const char* cipherList = "ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:TLS_AES_256_GCM_SHA384:AES256-GCM-SHA384:AES128-SHA256:AES128-SHA";

    SSL_CTX_set_info_callback(ctx, ssl_info_callback);
    if(SSL_CTX_set_cipher_list(ctx, cipherList) == 0) {
        perror("Unable to set cipher suites");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if(SSL_CTX_use_certificate_file(ctx, "bob-cert.crt", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if(SSL_CTX_use_certificate_chain_file(ctx, "ca-chain-cert.crt") <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if(SSL_CTX_use_PrivateKey_file(ctx, "bob-key.pem", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    // Check if the private key matches the certificate
    if (SSL_CTX_check_private_key(ctx) != 1) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}


int main()
{
	int sockfd, connfd;
	struct sockaddr_in servaddr, cli;

    sockfd = create_socket();
	// // socket create and verification
	// sockfd = socket(AF_INET, SOCK_STREAM, 0);
	// if (sockfd == -1) {
	// 	printf("socket creation failed...\n");
	// 	exit(0);
	// }
	// else
	// 	printf("Socket successfully created..\n");
	bzero(&servaddr, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr.sin_port = htons(PORT);

	// connect the client socket to server socket
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr))
		!= 0) {
		printf("connection with the server failed...\n");
		exit(0);
	}
	else
		printf("connected to the server..\n");

    // Making SSL connection
    SSL_CTX *ctx;
    ctx = create_context();
    configure_context(ctx);

    SSL *ssl;
    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sockfd);

    if(SSL_connect(ssl) <= 0 ){
        ERR_print_errors_fp(stderr);
        printf("Handshake failed\n");
    }
	// function for chat
	func(sockfd);

	// close the socket
	close(sockfd);
}
