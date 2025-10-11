/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   NetworkManager.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: odana <odana@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/08 16:42:17 by odana             #+#    #+#             */
/*   Updated: 2025/10/12 00:41:04 by odana            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef NETWORK_MANAGER_HPP
# define NETWORK_MANAGER_HPP

# include <vector>
# include <map>
# include <queue>
# include <string>
# include <utility>
# include <sys/socket.h>    // socket, bind, listen, setsockopt
# include <netinet/in.h>    // sockaddr_in, INADDR_ANY
# include <arpa/inet.h>     // htons, inet_addr
# include <fcntl.h>         // fcntl, O_NONBLOCK
# include <unistd.h>        // close
# include <poll.h>          // pollfd, POLLIN
# include <stdexcept>
# include <errno.h>

class NetworkManager
{
    private:

        int _serverSocket;
        std::vector<struct pollfd>  _pollFds;
        std::map<int, std::string>  _readBuffers;
        std::map<int, std::queue<std::string> > _writeQueues;
        std::vector<int> _newConnections;
        std::vector<int> _disconnectedClients;
    
    public: 
        NetworkManager();
        ~NetworkManager();
        
        void    initialize(int port);
        void    pollEvents();
        void    sendMessage(int clientFd, const std::string& message);
        void    removeClient(int fd);
        bool    isValidSocket(int fd);
        std::vector<int>    getNewClients();
        std::vector<int>    getDisconnectedClients();
        std::vector<std::pair<int, std::string> > getCompleteMessages();

    private:
        void    handleNewConnection();
        void    handleClientEvent(size_t index);
        void    handleIncomingData(size_t index);
        void    handleOutgoingData(size_t index);
        void    cleanupDisconnectedClients();
    };

# endif