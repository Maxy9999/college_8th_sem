// network.h

#ifndef NETWORK_H
#define NETWORK_H

#include "common.h"

/* ================= SOCKET CONFIG ================= */

#define DEFAULT_PORT 8080
#define BACKLOG 10   // pending connections queue

/* ================= SERVER FUNCTIONS ================= */

// Create, bind and listen on a server socket
int create_server_socket(int port);

// Accept incoming client connection
int accept_connection(int server_fd, struct sockaddr_in* client_addr);

/* ================= CLIENT FUNCTIONS ================= */

// Create client socket and connect to server
int connect_to_server(const char* ip, int port);

/* ================= DATA TRANSFER ================= */

// Send data safely over socket
int send_data(int sockfd, const char* data);

// Receive data safely from socket
int recv_data(int sockfd, char* buffer, size_t size);

/* ================= UTILITY ================= */

// Close socket safely
void close_socket(int sockfd);

// Print client address info (optional debug)
void print_client_info(struct sockaddr_in* client_addr);

#endif