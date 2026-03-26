# SOCKS5 proxy server

A SOCKS5 proxy server implemented in C. It accepts SOCKS5 client connections on port 1080 and tunnels TCP traffic to any IPv4 address or domain name the client requests.

## Project Structure

```
.
├── server_files/           # Main server implementation
│   ├── main.c              # Entry point — creates socket, binds to port 1080
│   ├── CMakeLists.txt      # CMake build configuration
│   ├── Dockerfile          # Docker build and run configuration
│   └── server/
│       ├── socks.h         # SOCKS5 protocol constants and packed structs
│       ├── handlers.c/h    # SOCKS5 greeting, request parsing, and tunnel relay
│       ├── server.c/h      # Accept loop and per-client worker threads
│       └── utils.c/h       # Low-level socket helpers
└── aux/
    └── client.c            # Standalone TCP echo client for testing
```

## Code Walkthrough

### `server_files/main.c` — Entry Point

Creates a TCP socket, sets `SO_REUSEADDR` so the port can be rebound quickly after a restart, binds to `0.0.0.0:1080`, and calls `server_loop()`.

```c
server_fd = socket(AF_INET, SOCK_STREAM, 0);
setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
bind(server_fd, &bind_addr, sizeof(bind_addr));
listen(server_fd, SOMAXCONN);
server_loop(server_fd);
```

### `server_files/server/server.c` — Accept Loop & Threading

`server_loop()` runs forever: it calls `accept()` to wait for a new client, enables `TCP_NODELAY` on the accepted socket (disables Nagle's algorithm for lower latency), then spawns a detached `pthread` that runs `client_worker()`.

`client_worker()` drives the full SOCKS5 session for one client in three steps:

```c
handler_greeting(client_fd);   // 1. Negotiate auth method
remote_fd = handle_request(client_fd); // 2. Connect to destination
start_tunnel(client_fd, remote_fd);    // 3. Relay data
```

Each client runs in its own thread so the main thread never blocks.

### `server_files/server/handlers.c` — SOCKS5 Protocol Logic

**`handler_greeting(client_fd)`**

Reads the client's method-selection message and scans the offered authentication methods. If `NO_AUTH` (0x00) is present, the server confirms it; otherwise it sends `NO_ACCEPTABLE_METHODS` (0xFF) and closes the connection.

**`handle_request(client_fd)`**

Reads the SOCKS5 request header and dispatches on the address type:

- **IPv4 (`ATYP = 0x01`)** — reads the 4-byte address and 2-byte port, creates a socket, and calls `connect()` directly.
- **Domain name (`ATYP = 0x03`)** — reads the length-prefixed hostname, resolves it with `getaddrinfo()`, and tries each returned address until one connects.

On success it sends a SOCKS5 reply (success code + bound address) and returns the `remote_fd`. On failure it sends an error reply and returns `-1`.

**`start_tunnel(client_fd, remote_fd)`**

Enters a loop that uses `select()` to wait for data on either socket. When data arrives from the client it is forwarded to the remote server, and vice versa. The loop exits when either side closes the connection.

```
client ──► select() ──► remote
client ◄── select() ◄── remote
```

### `server_files/server/utils.c` — Socket Helpers

**`recv_exact(fd, buffer, size, flags)`** — Loops on `recv()` until exactly `size` bytes have been received, correctly handling short reads and `EINTR`/`EAGAIN` interruptions.

**`recv_string(fd, str)`** — Reads a SOCKS5 length-prefixed string: first a 1-byte length, then that many bytes of payload. Used for domain names in SOCKS5 requests.

### `server_files/server/socks.h` — Protocol Definitions

Defines all SOCKS5 constants from [RFC 1928](https://datatracker.ietf.org/doc/html/rfc1928) and four `__attribute__((packed))` structs that map directly onto the wire format:

| Struct | Wire bytes | Purpose |
|---|---|---|
| `identidier_selection_msg_t` | `VER NMETHODS` | Client → Server: method negotiation header |
| `method_selection_msg_t` | `VER METHOD` | Server → Client: chosen method |
| `socks_request_t` | `VER CMD RSV ATYP` | Client → Server: connection request header |
| `socks_reply_t` | `VER REP RSV ATYP` | Server → Client: connection reply header |

### `aux/client.c` — Echo Test Client

A minimal TCP client used for testing. Connects to a server, sends a string, receives the echoed response, and prints it.

```
Usage: ./client <server_ip> <message> [<port>]
```

Defaults to port 7 (the standard echo port) if no port is given.

## SOCKS5 Protocol Flow

```
Client                        Proxy (this server)              Remote
  |                                   |                           |
  |--- GREETING (VER + NMETHODS) ---->|                           |
  |<-- METHOD SELECTION (NO_AUTH) ----|                           |
  |                                   |                           |
  |--- REQUEST (CMD=CONNECT, ATYP,    |                           |
  |            DST.ADDR, DST.PORT) -->|                           |
  |                                   |--- TCP connect() -------->|
  |<-- REPLY (SUCCEEDED, BND.ADDR,    |<-- connection accepted ---|
  |           BND.PORT) --------------|                           |
  |                                   |                           |
  |<=== bidirectional tunnel (select loop) ====================>  |
```

## Build & Run

### CMake (recommended)

```bash
cd server_files
cmake -B build
cmake --build build
./build/server_files
```

### GCC directly

```bash
cd server_files
gcc -std=c11 main.c server/server.c server/handlers.c server/utils.c -lpthread -o socks_server
./socks_server
```

### Docker

```bash
cd server_files
docker build -t friendly-parakeet .
docker run -p 1080:1080 friendly-parakeet
```

The server listens on port **1080** in all cases.

## Testing

Configure any SOCKS5-capable application (e.g. `curl`) to use `127.0.0.1:1080` as a proxy:

```bash
curl --socks5 127.0.0.1:1080 http://example.com
```

To test the standalone echo client in `aux/`:

```bash
cd aux
gcc client.c -o client
./client 127.0.0.1 "hello" 7
```
