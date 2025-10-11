/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   NetworkManager.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: odana <odana@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/08 16:56:49 by odana             #+#    #+#             */
/*   Updated: 2025/10/12 00:46:21 by odana            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/NetworkManager.hpp"

NetworkManager::NetworkManager() : _serverSocket(-1) {}

NetworkManager::~NetworkManager()
{
    for (size_t i = 1; i < _pollFds.size(); i++)
        close(_pollFds[i].fd);
    if (_serverSocket != -1)
        close(_serverSocket);
}

/*
** initialize(int port)
** Sets up server socket and begins listening for connections.
** 
** Steps:
** 1. Creates TCP socket (SOCK_STREAM)
** 2. Sets SO_REUSEADDR (allows immediate restart)
** 3. Sets non-blocking mode
** 4. Binds to specified port on all interfaces (INADDR_ANY)
** 5. Starts listening with maximum queue (SOMAXCONN)
** 6. Adds server socket to poll array for monitoring
**
** Throws: std::runtime_error on socket/bind/listen failure
** Note: Port must be between 1-65535 (validated by IRCServer)
*/
void    NetworkManager::initialize(int port)
{
    _serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (_serverSocket == -1)
        throw std::runtime_error("Error: socket creation failed");
        
    int opt = 1;
    setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    fcntl(_serverSocket, F_SETFL, O_NONBLOCK);
    
    struct sockaddr_in  serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);

    if (bind(_serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1)
    {
        close(_serverSocket);
        throw std::runtime_error("Error: bind failed - port may be in use");
    }
    
    if (listen(_serverSocket, SOMAXCONN) == -1)
    {
        close(_serverSocket);
        throw std::runtime_error("Error: listen failed");
    }
    
    struct pollfd serverPollFd;
    serverPollFd.fd = _serverSocket;
    serverPollFd.events = POLLIN;
    serverPollFd.revents = 0;

    _pollFds.push_back(serverPollFd);
}

/*
** pollEvents()
** Main event detection loop - waits for and processes network events.
** 
** Process:
** 1. Clears previous event tracking
** 2. Calls poll() - BLOCKS until activity on any file descriptor
** 3. Iterates through all file descriptors with events
**    - Index 0 (server socket): New connection → handleNewConnection()
**    - Index 1+ (clients): Data/disconnect → handleClientEvent()
** 4. Cleans up disconnected clients
**
** Called repeatedly in main server loop.
** Handles multiple simultaneous events in single call.
*/
void    NetworkManager::pollEvents()
{
    _newConnections.clear();
    _disconnectedClients.clear();
    
    int ready = poll(&_pollFds[0], _pollFds.size(), -1);
    
    if (ready == -1)
    {
        if (errno == EINTR)
            return ;
        throw std::runtime_error("Error: poll failed");
    }
    
    for (size_t i = 0; i < _pollFds.size(); i++)
    {
        if (_pollFds[i].revents == 0)
            continue ;
        if (i == 0)
            handleNewConnection();
        else
            handleClientEvent(i);
    }
    cleanupDisconnectedClients();
}

/*
** handleNewConnection() [PRIVATE]
** Accepts new client connection and adds to monitoring.
**
** Steps:
** 1. accept() - creates new client socket
** 2. Sets client socket to non-blocking mode
** 3. Adds to _pollFds for future monitoring (POLLIN events)
** 4. Tracks in _newConnections for IRCServer processing
**
** Errors handled gracefully - bad connections don't crash server.
*/
void    NetworkManager::handleNewConnection()
{
    int clientFd = accept(_serverSocket, NULL, NULL);
    if (clientFd == -1)
        return ;
    
    if (fcntl(clientFd, F_SETFL, O_NONBLOCK) == -1)
    {
        close(clientFd);
        return ;
    }

    struct pollfd clientPollFd;
    clientPollFd.fd = clientFd;
    clientPollFd.events = POLLIN;
    clientPollFd.revents = 0;
    
    _pollFds.push_back(clientPollFd);
    _newConnections.push_back(clientFd);
}  

/*
** handleClientEvent(size_t index) [PRIVATE]
** Routes client socket events to appropriate handlers.
**
** Priority order:
** 1. POLLHUP/POLLERR → Disconnection (mark for cleanup)
** 2. POLLIN → Incoming data (handleIncomingData)
** 3. POLLOUT → Ready to send (handleOutgoingData)
**
** Single event can trigger multiple handlers (read + write).
*/
void NetworkManager::handleClientEvent(size_t index)
{
    if (_pollFds[index].revents & (POLLHUP | POLLERR))
    {
        _disconnectedClients.push_back(_pollFds[index].fd);
        return;
    }
    
    if (_pollFds[index].revents & POLLIN)
        handleIncomingData(index);
    
    if (_pollFds[index].revents & POLLOUT)
        handleOutgoingData(index);
}

