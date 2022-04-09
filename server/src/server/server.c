#include "server.h"

static Server *_server = NULL;

static string_t get_cb_key(const char *type, const char *route) {
    string_t key = mx_strnew(strlen(type) + strlen(route) + 1);
    strcat(key, type);
    strcat(key, " ");
    strcat(key, route);
    return key;
}

static int socket_write(socket_ctx *ctx, const void *buff, int num) {
    if (ctx->use_ssl)
        return SSL_write(ctx->ssl, buff, num);
    else
        return write(ctx->socket, buff, num);
}

static int socket_read(socket_ctx *ctx, void *buff, int num) {
    if (ctx->use_ssl)
        return SSL_read(ctx->ssl, buff, num);
    else
        return read(ctx->socket, buff, num);
}

static ssize_t write_all(socket_ctx *ctx, const void *buf, size_t len) {
    const char *p = buf;
    while (len) {
        ssize_t ret = socket_write(ctx, p, len);
        if (ret < 0)
            return -1;
        p += ret;
        len -= ret;
    }
    return 0;
}

static bool is_ssl(int socket) {
    char buff[3] = {0};
    recv(socket, buff, 3, MSG_PEEK);
    if (!strncmp(buff, SSL_PREFIX, 3))
        return true;
    return false;
}

static string_t read_start_line(socket_ctx *ctx, size_t *len) {
    size_t cap = 100;
    char *buffer = calloc(cap, sizeof(char));
    size_t actual_size = 0;
    actual_size += socket_read(ctx, buffer, 2);
    if (actual_size < 2) {
        free(buffer);
        return NULL;
    }
    while (strncmp(buffer + actual_size - 2, "\r\n", 2) != 0) {
        int read_count = socket_read(ctx, buffer + actual_size, 1);
        if (read_count <= 0)
            break;
        actual_size += read_count;
        if (actual_size == cap) {
            cap *= 2;
            buffer = realloc(buffer, cap);
        }
    }
    buffer[actual_size] = 0;
    if (len)
        *len = actual_size;
    return buffer;
}

static string_t read_headers(socket_ctx *ctx, size_t *len) {
    size_t cap = 100;
    char *buffer = calloc(cap, sizeof(char));
    size_t actual_size = 0;
    actual_size += socket_read(ctx, buffer, 4);
    if (actual_size < 4) {
        free(buffer);
        return NULL;
    }
    while (strncmp(buffer + actual_size - 4, "\r\n\r\n", 4) != 0) {
        int read_count = socket_read(ctx, buffer + actual_size, 1);
        if (read_count <= 0)
            break;
        actual_size += read_count;
        if (actual_size == cap) {
            cap *= 2;
            buffer = realloc(buffer, cap);
        }
    }
    buffer[actual_size] = 0;
    if (len)
        *len = actual_size;
    return buffer;
}

static string_t read_all(socket_ctx *ctx, size_t *len) {
    size_t cap = 1024;
    char *buffer = calloc(cap, sizeof(char));
    if (recv(ctx->socket, buffer, 1, MSG_PEEK | MSG_DONTWAIT) <= 0) {
        free(buffer);
        return mx_strdup("");
    }
    size_t actual_size = 0;
    int read_count = 0;
    while ((read_count = socket_read(ctx, buffer + actual_size,
                                     cap - actual_size)) > 0) {
        actual_size += read_count;
        if (actual_size == cap) {
            cap *= 2;
            buffer = realloc(buffer, cap);
        }
    }
    if (len)
        *len = actual_size;
    return buffer;
}

static string_t read_body(socket_ctx *ctx, size_t estimate_len, size_t *len) {
    if (estimate_len == 0) {
        if (len)
            *len = 0;
        return mx_strdup("");
    }
    size_t cap = estimate_len;
    char *buffer = calloc(cap, sizeof(char));
    size_t actual_size = 0;
    int read_count = 0;
    while ((read_count = socket_read(ctx, buffer + actual_size,
                                     cap - actual_size)) >= 0) {
        actual_size += read_count;
        if (actual_size == estimate_len)
            break;
    }
    if (len)
        *len = actual_size;
    return buffer;
}

static void socket_close(socket_ctx *ctx) {
    if (ctx->use_ssl) {
        SSL_shutdown(ctx->ssl);
    } else {
        shutdown(ctx->socket, SHUT_RDWR);
    }
    SSL_free(ctx->ssl);
    close(ctx->socket);
}

static void *async_socket_exit(socket_ctx *ctx) {
    socket_close(ctx);
    free(ctx);
    pthread_exit(NULL);
    return NULL;
}

