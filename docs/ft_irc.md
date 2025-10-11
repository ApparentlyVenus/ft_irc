# ft_irc - Project Architecture

## Overview

IRC server implementation in C++98 following a modular, layered architecture.

---

## Component Hierarchy

```
┌─────────────────────────────────────────────────────────────┐
│                         IRCServer                           │
│                    (Main Orchestrator)                      │
│                                                             │
│  - Owns all components                                      │
│  - Main event loop                                          │
│  - Coordinates between layers                               │
└──────────────────┬──────────────────────────────────────────┘
                   │
    ┌──────────────┼──────────────┬──────────────┬────────────┐
    │              │              │              │            │
    ▼              ▼              ▼              ▼            ▼
┌─────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐
│Network  │  │Message   │  │Command   │  │User      │  │Channel   │
│Manager  │  │Processor │  │Engine    │  │Registry  │  │Registry  │
└─────────┘  └──────────┘  └──────────┘  └──────────┘  └──────────┘
```

---

## Layers

### Network Layer
**Components:** NetworkManager

**Responsibility:** Raw TCP/IP communication
- Socket operations
- I/O multiplexing (poll)
- Connection lifecycle
- Non-blocking I/O
- Data buffering

**Interface:** Raw bytes ↔ Complete message strings

---

### Protocol Layer
**Components:** MessageProcessor, CommandEngine

**Responsibility:** IRC protocol handling
- Parse IRC message format
- Build IRC responses
- Route commands to handlers
- Execute command logic

**Interface:** Message strings ↔ IRC actions

---

### Application Layer
**Components:** UserRegistry, ChannelRegistry

**Responsibility:** IRC server state
- User management
- Channel management
- Authentication
- Permissions

**Interface:** IRC actions ↔ Server state

---

## Component Details

### IRCServer 
**Type:** Main orchestrator

**Owns:**
- NetworkManager
- MessageProcessor
- CommandEngine
- UserRegistry
- ChannelRegistry

**Main Loop:**
```
while (running)
{
    networkManager.pollEvents()
    handleNewConnections()
    handleMessages()
    handleDisconnections()
}
```

**Responsibilities:**
- Initialize all components
- Run event loop
- Coordinate component interactions
- Handle shutdown

---

### NetworkManager 
**Type:** Network abstraction layer

**Data Members:**
```cpp
int _serverSocket
std::vector<struct pollfd> _pollFds
std::map<int, std::string> _readBuffers
std::map<int, std::queue<std::string>> _writeQueues
std::vector<int> _newConnections
std::vector<int> _disconnectedClients
```

**Public Interface:**
```cpp
void initialize(int port)
void pollEvents()
void sendMessage(int fd, const std::string& message)
void removeClient(int fd)
bool isValidSocket(int fd)
std::vector<int> getNewClients()
std::vector<int> getDisconnectedClients()
std::vector<std::pair<int, std::string>> getCompleteMessages()
```

**What it knows:** File descriptors, sockets, poll events
**What it doesn't know:** IRC protocol, users, channels

---

### MessageProcessor 
**Type:** Protocol parser/formatter

**Data Structure:**
```cpp
struct IRCMessage {
    std::string prefix;              // :nick!user@host
    std::string command;             // NICK, JOIN, etc.
    std::vector<std::string> params; // Command parameters
    std::string trailing;            // Text after final ':'
};
```

**Public Interface:**
```cpp
static IRCMessage parse(const std::string& raw)
static std::string buildNumericReply(int code, const std::string& target, ...)
static std::string buildMessage(const std::string& prefix, const std::string& command, ...)
```

**Responsibilities:**
- Parse: Raw string → IRCMessage struct
- Format: Response data → IRC protocol string
- Validate message structure
- Handle IRC syntax edge cases

**What it knows:** IRC message format (RFC 1459/2812)
**What it doesn't know:** Command logic, users, channels

---

### CommandEngine
**Type:** Command dispatcher/executor

**Data Members:**
```cpp
std::map<std::string, ICommand*> _commandHandlers
UserRegistry& _userRegistry
ChannelRegistry& _channelRegistry
```

**Public Interface:**
```cpp
void registerCommand(const std::string& name, ICommand* handler)
void execute(Client* client, const IRCMessage& message)
```

**Command Handler Interface:**
```cpp
class ICommand {
public:
    virtual void execute(Client* client, const IRCMessage& msg) = 0;
    virtual bool requiresAuth() const = 0;
};
```

**Responsibilities:**
- Route commands to correct handlers
- Check authentication requirements
- Execute command logic
- Generate responses

**Commands to Implement:**
- Authentication: PASS, NICK, USER
- Channels: JOIN, PART, KICK, INVITE, TOPIC, MODE
- Messaging: PRIVMSG, NOTICE
- Info: NAMES, LIST, WHO

---

### UserRegistry 
**Type:** User state management

**Data Members:**
```cpp
std::map<int, Client*> _clientsByFd
std::map<std::string, Client*> _clientsByNick
```

