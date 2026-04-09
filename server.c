// server.c

#include "common.h"
#include "network.h"
#include "command.h"
#include "auth.h"
#include "logger.h"
#include "config.h"
#include "simulation.h"
#include "db.h"
#include <signal.h>

#define _POSIX_C_SOURCE 200809L
#define MAX_CLIENTS 100
#define BUFFER_SIZE 1024

typedef struct {
    int client_fd;
    int client_id;
    char role[20];
    char client_ip[INET_ADDRSTRLEN];
    int client_port;
} client_ctx_t;

static int client_counter = 0;
pthread_mutex_t client_lock = PTHREAD_MUTEX_INITIALIZER;
static volatile sig_atomic_t shutdown_requested = 0;
static int server_socket_fd = -1;

static void request_shutdown(int signum) {
    (void)signum;
    shutdown_requested = 1;
    if (server_socket_fd >= 0)
        close(server_socket_fd);
}

static void install_signal_handlers(void) {
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = request_shutdown;
    sigemptyset(&sa.sa_mask);

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
}

static void log_client_disconnect(const client_ctx_t* ctx, const char* reason) {
    char message[128];

    snprintf(message, sizeof(message),
             "Client %d disconnected: %s:%d (%s)",
             ctx->client_id,
             ctx->client_ip,
             ctx->client_port,
             reason);

    printf("%s\n", message);
    log_info(message);
}

/* ================= CLIENT HANDLER ================= */

void* handle_client(void* arg) {
    client_ctx_t* ctx = (client_ctx_t*)arg;
    int fd = ctx->client_fd;
    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    log_event("INFO", "Client connected");

    // ===== AUTHENTICATION =====
    send_data(fd, "LOGIN <username> <password>\n");

    int bytes = recv_data(fd, buffer, sizeof(buffer));
    if (bytes <= 0) {
        log_client_disconnect(ctx, "disconnected before authentication");
        close(fd);
        free(ctx);
        pthread_exit(NULL);
    }

    buffer[strcspn(buffer, "\n")] = '\0';

    if (authenticate_user(buffer, ctx->role) != 0) {
        send_data(fd, "AUTH FAILED\n");
        log_event("WARN", "Authentication failed");
        log_client_disconnect(ctx, "authentication failed");
        close(fd);
        free(ctx);
        pthread_exit(NULL);
    }

    send_data(fd, "AUTH SUCCESS\n");
    log_event("INFO", "Authentication success");

    // ===== COMMAND LOOP =====
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        memset(response, 0, sizeof(response));

        int n = recv_data(fd, buffer, sizeof(buffer));
        if (n <= 0) {
            log_client_disconnect(ctx, "connection closed");
            break;
        }

        buffer[strcspn(buffer, "\n")] = '\0';

        if (strcmp(buffer, "EXIT") == 0) {
            send_data(fd, "Goodbye\n");
            log_client_disconnect(ctx, "client requested exit");
            break;
        }

        // Process command
        process_command(buffer, response, ctx->role);

        // Send response
        send_data(fd, response);

        // Log command
        log_event("INFO", buffer);
    }

    close(fd);
    free(ctx);
    pthread_exit(NULL);
}

/* ================= MAIN SERVER ================= */

int main() {
    install_signal_handlers();

    // ===== LOAD CONFIG =====
    if (load_config("config.txt") != 0) {
        printf("Failed to load config\n");
        return 1;
    }

    // ===== INIT LOGGER =====
    init_logger("server.log");

    // ===== INIT FOLLOWUP SYSTEM =====
    init_followup_system();

    // ===== LOAD DATABASE =====
    load_database();

    // ===== INIT AUTH SYSTEM =====
    if (init_auth_system() != SUCCESS) {
        printf("Failed to initialize authentication system\n");
        return 1;
    }

    // ===== START SIMULATION THREADS =====
    start_simulation();

    // ===== SOCKET SETUP =====
    int server_fd = create_server_socket(get_config_port());
    if (server_fd < 0) {
        perror("socket creation failed");
        return 1;
    }
    server_socket_fd = server_fd;

    log_event("INFO", "Server started");

    while (!shutdown_requested) {
        struct sockaddr_in client_addr;

        int client_fd = accept_connection(server_fd, &client_addr);
        if (client_fd < 0) {
            if (shutdown_requested)
                break;
            perror("accept failed");
            continue;
        }

        // Create client context
        client_ctx_t* ctx = malloc(sizeof(client_ctx_t));
        ctx->client_fd = client_fd;

        pthread_mutex_lock(&client_lock);
        ctx->client_id = ++client_counter;
        pthread_mutex_unlock(&client_lock);

        inet_ntop(AF_INET, &(client_addr.sin_addr), ctx->client_ip, sizeof(ctx->client_ip));
        ctx->client_port = ntohs(client_addr.sin_port);

        // Create thread for client
        pthread_t tid;
        pthread_create(&tid, NULL, handle_client, ctx);
        pthread_detach(tid);
    }

    printf("Shutting down server...\n");
    log_info("Server shutdown requested");

    if (server_socket_fd >= 0) {
        close(server_socket_fd);
        server_socket_fd = -1;
    }

    stop_simulation();
    close_database();
    cleanup_auth_system();
    cleanup_followup_system();
    close_logger();

    return 0;
}