static void *destroy_request(Request *req) {
    if (!req)
        return NULL;
    if (req->type)
        free(req->type);
    if (req->location)
        free(req->location);
    if (req->query)
        dict_destroy(req->query, free);
    if (req->raw_query_args)
        free(req->raw_query_args);
    if (req->body)
        free(req->body);
    if (req->headers)
        free(req->headers);
    if (req->raw_start_line)
        free(req->raw_start_line);
    if (req->raw_headers)
        free(req->raw_headers);
    free(req);
    return NULL;
}

static Request *create_request(socket_ctx *ctx) {
    Request *req = (Request *)malloc(sizeof(Request));
    req->ctx = ctx;
    req->type = NULL;
    req->location = NULL;
    req->body_len = 0;
    req->body = NULL;
    req->headers = NULL;
    req->query = NULL;
    req->raw_query_args = NULL;
    req->raw_headers = NULL;
    req->raw_start_line = NULL;
    return req;
}

static void *destroy_response(Response *res) {
    if (!res)
        return NULL;
    if (res->headers)
        free(res->headers);
    if (res->body)
        free(res->body);
    free(res);
    return NULL;
}

static Response *create_response(socket_ctx *ctx) {
    Response *res = (Response *)malloc(sizeof(Response));
    res->ctx = ctx;
    res->status = RESPONSE_OK;
    res->headers = create_headers();
    res->body = NULL;
    res->body_len = 0;
    return res;
}

