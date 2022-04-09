#include "crypto/crypto.h"
#include "db/db.h"
#include "email/email.h"
#include "handlers/root.h"
#include "middlewares/middlewares.h"
#include "server/server.h"
#include "uchat_server.h"

void demonize() {
    pid_t pid = fork();
    if (pid < 0) {
        perror("Cild process creation failed");
        exit(1);
    }
    if (pid > 0) {
        printf("Deamon started\nUse 'kill %d' to stop.\n", pid);
        exit(0);
    }
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
}

int main(int argc, const char **argv) {
    mx_redirect_script_home((char *)argv[0]);
    int port = 9000;
    short demon = true;
    if (argc >= 2 && !mx_streq(argv[1], "-")) {
        int new_port = atoi(argv[1]);
        if (new_port > 0)
            port = new_port;
        else {
            mx_printstr("Invalid Port \"");
            mx_printstr(argv[1]);
            mx_printstr("\".\n");
        }
    }
    if (argc >= 3 && mx_streq(argv[2], "--no-demon"))
        demon = false;

    if (demon)
        demonize();

    mx_printstr("Starting at port: ");
    mx_printint(port);
    mx_printchar('\n');

    int db_ping_return;
    init_db();
    if ((db_ping_return = db_ping()) != 0)
        return db_ping_return;

    init_poller();

    Server *server = init_server(port, NULL);

    mx_printstr("Running at ");
    mx_printstr(server->host);
    mx_printchar('\n');

    server->ctx_creator = CTX_CONSTRUCTOR(init_ctx);
    server->ctx_destroyer = CTX_DESTRUCTOR(destroy_ctx);

    apply_middlewares();

    attach_handlers();

    notify_server_started();

    server->start();

    destroy_server();
    destroy_db();
    destroy_poller();
    return 0;
}
