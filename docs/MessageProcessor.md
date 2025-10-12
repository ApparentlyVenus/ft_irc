# MessageProcessor - Technical Reference

## Overview

**Purpose**: IRC protocol parser and formatter - converts between raw strings and structured data.

**Type**: Static utility class (no instances needed)

**Position**: Protocol Layer
```
NetworkManager → MessageProcessor → CommandEngine
   (raw bytes)    (parse/format)      (execute)
```

---

## Core Responsibilities

1. **Parse** incoming IRC messages (string → IRCMessage)
2. **Build** outgoing IRC responses (IRCMessage → string)
3. **Format** numeric replies with proper IRC syntax

**Does NOT:**
- Execute commands
- Manage users/channels
- Validate business logic (only syntax)

---

## Data Structure

### IRCMessage
```cpp
struct IRCMessage {
    std::string prefix;              // Optional: "nick!user@host" or "server"
    std::string command;             // Required: "NICK", "JOIN", "001", etc.
    std::vector<std::string> params; // Optional: command parameters
    std::string trailing;            // Optional: text after final ':'
};
```

**Empty command = invalid message**

---

## Functions

### parse()
```cpp
static IRCMessage parse(const std::string& rawMessage);
```

**Purpose**: Converts raw IRC string to structured data

**IRC Format**: `[:prefix] <command> [params] [:trailing]\r\n`

**Examples:**

| Input | Output |
|-------|--------|
| `"NICK alice\r\n"` | `{command="NICK", params=["alice"]}` |
| `"USER alice 0 * :Alice\r\n"` | `{command="USER", params=["alice","0","*"], trailing="Alice"}` |
| `":alice!u@h PRIVMSG #c :Hi\r\n"` | `{prefix="alice!u@h", command="PRIVMSG", params=["#c"], trailing="Hi"}` |

**Returns**: IRCMessage object (check `command.empty()` for validity)

---

### buildNumericReply()
```cpp
static std::string buildNumericReply(int code, 
                                     const std::string& target,
                                     const std::string& message);
```

**Purpose**: Builds server numeric responses (3-digit codes)

**Format**: `:server CODE target :message\r\n`

**Examples:**

| Code | Target | Message | Output |
|------|--------|---------|--------|
| 001 | alice | Welcome | `:ircserv 001 alice :Welcome\r\n` |
| 433 | bob | Nickname in use | `:ircserv 433 bob :Nickname in use\r\n` |
| 332 | alice | Topic | `:ircserv 332 alice :Topic\r\n` |

**Common Codes:**
- 001: Welcome
- 332: Channel topic
- 353: Names list
- 433: Nickname already in use
- 461: Not enough parameters
- 462: Already registered

---

### buildMessage()
```cpp
static std::string buildMessage(const IRCMessage& message);
```

**Purpose**: Builds general IRC messages from structure

**Format**: `:prefix COMMAND params :trailing\r\n`

**Example:**
```cpp
IRCMessage msg;
msg.prefix = "alice!user@host";
msg.command = "PRIVMSG";
msg.params.push_back("#general");
msg.trailing = "Hello world";

std::string output = buildMessage(msg);
// Result: ":alice!user@host PRIVMSG #general :Hello world\r\n"
```

**Use Cases:**
- Broadcasting PRIVMSG to channel members
- Notifying JOINs/PARTs
- Forwarding client commands

---

## IRC Message Format Rules

### Prefix (Optional)
- Starts with `:`
- Format: `nick!user@host` or just `servername`
- Indicates message source

### Command (Required)
- Word (NICK, JOIN, PRIVMSG) or 3-digit number (001, 433)
- Case-insensitive in parsing

### Params (Optional)
- Space-separated words
- No spaces allowed within param (unless in trailing)

### Trailing (Optional)
- Starts with `:` (space before colon)
- Can contain spaces
- Always last component

### Terminator (Required)
- `\r\n` (carriage return + line feed)
- Every IRC message must end with this

---

## Parsing Logic

**Step-by-step:**
1. Remove `\r\n` from end
2. If starts with `:` → extract prefix until space
3. Extract next word → command
4. Split remaining by spaces → params
5. If param starts with `:` → rest is trailing

**Example trace:**
```
Input: ":alice!u@h PRIVMSG #general :Hello world\r\n"

Step 1: ":alice!u@h PRIVMSG #general :Hello world"
Step 2: prefix="alice!u@h", rest="PRIVMSG #general :Hello world"
Step 3: command="PRIVMSG", rest="#general :Hello world"
Step 4: params=["#general"], rest=":Hello world"
Step 5: trailing="Hello world"
```

---

## Building Logic

**Numeric Reply:**
```
1. Prefix: ":ircserv "
2. Code: "001 " (padded to 3 digits)
3. Target: "alice "
4. Message: ":Welcome"
5. Terminator: "\r\n"

Result: ":ircserv 001 alice :Welcome\r\n"
```

**General Message:**
```
1. Prefix (if exists): ":alice!u@h "
2. Command: "PRIVMSG "
3. Params: "#general "
4. Trailing (if exists): ":Hello"
5. Terminator: "\r\n"

Result: ":alice!u@h PRIVMSG #general :Hello\r\n"
```

---

## Error Handling

**Parse errors:**
- Empty message → returns IRCMessage with empty command
- Malformed syntax → extracts what it can, rest ignored
- No validation of command validity (CommandEngine's job)

**Caller responsibility:**
```cpp
IRCMessage msg = MessageProcessor::parse(raw);
if (msg.command.empty())
{
    // Invalid message, ignore
    return;
}
// Process valid message
```

---

## Integration

### With NetworkManager
```cpp
// Incoming
std::vector<std::pair<int, std::string>> messages = 
    networkManager.getCompleteMessages();

for each message:
    IRCMessage parsed = MessageProcessor::parse(message.second);
    if (!parsed.command.empty())
        commandEngine.execute(client, parsed);
```

### With CommandEngine
```cpp
// Outgoing
std::string response = MessageProcessor::buildNumericReply(001, "alice", "Welcome");
networkManager.sendMessage(clientFd, response);
```

---

## Constants

**Server Name**: `"ircserv"` (hardcoded in buildNumericReply)

**Message Terminator**: `"\r\n"` (always)

**Max Message Length**: 512 bytes (IRC spec, enforced by NetworkManager)

---

## Common Patterns

**Client sends command:**
```
Client → "NICK alice\r\n"
NetworkManager → MessageProcessor.parse()
MessageProcessor → IRCMessage{command="NICK", params=["alice"]}
CommandEngine → Execute NickCommand
```

**Server sends reply:**
```
CommandEngine → buildNumericReply(001, "alice", "Welcome")
MessageProcessor → ":ircserv 001 alice :Welcome\r\n"
NetworkManager → Send to client
```

**Broadcast to channel:**
```
One client → "PRIVMSG #general :Hello\r\n"
Parse → Execute → For each member:
  buildMessage({prefix="sender", command="PRIVMSG", ...})
  Send to member
```