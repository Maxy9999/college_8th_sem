#!/usr/bin/bash

CC = gcc
CFLAGS = -Wall -Wextra -pthread -I.

# Source files
SRCS = server.c client.c followup.c simulation.c network.c db.c auth.c logger.c config.c command.c

# Object files
OBJS = $(SRCS:.c=.o)

# Executables
SERVER = server
CLIENT = client

# Default target
all: $(SERVER) $(CLIENT)

# Build server
$(SERVER): server.o followup.o simulation.o network.o db.o auth.o logger.o config.o command.o
	$(CC) $(CFLAGS) -o $(SERVER) server.o followup.o simulation.o network.o db.o auth.o logger.o config.o command.o

# Build client
$(CLIENT): client.o network.o
	$(CC) $(CFLAGS) -o $(CLIENT) client.o network.o

# Compile .c to .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -f *.o $(SERVER) $(CLIENT)

# Run server
run-server: $(SERVER)
	./$(SERVER)

# Run client
run-client: $(CLIENT)
	./$(CLIENT)