**Client State:**
```cpp
enum ClientState {
    CONNECTING,
    AUTHENTICATING,
    REGISTERED
};

class Client {
    int _fd;
    std::string _nickname;
    std::string _username;
    std::string _realname;
    std::string _hostname;
    ClientState _state;
    std::set<std::string> _channels;
    bool _isOperator;
};
```

**Public Interface:**
```cpp
void addClient(int fd, Client* client)
void removeClient(int fd)
Client* getClientByFd(int fd)
Client* getClientByNick(const std::string& nick)
bool isNickAvailable(const std::string& nick)
void updateNickname(Client* client, const std::string& newNick)
```

**Responsibilities:**
- Track all connected users
- Manage authentication state
- Enforce unique nicknames
- User lookup operations

---

### ChannelRegistry 
**Type:** Channel state management

**Data Members:**
```cpp
std::map<std::string, Channel*> _channels
```

**Channel State:**
```cpp
class Channel {
    std::string _name;
    std::string _topic;
    std::string _key;
    int _userLimit;
    std::set<char> _modes;
    std::map<std::string, Client*> _members;
    std::set<std::string> _operators;
    std::set<std::string> _inviteList;
};
```

**Channel Modes:**
- `i`: Invite-only
- `t`: Topic restricted to operators
- `k`: Channel key (password)
- `o`: Operator privilege
- `l`: User limit

**Public Interface:**
```cpp
Channel* createChannel(const std::string& name, Client* creator)
Channel* getChannel(const std::string& name)
void removeChannel(const std::string& name)
void removeUserFromAll(const std::string& nickname)
```

**Responsibilities:**
- Create/destroy channels
- Manage channel membership
- Enforce channel modes
- Broadcast messages to channel members

---

## Data Flow

### Client Connection
```
1. Client connects
   ↓
2. NetworkManager.pollEvents() detects new connection
   ↓
3. IRCServer.handleNewConnections()
   ↓
4. UserRegistry.addClient(fd) → Creates Client object
   ↓
5. Client state = CONNECTING
```

### Message Processing
```
1. Client sends: "NICK alice\r\n"
   ↓
2. NetworkManager buffers → getCompleteMessages()
   ↓
3. MessageProcessor.parse() → IRCMessage{command="NICK", params=["alice"]}
   ↓
4. CommandEngine.execute(client, message)
   ↓
5. Routes to NickCommand
   ↓
6. NickCommand checks UserRegistry, updates nickname
   ↓
7. Generate response
   ↓
8. MessageProcessor.buildReply() → ":server 001 alice :Welcome\r\n"
   ↓
9. NetworkManager.sendMessage(fd, response)
```

### Channel Message Broadcast
```
1. Client sends: "PRIVMSG #general :Hello\r\n"
   ↓
2. Parse → Execute PrivmsgCommand
   ↓
3. ChannelRegistry.getChannel("#general")
   ↓
4. Channel.broadcast(message, sender)
   ↓
5. For each member in channel:
      NetworkManager.sendMessage(member_fd, formatted_message)
```

### Client Disconnection
```
1. NetworkManager detects disconnect
   ↓
2. IRCServer.handleDisconnections()
   ↓
3. ChannelRegistry.removeUserFromAll(nickname)
   ↓
4. UserRegistry.removeClient(fd)
   ↓
5. NetworkManager cleanup already done
```

---

## Design Principles

### Separation of Concerns
Each component has ONE clear responsibility:
- NetworkManager: Network operations
- MessageProcessor: Protocol translation
- CommandEngine: Command execution
- UserRegistry: User state
- ChannelRegistry: Channel state

### Dependency Direction
```
IRCServer
    ↓
CommandEngine → UserRegistry, ChannelRegistry
    ↓
MessageProcessor
    ↓
NetworkManager
```

Lower layers don't know about upper layers.

### Error Handling
- Network errors: Handle gracefully, don't crash
- Protocol errors: Send IRC error responses
- State errors: Validate and reject invalid operations

### Scalability
- Single-threaded event loop
- Non-blocking I/O
- Efficient data structures
- Minimal memory allocation in hot paths

---

## Key Requirements (from subject)

### Mandatory
- ✅ Handle multiple clients simultaneously
- ✅ No forking
- ✅ All I/O non-blocking
- ✅ Single poll() equivalent
- [ ] Authentication (password)
- [ ] Set nickname and username
- [ ] Join channels
- [ ] Send/receive messages
- [ ] Operators and regular users
- [ ] Operator commands: KICK, INVITE, TOPIC, MODE
- [ ] Channel modes: i, t, k, o, l

### Bonus
- [ ] File transfer
- [ ] Bot

---

## Testing Strategy

### Unit Testing
- Each component tested independently
- Mock dependencies where needed
- Test edge cases and error conditions

### Integration Testing
- Test component interactions
- Verify data flow between layers
- Test with actual IRC clients

### Reference Client
Choose one IRC client as reference:
- irssi
- WeeChat
- HexChat
- nc (for basic testing)

---

## Notes

**Performance:**
- poll() handles up to 1000+ connections efficiently
- Memory overhead minimal per client
- No unnecessary copying of data

**Maintainability:**
- Clear interfaces between components
- Comprehensive documentation
- Consistent naming conventions
- Error messages include context