static void percent_decode(char *out, const char *in) {
    static const char tbl[256] = {
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -1, -1, -1, -1, -1, -1,
        -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
    char c, v1, v2;
    if (in != NULL) {
        while ((c = *in++) != '\0') {
            if (c == '%') {
                if ((v1 = tbl[(unsigned char)*in++]) < 0 ||
                    (v2 = tbl[(unsigned char)*in++]) < 0) {
                    c = '%';
                    (void)((unsigned char)*in--);
                } else {
                    c = (v1 << 4) | v2;
                }
            }
            *out++ = c;
        }
    }
    *out = '\0';
}

static Dict *parse_query(const char *query_str) {
    Dict *q = create_dict();
    if (!query_str)
        return q;
    string_t *options = mx_strsplit(query_str, '&');
    while (*options) {
        string_t *parts = mx_strdivide(*options, '=');
        dict_set(q, parts[0], mx_strdup(parts[1]));
        mx_del_strarr(&parts);
        options++;
    }
    mx_del_strarr(&options);
    return q;
}

static Request *read_request(socket_ctx *ctx, Response *res, int *skip) {
    size_t start_line_len, headers_len;
    string_t tmp;
    Request *req = create_request(ctx);

    if ((tmp = read_start_line(ctx, &start_line_len)) == NULL)
        return destroy_request(req);
    req->raw_start_line = mx_strdup(tmp);
    free(tmp);
    string_t *start_line_args = mx_strsplit(req->raw_start_line, ' ');
    if (!start_line_args[0] || !start_line_args[1])
        return destroy_request(req);
    req->type = mx_strdup(start_line_args[0]);

    tmp = mx_strnew(strlen(start_line_args[1]));
    percent_decode(tmp, start_line_args[1]);
    string_t *path_parts = mx_strdivide(tmp, '?');
    free(tmp);
    req->location = mx_strdup(path_parts[0]);
    req->raw_query_args = mx_strdup(path_parts[1]);
    req->query = parse_query(req->raw_query_args);
    mx_del_strarr(&start_line_args);
    mx_del_strarr(&path_parts);

    if ((tmp = read_headers(ctx, &headers_len)) == NULL)
        return destroy_request(req);
    req->raw_headers = mx_strdup(tmp);
    free(tmp);

    int h_err;
    req->headers = parse_headers(req->raw_headers, headers_len, &h_err);
    if (h_err)
        return destroy_request(req);

    char *content_len_str = get_header(req->headers, "Content-Length");
    if (content_len_str == NULL) {
        req->body = read_all(ctx, &req->body_len);
    } else {
        size_t content_len;
        sscanf(content_len_str, "%zu", &content_len);
        if (content_len > 524288000) {
            *skip = 1;
            res->status = RESPONSE_413;
            res->body = strdup("{\"code\"413,\"message\": \"Request Entity Too Large\"}");
            res->body_len = strlen(res->body);
            set_header(res->headers, HEADER_CONTENT_TYPE, CONTENT_TYPE_application_json);
            return req;
        }
        req->body = read_body(ctx, content_len, &req->body_len);
        if (content_len != req->body_len) {
            // invalid body len
            return destroy_request(req);
        }
    }

    return req;
}

bool is_socket_dead(int sock_fd) {
    char t;
    return !recv(sock_fd, &t, 1, MSG_PEEK | MSG_DONTWAIT);
}

static size_t respond(Response *res) {
    socket_ctx *ctx = res->ctx;
    size_t wrote_count = 0;
    string_t headers_str;
    char buff[30];
    memset(buff, 0, 30);
    sprintf(buff, "%zu", res->body_len);
    set_header(res->headers, HEADER_CONTENT_LENGTH, buff);
    headers_str = headers_2_str(res->headers);

    wrote_count += write_all(ctx, "HTTP/1.1 ", 9);
    wrote_count += write_all(ctx, res->status, strlen(res->status));
    wrote_count += write_all(ctx, "\r\n", 2);
    wrote_count += write_all(ctx, headers_str, strlen(headers_str));
    wrote_count += write_all(ctx, res->body, res->body_len);

    free(headers_str);
    return wrote_count;
}

static api_callback get_handler(const char *type, const char *route) {
    string_t key = get_cb_key(type, route);
    api_callback cb = API_CB(dict_get(_server->_handlers, key));
    free(key);
    return cb;
}

static void *async_accept(void *_ctx) {
    socket_ctx *ctx = (socket_ctx *)_ctx;
    ctx->use_ssl = is_ssl(ctx->socket);
    int i;

    if (ctx->use_ssl) {
        ctx->ssl = SSL_new(ctx->server->_ssl_ctx);
        SSL_set_fd(ctx->ssl, ctx->socket);
        int ssl_accept_result = SSL_accept(ctx->ssl);
        if (ssl_accept_result <= 0) {
            ERR_print_errors_fp(stderr);
            return async_socket_exit(ctx);
        }
    }

    int skip = 0;
    Response *res = create_response(ctx);
    Request *req = read_request(ctx, res, &skip);
    if (req == NULL) {
        return async_socket_exit(ctx);
    }

    any req_ctx =
        _server->ctx_creator ? _server->ctx_creator(req, res, _server) : NULL;

    if (!skip)
        for (i = 0; i < _server->_before_middlewares_count; i++)
            if (_server->_before_middlewares[i](req, res, req_ctx, _server)) {
                skip = 1;
                break;
            }

    if (!skip) {
        api_callback cb = get_handler(req->type, req->location);
        if (!cb)
            cb = get_handler("*", req->location);
        if (!cb)
            cb = get_handler(req->type, "*");
        if (!cb)
            cb = get_handler("*", "*");

        if (cb) {
            if (cb(req, res, req_ctx, _server))
                skip = 1;
        } else {
            res->status = RESPONSE_Not_Found;
            skip = 1;
        }
    }

    if (!skip)
        for (i = 0; i < _server->_after_middlewares_count; i++)
            _server->_after_middlewares[i](req, res, req_ctx, _server);

    respond(res);

    if (_server->ctx_destroyer)
        _server->ctx_destroyer(req_ctx);
    destroy_request(req);
    destroy_response(res);
    return async_socket_exit(ctx);
}

static int stop() {
    _server->_running = 0;
    close(_server->_socket);
    return 0;
}

static int start() {
    if (listen(_server->_socket, SOMAXCONN) < 0) {
        perror("Failed to start listening...\n");
        return 1;
    }
    int address_length = sizeof(_server->_address);
    while (_server->_running) {
        int new_client =
            accept(_server->_socket, (struct sockaddr *)&_server->_address, (socklen_t *)&address_length);

        if (!_server || !_server->_running)
            break;
        if (new_client < 0) {
            if (errno == ECONNABORTED)
                break;
            else if (errno == EPROTO || errno == ENOPROTOOPT ||
                     errno == ENETDOWN || errno == EHOSTUNREACH ||
                     errno == EOPNOTSUPP ||
#if __linux__
                     errno == ENONET ||
#endif
                     errno == ENETUNREACH || errno == EHOSTDOWN ||
                     errno == EWOULDBLOCK || errno == EAGAIN)
                continue;
            else {
                perror("listen");
                _server->stop();
                return 1;
            }
        }
        socket_ctx *ctx = (socket_ctx *)malloc(sizeof(socket_ctx));
        ctx->server = _server;
        ctx->socket = new_client;
        ctx->ssl = NULL;
        ctx->use_ssl = false;
        pthread_t response_thread;
        pthread_create(&response_thread, NULL, async_accept, (void *)ctx);
    }
    return 0;
}

Server *init_server(int port, any app_ctx) {
    if (_server)
        return _server;

    SSL_library_init();

    _server = (Server *)malloc(sizeof(Server));
    _server->start = start;
    _server->stop = stop;
    _server->app_ctx = app_ctx;
    _server->ctx_creator = NULL;
    _server->_running = true;
    _server->_port = port;
    _server->_timeout.tv_sec = 5;
    _server->_timeout.tv_usec = 0;
    _server->_address.sin_family = PF_INET;
    _server->_address.sin_addr.s_addr = INADDR_ANY;
    _server->_address.sin_port = htons(port);
    _server->_socket = socket(PF_INET, SOCK_STREAM, 0);
    _server->_ssl_ctx = init_ssl_ctx("server.cert", "server.key");
    _server->_handlers = create_dict();
    _server->_after_middlewares_count = 0;
    _server->_before_middlewares_count = 0;
    memset(_server->_before_middlewares, 0, MAX_MIDDLEWARE_NUMBER);
    memset(_server->_after_middlewares, 0, MAX_MIDDLEWARE_NUMBER);
    char *host = getenv("HOST");
    if (!host) {
        string_t s_port = mx_itoa(port);
        _server->host = mx_strjoin("http://localhost:", s_port);
        mx_strdel(&s_port);
    } else {
        _server->host = mx_strdup(host);
    }

    int reuse = 1;
    int alive = 1;
    if (setsockopt(_server->_socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse, sizeof(reuse)) < 0) {
        perror("Failed to reuse addr...\n");
        exit(1);
    }
    if (setsockopt(_server->_socket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&_server->_timeout, sizeof(_server->_timeout)) < 0) {
        perror("Failed to set request timeout...\n");
        exit(1);
    }
    if (setsockopt(_server->_socket, SOL_SOCKET, SO_KEEPALIVE, (const char *)&alive, sizeof(alive))) {
        perror("Failed to set keep alive...\n");
        exit(1);
    }
    // if (setsockopt(_server.socket, SOL_SOCKET, SO_SNDTIMEO, (const
    // char*)&_server.timeout, sizeof(_server.timeout)) < 0) {
    //     perror("Failed to set send timeout...\n");
    //     exit(1);
    // }
    if (_server->_socket == 0) {
        perror("Failed to connect socket...\n");
        exit(1);
    }
    if (bind(_server->_socket, (struct sockaddr *)&_server->_address,
             sizeof(_server->_address)) < 0) {
        perror("Failed to bind socket...\n");
        exit(1);
    }
    signal(SIGPIPE, SIG_IGN);
    return _server;
}

void destroy_server() {
    if (!_server)
        return;
    SSL_CTX_free(_server->_ssl_ctx);
    dict_destroy(_server->_handlers, free);
    free(_server->host);
    free(_server);
    _server = NULL;
}

void print_request(Request *req) {
    mx_printstr("{\n");
    mx_printstr("\ttype: \"");
    mx_printstr(req->type);
    mx_printstr("\",\n");
    mx_printstr("\tssl: ");
    mx_printstr(req->ctx->use_ssl ? "true" : "false");
    mx_printstr(",\n");
    mx_printstr("\tlocation: \"");
    mx_printstr(req->location);
    mx_printstr("\",\n");
    mx_printstr("\tquery_args: ");
    if (req->raw_query_args) {
        mx_printstr("\"");
        mx_printstr(req->raw_query_args);
        mx_printstr("\"");
    } else {
        mx_printstr("null");
    }
    mx_printstr(",\n");
    mx_printstr("\theaders: {\n");
    for (int i = 0; i < req->headers->len; i++) {
        mx_printstr("\t\t\"");
        mx_printstr(req->headers->headers[i].name);
        mx_printstr("\": \"");
        mx_printstr(req->headers->headers[i].value);
        mx_printstr("\",\n");
    }
    mx_printstr("\t}\n");
    mx_printstr("}\n");
}

int add_before_middleware(api_callback cb) {
    if (!_server)
        return -1;
    if (_server->_before_middlewares_count >= MAX_MIDDLEWARE_NUMBER || !cb)
        return 0;
    _server->_before_middlewares[_server->_before_middlewares_count++] = cb;
    return 1;
}

int add_after_middleware(api_callback cb) {
    if (!_server)
        return -1;
    if (_server->_after_middlewares_count >= MAX_MIDDLEWARE_NUMBER || !cb)
        return 0;
    _server->_after_middlewares[_server->_after_middlewares_count++] = cb;
    return 1;
}

int add_handler(const char *type, const char *route, api_callback cb) {
    if (!_server)
        return -1;
    string_t key = get_cb_key(type, route);
    dict_set(_server->_handlers, key, (void *)cb);
    free(key);
    return 1;
}
