# 🗨️ ChatRoom — Asynchronous C++ Chat Application (Boost.Asio)

A minimal asynchronous **TCP chat server and client** built using **C++17** and **Boost.Asio**.  
Supports multiple clients, message broadcasting, and a lightweight message protocol.

---

## 🚀 Features

- Multi-client chat server  
- Fully asynchronous I/O (`async_read_until`, `async_write`)  
- Custom header+body message format  
- Clean class architecture (Room, Session, Participant)  
- Simple threaded client for interactive messaging  

---

## 🧠 Architecture Overview

### 🟦 Participant (Interface)
Defines functions every chat participant must support:
```cpp
virtual void deliver(Message&) = 0;
virtual void write(Message&) = 0;
```
🟧 Room

Manages all connected clients:

join() / leave()

Broadcasts messages from one session to others

🟩 Session

Represents a single connected client:

Starts reading with async_read()

Sends incoming messages to the Room

Sends outgoing messages back to client with async_write()

Uses shared_from_this() for safe async lifetime

✉️ Message

Simple protocol:
```
[4-byte header][body]
```
Header stores the body length (ASCII). Max body size = 512 bytes.

📦 Build Instructions
Install Boost (Ubuntu / WSL)
```
sudo apt update
sudo apt install libboost-all-dev
```
Build Server
```
g++ -std=c++17 chatRoom.cpp -o server -lboost_system -pthread
```
Build Client
```
g++ -std=c++17 client.cpp -o client -lboost_system -pthread
```
▶️ Running
Start Server
```
./server 8080
```
Start Client(s)
```
./client 8080
```

Open multiple terminals for multiple clients.

🔄 Message Flow
```
Client → Session::async_read()
       → Session::deliver()
       → Room::deliver()
       → All other Sessions::write()
       → async_write() → Clients
```
