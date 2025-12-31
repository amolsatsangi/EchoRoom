# EchoRoom

A lightweight multi-client chat application in C++ using Boost.Asio. Demonstrates framed binary protocol, asynchronous server design, and clean CLI interface.

---

## Features

- Multi-client TCP chat with length-prefixed message protocol
- Asynchronous server with safe write ordering
- Simple blocking client with command support
- Graceful disconnect handling

---

## Message Protocol

Fixed 4-byte header + variable body (max 512 bytes):
```
+----------+-------------------+
| Header | Body |
| (4 bytes)| (N bytes) |
+----------+-------------------+
```

**Example:** `"0005Hello"` → Header: `0005`, Body: `Hello`

---

## Architecture
```
Clients (CLI)
|
| TCP (framed messages)
|
Server (Boost.Asio)
```

**Server** (Boost.Asio)
- Accepts multiple connections
- Reads/broadcasts framed messages
- Ensures write safety via per-client queue + `writeInProgress` flag

**Client** (CLI)
- Reader thread: receives messages
- Main thread: sends input
- Commands: `/exit`, `/help` (local only)

---

## Building

**Requirements:** C++17, Boost.Asio, pthread
```bash
# Server
g++ -std=c++17 chatRoom.cpp -o server -lboost_system -pthread

# Client
g++ -std=c++17 client.cpp -o client -lboost_system -pthread
```
Run:
```
./server 9099
```
```
./client 9099 # Launch multiple instances
```  

Design Focus: Correctness and clarity over features.
Protocol violations (invalid headers or oversized messages) result in immediate connection termination to maintain stream integrity.