/*
** handleIncomingData(size_t index) [PRIVATE]
** Reads data from client socket and buffers it.
**
** Process:
** 1. recv() up to 4096 bytes from client
** 2. Append to _readBuffers[fd] (accumulates partial messages)
** 3. Handle special cases:
**    - bytesRead > 0: Data received successfully
**    - bytesRead == 0: Client closed connection (graceful)
**    - bytesRead == -1: Error (mark disconnect unless EAGAIN/EWOULDBLOCK)
**
** Buffer persists until complete IRC message (\r\n) extracted.
*/
void NetworkManager::handleIncomingData(size_t index)
{
    int clientFd = _pollFds[index].fd;
    char buffer[4096];
    int bytesRead = recv(clientFd, buffer, sizeof(buffer) - 1, 0);
    
    if (bytesRead > 0)
    {
        buffer[bytesRead] = '\0';
        _readBuffers[clientFd] += buffer;
    }
    else if (bytesRead == 0)
        _disconnectedClients.push_back(clientFd);
    else
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
            _disconnectedClients.push_back(clientFd);
    }
}

/*
** handleOutgoingData(size_t index) [PRIVATE]
** Sends queued messages when socket is writable.
**
** Process:
** 1. Check if messages queued for this client
** 2. send() front message from queue
** 3. If successful, remove from queue
** 4. If queue empty, stop monitoring POLLOUT (clear flag)
**
** Non-blocking send: If socket buffer full (EAGAIN), try next cycle.
** Serious errors → mark client for disconnection.
*/
void NetworkManager::handleOutgoingData(size_t index)
{
    int fd = _pollFds[index].fd;
    
    if (_writeQueues[fd].empty())
        return;
    
    std::string& message = _writeQueues[fd].front();
    int bytesSent = send(fd, message.c_str(), message.length(), 0);
    
    if (bytesSent > 0)
    {
        _writeQueues[fd].pop();
        
        if (_writeQueues[fd].empty())
            _pollFds[index].events &= ~POLLOUT;
    }
    else if (bytesSent == -1)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
            _disconnectedClients.push_back(fd);
    }
}

/*
** cleanupDisconnectedClients() [PRIVATE]
** Removes disconnected clients from all tracking structures.
**
** For each disconnected fd:
** 1. Remove from _pollFds (stop monitoring)
** 2. Erase from _readBuffers (free partial message data)
** 3. Erase from _writeQueues (discard pending messages)
** 4. close() socket file descriptor
**
** Called at end of pollEvents() after all events processed.
** Ensures safe removal without disrupting iteration.
*/
void    NetworkManager::cleanupDisconnectedClients()
{
    for (size_t i = 0; i < _disconnectedClients.size(); i++)
    {
        int fd = _disconnectedClients[i];
        
        for (size_t j = 0; j < _pollFds.size(); j++)
        {
            if (_pollFds[j].fd == fd)
            {
                _pollFds.erase(_pollFds.begin() + j);
                break;
            }
        }
        
        _readBuffers.erase(fd);
        _writeQueues.erase(fd);
        
        close(fd);
    }
}

/*
** getCompleteMessages()
** Extracts complete IRC messages from read buffers.
**
** IRC message format: Must end with \r\n
** 
** Process:
** - Iterates through all _readBuffers
** - Finds messages ending with \r\n
** - Extracts and returns them as (fd, message) pairs
** - Leaves partial messages in buffer for next cycle
**
** Returns: Vector of (file descriptor, complete message) pairs
** Note: Single buffer can yield multiple messages
*/
std::vector<std::pair<int, std::string> >   NetworkManager::getCompleteMessages()
{
    std::vector<std::pair<int, std::string> > messages;
    
    for (std::map<int, std::string>::iterator it = _readBuffers.begin();
            it != _readBuffers.end(); ++it)
    {
        int fd = it->first;
        std::string& buffer = it->second;

        size_t  pos;
        while ((pos = buffer.find("\r\n")) != std::string::npos)
        {
           std::string message = buffer.substr(0, pos + 2);
           messages.push_back(std::make_pair(fd, message));
           buffer.erase(0, pos + 2); 
        }
    }
    return (messages);
}

/*
** sendMessage(int clientFd, const std::string& message)
** Queues message for transmission to specific client.
**
** Process:
** 1. Add message to client's write queue
** 2. Enable POLLOUT monitoring (tells poll to notify when writable)
** 3. Actual send happens in handleOutgoingData() when socket ready
**
** Queue-based design prevents blocking on full socket buffers.
** Messages sent in FIFO order.
*/
void    NetworkManager::sendMessage(int clientFd, const std::string& message)
{
    _writeQueues[clientFd].push(message);
    for (size_t i = 0; i < _pollFds.size(); i++)
    {
        if (_pollFds[i].fd == clientFd)
        {
            _pollFds[i].events |= POLLOUT;
            break ; 
        }
    }
}

bool    NetworkManager::isValidSocket(int fd)
{
    for (size_t i = 0; i < _pollFds.size(); i++)
    {
        if (_pollFds[i].fd == fd)
            return (true);
    }
    return (false);
}

void    NetworkManager::removeClient(int fd)
{
    _disconnectedClients.push_back(fd);
}

std::vector<int> NetworkManager::getNewClients()
{
    return (_newConnections);
}

std::vector<int> NetworkManager::getDisconnectedClients()
{
    return (_disconnectedClients);
}