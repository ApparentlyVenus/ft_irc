# NetworkManager - Technical Reference

## Overview

**Purpose**: Pure networking layer - handles TCP/IP sockets and I/O multiplexing.

**Position**: 
```
IRCServer → NetworkManager → Operating System
```

**Does**: Socket operations, connection management, non-blocking I/O
**Doesn't**: IRC protocol, commands, users, channels

---

## Core Functions

### initialize(int port)

**What it does**: Sets up server socket to accept connections

**Steps**:
1. `socket()` - Create TCP socket
2. `setsockopt(SO_REUSEADDR)` - Allow port reuse
3. `fcntl(O_NONBLOCK)` - Set non-blocking
4. `bind()` - Claim the port
5. `listen()` - Start accepting connections
6. Add to `_pollFds` for monitoring

### pollEvents()

**What it does**: Main event loop - waits for network activity

**Steps**:
1. Clear previous events
2. `poll()` - Block until activity
3. Loop through `_pollFds`
   - Index 0: Server socket → `handleNewConnection()`
   - Index > 0: Client sockets → `handleClientEvent()`
4. `cleanupDisconnectedClients()`

**Called**: Repeatedly in main server loop

---

## System Calls

### socket(AF_INET, SOCK_STREAM, 0)
- Creates TCP/IPv4 socket
- Returns: file descriptor

### setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, size)
- Allows immediate port reuse after restart
- Prevents "Address already in use" error

### fcntl(fd, F_SETFL, O_NONBLOCK)
- Makes socket non-blocking
- Operations return immediately with EAGAIN if not ready

### bind(fd, address, size)
- Associates socket with port
- "Claims" port for this process

### listen(fd, SOMAXCONN)
- Marks socket as passive (accepting connections)
- SOMAXCONN = max queue size

### accept(fd, NULL, NULL)
- Accepts new connection
- Returns: new socket for that client
- **Must call fcntl() on returned socket**

### recv(fd, buffer, size, 0)
- Reads data from socket
- Returns:
  - `> 0`: Bytes received
  - `0`: Connection closed
  - `-1`: Error (check errno)

### send(fd, data, size, 0)
- Sends data through socket
- Returns: Bytes actually sent (may be partial)

### poll(fds, nfds, timeout)
- Monitors multiple file descriptors
- Blocks until activity on ANY fd
- Returns: Number of fds with events

### close(fd)
- Closes socket
- Releases resources

---

## Flags & Constants

### Poll Events
- `POLLIN`: Data available to read / new connection waiting
- `POLLOUT`: Socket ready for writing
- `POLLHUP`: Connection closed
- `POLLERR`: Error on socket

### Flag Operations
```cpp
events |= POLLOUT;              // Add flag
events &= ~POLLOUT;             // Remove flag
if (revents & POLLIN)           // Check flag
if (revents & (POLLHUP | POLLERR))  // Check multiple
```

### Socket Options
- `SO_REUSEADDR`: Allow address reuse
- `O_NONBLOCK`: Non-blocking mode
- `AF_INET`: IPv4
- `SOCK_STREAM`: TCP
- `INADDR_ANY`: Bind to all interfaces (0.0.0.0)

### Byte Order
- `htons(port)`: Host to Network Short - converts port to network byte order

---

## Error Codes (errno)

### EAGAIN / EWOULDBLOCK
- **Meaning**: Operation would block (normal for non-blocking I/O)
- **Action**: Try again later
```cpp
if (errno == EAGAIN) return;  // Not an error
```

### EINTR
- **Meaning**: System call interrupted by signal
- **Action**: Retry operation
```cpp
if (errno == EINTR) return;  // Just retry
```

### EADDRINUSE
- **Meaning**: Port already in use
- **Fix**: Use SO_REUSEADDR or different port

### ECONNRESET
- **Meaning**: Connection reset by peer
- **Action**: Treat as disconnect

---

## Data Structures

### sockaddr_in
```cpp
struct sockaddr_in {
    sa_family_t sin_family;     // AF_INET
    in_port_t sin_port;         // htons(port)
    struct in_addr sin_addr;    // INADDR_ANY
    char sin_zero[8];           // Padding
};
```

### pollfd
```cpp
struct pollfd {
    int fd;         // File descriptor to monitor
    short events;   // Events to watch (POLLIN, POLLOUT)
    short revents;  // Events that occurred (filled by poll)
};
```

---

## Key Concepts

### Non-Blocking I/O
**Blocking**: Wait forever until ready
**Non-Blocking**: Return immediately with EAGAIN if not ready

### Message Buffering
IRC messages end with `\r\n`. Messages may arrive in fragments:
```cpp
recv() → "NICK al"         // Partial
recv() → "ice\r\n"         // Complete
```
Solution: Accumulate in `_readBuffers[fd]` until `\r\n` found

### Write Queuing
Socket buffer might be full. Queue messages in `_writeQueues[fd]`, send when POLLOUT triggers.

### File Descriptors
- Just integers: 0, 1, 2, 3, 4...
- fd=0: stdin, fd=1: stdout, fd=2: stderr
- fd=3: Server socket
- fd=4+: Client sockets

---

## Common Pitfalls

1. **Forget fcntl() on new client sockets**
   - accept() returns BLOCKING socket
   - Must explicitly set O_NONBLOCK

2. **Treat EAGAIN as error**
   - EAGAIN is normal for non-blocking I/O
   - Check errno before disconnecting

3. **Don't buffer partial messages**
   - IRC messages need `\r\n` terminator
   - Accumulate until complete

4. **Modify _pollFds during iteration**
   - Mark for removal, cleanup after loop

5. **Don't check bind() errors**
   - Always check return values

---

## Integration with IRC Layer

```
IRCServer main loop:
  1. networkManager.pollEvents()        // Wait for events
  2. getNewClients()                    // Create User objects
  3. getCompleteMessages()              // Parse & execute commands
  4. getDisconnectedClients()           // Cleanup User objects
  5. sendMessage(fd, response)          // Send responses
  6. Repeat
```
