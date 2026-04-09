// network.c

#include "network.h"
#include <errno.h>

/* ================= SERVER SOCKET ================= */

int create_server_socket(int port) {
    int server_fd;
    struct sockaddr_in server_addr;

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return -1;
    }

    // Reuse address
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Setup address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // Bind
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(server_fd);
        return -1;
    }

    // Listen
    if (listen(server_fd, BACKLOG) < 0) {
        perror("listen");
        close(server_fd);
        return -1;
    }

    printf("Server listening on port %d\n", port);

    return server_fd;
}

int accept_connection(int server_fd, struct sockaddr_in* client_addr) {
    socklen_t len = sizeof(*client_addr);

    int client_fd = accept(server_fd, (struct sockaddr*)client_addr, &len);
    if (client_fd < 0) {
        if (errno != EINTR && errno != EBADF)
            perror("accept");
        return -1;
    }

    print_client_info(client_addr);

    return client_fd;
}

/* ================= CLIENT ================= */

int connect_to_server(const char* ip, int port) {
    int sockfd;
    struct sockaddr_in server_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sockfd);
        return -1;
    }

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

/* ================= DATA TRANSFER ================= */

int send_data(int sockfd, const char* data) {
    if (!data) return -1;

    size_t len = strlen(data);
    ssize_t sent = send(sockfd, data, len, 0);

    if (sent < 0) {
        perror("send");
        return -1;
    }

    return (int)sent;
}

int recv_data(int sockfd, char* buffer, size_t size) {
    if (!buffer) return -1;

    memset(buffer, 0, size);

    ssize_t received = recv(sockfd, buffer, size - 1, 0);

    if (received < 0) {
        perror("recv");
        return -1;
    } else if (received == 0) {
        // Connection closed
        return 0;
    }

    buffer[received] = '\0';
    return (int)received;
}

/* ================= UTILITY ================= */

void close_socket(int sockfd) {
    if (sockfd >= 0)
        close(sockfd);
}

void print_client_info(struct sockaddr_in* client_addr) {
    char ip[INET_ADDRSTRLEN];

    inet_ntop(AF_INET, &(client_addr->sin_addr), ip, INET_ADDRSTRLEN);
    int port = ntohs(client_addr->sin_port);

    printf("Client connected: %s:%d\n", ip, port);
}
