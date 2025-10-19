/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IRCServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: odana <odana@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/08 16:14:51 by odana             #+#    #+#             */
/*   Updated: 2025/10/19 20:26:38 by odana            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IRC_SERVER_HPP
# define IRC_SERVER_HPP

# include <string>
# include <stdexcept>
# include <csignal>
# include <cstdlib>
# include "NetworkManager.hpp"
# include "MessageProcessor.hpp"
# include "CommandEngine.hpp"

class IRCServer
{
    private:

    IRCServer(const IRCServer& other);
    IRCServer&  operator=(const IRCServer& other);
    
    public:

    IRCServer(int port, const std::string password);
    ~IRCServer();
    
    void    initialize();
    void    run();
    void    shutdown();

    static void signalHandler(int sig);

    private:

    const int           _port;
    const std::string   _password;
    static IRCServer*   _instance;
    bool    _running;

    NetworkManager  _networkManager;
    // UserRegistry    _userRegistry; // TODO @yitani
    // ChannelRegistry _channelRegistry; // TODO @yitani
    // CommandEngine   _commandEngine; // TODO @yitani
    
};

#endif