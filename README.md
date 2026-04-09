# Healthcare Follow-up Management System

A multi-user client-server application in C for managing patient follow-ups over TCP sockets.

The system supports:

- Role-based login for `ADMIN`, `DOCTOR`, and `RECEPTION`
- Command-based patient management from a terminal client
- In-memory data handling using a linked list, stack, and queue
- Background simulation using threads, mutexes, and semaphores
- File-based persistence for active, completed, and missed records
- Graceful shutdown with data save on `Ctrl+C`

## Features

- Multi-client server using `pthread`
- TCP socket communication between client and server
- Authentication and authorization
- Active follow-ups stored in a linked list
- Completed visits stored in a stack
- Missed visits stored in a queue
- Background patient generation and doctor simulation
- Logging to `server.log`
- Runtime configuration through `config.txt`

## Project Structure

- `server.c`: main server, client handling, graceful shutdown
- `client.c`: terminal client for login and commands
- `command.c` / `command.h`: command parsing, validation, dispatch
- `auth.c` / `auth.h`: login validation and role checks
- `followup.c` / `followup.h`: core patient operations and data structures
- `simulation.c` / `simulation.h`: background patient and doctor threads
- `network.c` / `network.h`: socket helpers
- `db.c` / `db.h`: file load/save logic
- `logger.c` / `logger.h`: thread-safe logging
- `config.c` / `config.h`: config loading and getters
- `common.h`: shared constants, structs, and utilities
- `data/users.dat`: login credentials
- `data/patients.dat`: active follow-ups, stored in binary
- `data/history.dat`: completed visits, stored in binary
- `data/missed.dat`: missed visits, stored in binary
- `config.txt`: server configuration
- `Makefile`: build and run targets

## Requirements

- Linux or Unix-like environment
- `gcc`
- `make`
- POSIX threads support

## Build

```bash
make
```

This creates:

- `server`
- `client`

To clean:

```bash
make clean
```

## Run

Start the server in one terminal:

```bash
./server
```

Start the client in another terminal:

```bash
./client
```

You can also use:

```bash
make run-server
make run-client
```

## Default Configuration

From `config.txt`:

```text
PORT=8080
MAX_CLIENTS=50
DOCTOR_THREADS=3
LOG_LEVEL=INFO
```

## Default Users

From `data/users.dat`:

```text
admin 1234 ADMIN
doc1 pass DOCTOR
rec1 pass RECEPTION
```

## Login Format

```text
LOGIN <username> <password>
```

Example:

```text
LOGIN admin 1234
```

## Supported Commands

```text
ADD <id> <name> <doctor> <day>
COMPLETE <id>
MISS <id>
HANDLE_MISSED
UNDO
SHOW
SHOW_MISSED
SHOW_HISTORY
EXIT
```

## Role Permissions

`ADMIN`

- All commands allowed

`DOCTOR`

- `COMPLETE`
- `MISS`
- `SHOW`
- `SHOW_HISTORY`

`RECEPTION`

- `ADD`
- `SHOW`
- `SHOW_MISSED`

## Typical Admin Session

```text
LOGIN admin 1234
ADD 2001 Alice DrA 3
SHOW
COMPLETE 2001
SHOW_HISTORY
ADD 2002 Bob DrB 4
MISS 2002
SHOW_MISSED
HANDLE_MISSED
UNDO
EXIT
```

## How It Works

1. The server loads config, logger, in-memory structures, database, and auth data.
2. The server starts simulation threads.
3. A client connects over TCP.
4. The client sends login credentials.
5. The server authenticates the user and stores the role for that session.
6. Every command is tokenized, validated, authorized, and executed.
7. Patient data is updated in memory:
   - linked list for active patients
   - stack for completed visits
   - queue for missed visits
8. On `Ctrl+C`, the server stops simulation threads and saves data to the `data/` files.

## Data Structures

- Linked list: active patient follow-ups
- Stack: completed visit history
- Queue: missed visits waiting to be handled

## Background Simulation

The system starts background threads that:

- generate new patients periodically
- simulate doctors completing or missing visits

This means `SHOW`, `SHOW_MISSED`, and `SHOW_HISTORY` may change over time even without manual client commands.

## Persistence

The following files are binary data files:

- `data/patients.dat`
- `data/history.dat`
- `data/missed.dat`

They are loaded when the server starts and saved when the server shuts down gracefully.

To save data correctly, stop the server with `Ctrl+C` instead of killing the process abruptly.

## Logging

Runtime events are written to:

```text
server.log
```

Examples:

- server start
- login success/failure
- command activity
- client disconnects
- shutdown and database save

## Notes

- The client always connects to `127.0.0.1`.
- The current client is terminal-based only.
- Names in `ADD` should not contain spaces because command parsing is token-based.
- Large `SHOW` outputs are safely truncated instead of overflowing the response buffer.

## Example Test Commands

Admin:

```bash
printf 'LOGIN admin 1234\nSHOW\nADD 3001 Test DrX 2\nCOMPLETE 3001\nSHOW_HISTORY\nEXIT\n' | ./client
```

Doctor permission check:

```bash
printf 'LOGIN doc1 pass\nADD 4001 Test DrY 3\nEXIT\n' | ./client
```

Reception permission check:

```bash
printf 'LOGIN rec1 pass\nCOMPLETE 4001\nEXIT\n' | ./client
```

## Known Limitations

- Data files are binary and not human-readable
- The client uses a simple CLI only
- `MAX_CLIENTS` is configurable but not strictly enforced in the current server loop
- The simulation uses randomized patient handling, so results vary between runs

## Author

Manish Agarwal
