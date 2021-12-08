
// macos, system library
// gcc -o cert-callback-poc-system cert-callback-poc.c -lssl -lcrypto
// gcc -o cert-callback-poc-system cert-callback-poc.c -L/usr/lib -lssl -lcrypto

// macos, homebrew

// debian
// gcc -o cert-callback-poc-system cert-callback-poc.c -lssl -lcrypto
// ./cert-callback-poc-system smtp.pobox.com 465
// using library version: OpenSSL 1.1.1d  10 Sep 2019
// in verify callback with dn = /C=US/O=DigiCert Inc/CN=DigiCert SHA2 Secure Server CA
// in verify callback with dn = /C=AU/ST=Victoria/L=Melbourne/O=FastMail Pty Ltd/CN=*.pobox.com


#include <stdio.h>
#include <errno.h>
#include <unistd.h>
// #include <malloc.h>
#include <string.h>
#include <sys/socket.h>
#include <resolv.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#define FAIL    -1

int OpenConnection(const char *hostname, int port)
{
    int sd;
    struct hostent *host;
    struct sockaddr_in addr;
    if ( (host = gethostbyname(hostname)) == NULL )
    {
        perror(hostname);
        abort();
    }
    sd = socket(PF_INET, SOCK_STREAM, 0);
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = *(long*)(host->h_addr);
    if ( connect(sd, (struct sockaddr*)&addr, sizeof(addr)) != 0 )
    {
        close(sd);
        perror(hostname);
        abort();
    }
    return sd;
}

int tls_verify_callback(int preverify_ok, X509_STORE_CTX* ctx) {
    X509 *x509 = X509_STORE_CTX_get_current_cert(ctx);
    char *dn   = X509_NAME_oneline(X509_get_subject_name(x509), 0, 0);
    printf("in verify callback with dn = %s\n", dn);

    return 1;
}


SSL_CTX* InitCTX(void)
{
    SSL_METHOD *method;
    SSL_CTX *ctx;
    OpenSSL_add_all_algorithms();  /* Load cryptos, et.al. */
    SSL_load_error_strings();   /* Bring in and register error messages */
    method = TLSv1_2_client_method();  /* Create new client-method instance */
    ctx = SSL_CTX_new(method);   /* Create new context */
    if ( ctx == NULL )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }

    SSL_CTX_set_default_verify_paths(ctx);

    SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, tls_verify_callback);






    return ctx;
}

// void ShowCerts(SSL* ssl)
// {
//     X509 *cert;
//     char *line;
//     cert = SSL_get_peer_certificate(ssl); /* get the server's certificate */
//     if ( cert != NULL )
//     {
//         printf("Server certificates:\n");
//         line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
//         printf("Subject: %s\n", line);
//         free(line);       /* free the malloc'ed string */
//         line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
//         printf("Issuer: %s\n", line);
//         free(line);       /* free the malloc'ed string */
//         X509_free(cert);     /* free the malloc'ed certificate copy */
//     }
//     else
//         printf("Info: No client certificates configured.\n");
// }

int main(int count, char *strings[])
{
    SSL_CTX *ctx;
    int server;
    SSL *ssl;
    char buf[1024];
    char acClientRequest[1024] = {0};
    int bytes;
    char *hostname, *portnum;
    if ( count != 3 )
    {
        printf("usage: %s <hostname> <portnum>\n", strings[0]);
        exit(0);
    }
    SSL_library_init();
    hostname=strings[1];
    portnum=strings[2];
    ctx = InitCTX();

    printf("using library version: %s\n", SSLeay_version(SSLEAY_VERSION));

    server = OpenConnection(hostname, atoi(portnum));
    ssl = SSL_new(ctx);      /* create new SSL connection state */
    SSL_set_fd(ssl, server);    /* attach the socket descriptor */
    if ( SSL_connect(ssl) == FAIL )   /* perform the connection */
        ERR_print_errors_fp(stderr);
    else
    {
        printf("\nConnected with %s encryption\n", SSL_get_cipher(ssl));
        // ShowCerts(ssl);        /* get any certs */
        SSL_free(ssl);        /* release connection state */
    }
    close(server);         /* close socket */
    SSL_CTX_free(ctx);        /* release context */
    return 0;
}
