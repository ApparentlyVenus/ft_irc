# ft_irc - Constants Reference

## Network Constants

### Socket Domain
```cpp
AF_INET          // Address Family - IPv4 Internet protocols
AF_INET6         // Address Family - IPv6 Internet protocols (not used)
```

### Socket Type
```cpp
SOCK_STREAM      // TCP - Sequenced, reliable, connection-based
SOCK_DGRAM       // UDP - Connectionless, unreliable (not used)
```

### Socket Options (setsockopt)
```cpp
SOL_SOCKET       // Socket-level options
SO_REUSEADDR     // Allow local address reuse
```

### File Control (fcntl)
```cpp
F_SETFL          // Set file status flags
O_NONBLOCK       // Non-blocking I/O mode
```

### Listen Queue
```cpp
SOMAXCONN        // Maximum pending connections queue size (system-defined, typically 128)
```

### Address Constants
```cpp
INADDR_ANY       // Bind to all network interfaces (0.0.0.0)
```

---

## Poll Event Flags

### Input Events (events field - what to monitor)
```cpp
POLLIN           // Data available to read / new connection waiting (value: 0x001)
POLLOUT          // Socket ready for writing (value: 0x004)
POLLPRI          // Priority data available (value: 0x002) (not used)
```

### Output Events (revents field - what occurred)
```cpp
POLLIN           // Data available to read (value: 0x001)
POLLOUT          // Can write without blocking (value: 0x004)
POLLHUP          // Hang up - connection closed (value: 0x010)
POLLERR          // Error condition on socket (value: 0x008)
POLLNVAL         // Invalid file descriptor (value: 0x020) (not used)
```

---

## Error Codes (errno values)

### Non-Error Conditions
```cpp
EAGAIN           // Operation would block (non-blocking mode)
EWOULDBLOCK      // Same as EAGAIN on most systems
EINTR            // System call interrupted by signal
```

### Real Errors
```cpp
EADDRINUSE       // Address already in use (bind failed)
ECONNRESET       // Connection reset by peer
EPIPE            // Broken pipe (write to closed socket)
EACCES           // Permission denied (privileged port)
EMFILE           // Too many open files
```

---

## IRC Protocol Constants

### Message Terminator
```cpp
"\r\n"           // Carriage Return + Line Feed (required by IRC spec)
```

### Message Components
```cpp
':'              // Prefix indicator (start of prefix or trailing)
' '              // Space - parameter separator
```

### Maximum Message Length
```cpp
512              // Maximum IRC message length in bytes (IRC spec)
```

---

## Buffer Sizes

### Network Buffers
```cpp
4096             // recv() buffer size in bytes (standard network buffer)
```

---

## IRC Numeric Reply Codes

### Registration Replies (001-099)
```cpp
001              // RPL_WELCOME - "Welcome to the IRC Network"
002              // RPL_YOURHOST - "Your host is..."
003              // RPL_CREATED - "This server was created..."
004              // RPL_MYINFO - Server information
```

### Channel Topic (300-399)
```cpp
331              // RPL_NOTOPIC - "No topic is set"
332              // RPL_TOPIC - Channel topic
353              // RPL_NAMREPLY - List of users in channel
366              // RPL_ENDOFNAMES - End of NAMES list
```

### Error Codes (400-599)
```cpp
401              // ERR_NOSUCHNICK - "No such nick/channel"
403              // ERR_NOSUCHCHANNEL - "No such channel"
433              // ERR_NICKNAMEINUSE - "Nickname is already in use"
441              // ERR_USERNOTINCHANNEL - User not in channel
442              // ERR_NOTONCHANNEL - You're not in channel
443              // ERR_USERONCHANNEL - User already in channel
461              // ERR_NEEDMOREPARAMS - "Not enough parameters"
462              // ERR_ALREADYREGISTRED - "You may not reregister"
482              // ERR_CHANOPRIVSNEEDED - "You're not channel operator"
```

---

## Port Range

### Valid IRC Ports
```cpp
1-65535          // Valid port range
1-1023           // Privileged ports (require root)
1024-65535       // Unprivileged ports (recommended)
6667             // Standard IRC port
6697             // Standard IRC SSL port
```

---

## File Descriptor And Signals

### Standard File Descriptors
```cpp
0                // stdin (standard input)
1                // stdout (standard output)
2                // stderr (standard error)
3                // Typically first socket (server socket)
4+               // Client sockets
```

### Signals Handled
```cpp
SIGINT           // Interrupt signal (Ctrl+C)
SIGTERM          // Termination signal
```

---

## Byte Order Conversion

### Functions (not constants, but related)
```cpp
htons()          // Host TO Network Short (16-bit port numbers)
htonl()          // Host TO Network Long (32-bit addresses)
ntohs()          // Network TO Host Short
ntohl()          // Network TO Host Long
```

**Why needed:** Different CPUs store multi-byte values differently
- x86: Little-endian (least significant byte first)
- Network: Big-endian (most significant byte first)

---

## IRC Channel Modes

### Mode Characters
```cpp
'i'              // Invite-only channel
't'              // Topic restricted to operators
'k'              // Channel key (password) required
'o'              // Operator privilege
'l'              // User limit on channel
```

---

## System Limits (informational)

### Typical Values
```cpp
1024             // Default ulimit for open file descriptors (soft limit)
4096             // Common ulimit increase for servers
128              // Typical SOMAXCONN value
```
---

**When in doubt:** Check man pages (`man 2 socket`, `man 2 poll`) or IRC RFC documents.
