// client.c

#include "common.h"
#include "network.h"

#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    char send_buf[BUFFER_SIZE];
    char recv_buf[BUFFER_SIZE];

    // ===== CONNECT TO SERVER =====
    sockfd = connect_to_server("127.0.0.1", DEFAULT_PORT);
    if (sockfd < 0) {
        perror("Connection failed");
        return 1;
    }

    printf("Connected to server\n");

    // ===== RECEIVE LOGIN PROMPT =====
    memset(recv_buf, 0, sizeof(recv_buf));
    if (recv_data(sockfd, recv_buf, sizeof(recv_buf)) > 0) {
        printf("%s", recv_buf);
    }

    // ===== AUTHENTICATION LOOP =====
    while (1) {
        printf("Enter credentials (LOGIN <username> <password>): ");

        memset(send_buf, 0, sizeof(send_buf));
        fgets(send_buf, sizeof(send_buf), stdin);
        trim_newline(send_buf);

        send_data(sockfd, send_buf);

        memset(recv_buf, 0, sizeof(recv_buf));
        if (recv_data(sockfd, recv_buf, sizeof(recv_buf)) <= 0) {
            printf("Server disconnected\n");
            close_socket(sockfd);
            return 1;
        }

        printf("%s", recv_buf);

        if (strstr(recv_buf, "SUCCESS") != NULL) {
            break;
        }
    }

    // ===== COMMAND LOOP =====
    while (1) {
        printf("\n>> ");

        memset(send_buf, 0, sizeof(send_buf));
        fgets(send_buf, sizeof(send_buf), stdin);
        trim_newline(send_buf);

        if (strlen(send_buf) == 0) continue;

        send_data(sockfd, send_buf);

        if (strcmp(send_buf, "EXIT") == 0) {
            printf("Disconnected from server\n");
            break;
        }

        memset(recv_buf, 0, sizeof(recv_buf));
        int n = recv_data(sockfd, recv_buf, sizeof(recv_buf));
        if (n <= 0) {
            printf("Server disconnected\n");
            break;
        }

        printf("%s\n", recv_buf);
    }

    close_socket(sockfd);
    return 0;
}