#pragma once

#include <uchat_server.h>

#include <netinet/in.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <dict.h>
#include <libmx.h>

#include "HTTPTypes.h"
#include "headers.h"

#define SSL_PREFIX "\x16\x3\x1"

#define MAX_MIDDLEWARE_NUMBER 20

typedef struct Server_s Server;
typedef struct Request_s Request;
typedef struct Response_s Response;

typedef void *any;
typedef int (*api_callback)(Request *req, Response *res, any ctx, Server *server);
typedef any (*ctx_constructor)(Request *req, Response *res, Server *server);
typedef void (*ctx_destructor)(any ctx);
#define API_CB(x) ((api_callback)(x))
#define CTX_CONSTRUCTOR(x) ((ctx_constructor)(x))
#define CTX_DESTRUCTOR(x) ((ctx_destructor)(x))

struct Server_s {
    bool _running;
    int _port;
    struct sockaddr_in _address;
    int _socket;
    SSL_CTX *_ssl_ctx;
    struct timeval _timeout;
    api_callback _before_middlewares[MAX_MIDDLEWARE_NUMBER];
    int _before_middlewares_count;
    api_callback _after_middlewares[MAX_MIDDLEWARE_NUMBER];
    Dict *_handlers;
    int _after_middlewares_count;
    string_t host;
    int (*start)(void);
    int (*stop)(void);
    ctx_constructor ctx_creator;
    ctx_destructor ctx_destroyer;
    any app_ctx;
};

typedef struct socket_ctx_s {
    Server *server;
    int socket;
    SSL *ssl;
    bool use_ssl;
} socket_ctx;

struct Request_s {
    socket_ctx *ctx;
    char *type;
    char *location;
    Headers *headers;
    char *body;
    size_t body_len;
    Dict *query;

    char *raw_query_args;
    char *raw_start_line;
    char *raw_headers;
};

struct Response_s {
    socket_ctx *ctx;
    char *status; // should be static string
    Headers *headers;
    char *body;
    size_t body_len;
};

/**
 * @brief creates SSL context from cert and key filenames given
 *
 * @param cert cert filename
 * @param key cert filename
 * @return SSL_CTX*
 */
SSL_CTX *init_ssl_ctx(char *cert, char *key);

/**
 * @brief Creates and initializes Server
 *
 * @param port server port number to listen
 * @param app_ctx user`s app context be provided to middlewares and handlers
 * @return Server*
 */
Server *init_server(int port, any app_ctx);

/**
 * @brief Destroys server instance and cleanup data
 *
 */
void destroy_server();

/**
 * @brief Cheks if socket is dead
 * 
 * @param sock_fd 
 * @return bool
 */
bool is_socket_dead(int sock_fd);

/**
 * @brief Prints request in json format
 *
 * @param req request to print
 */
void print_request(Request *req);

/**
 * @brief Add middleware to run before processing
 *
 * @param cb middleware callback
 * @return int
 */
int add_before_middleware(api_callback cb);

/**
 * @brief Add middleware to run after processing
 *
 * @param cb middleware callback
 * @return int
 */
int add_after_middleware(api_callback cb);

/**
 * @brief Add request handler by type and route
 * 
 * @param type request type ("POST" or "GET" etc.)
 * @param route request route (e.g. "/users")
 * @param cb responder callback
 * @return int 
 */
int add_handler(const char *type, const char *route, api_callback cb);
