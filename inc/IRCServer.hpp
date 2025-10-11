/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IRCServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: odana <odana@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/08 16:14:51 by odana             #+#    #+#             */
/*   Updated: 2025/09/08 16:39:09 by odana            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IRC_SERVER_HPP
# define IRC_SERVER_HPP

# include <string>
# include <stdexcept>

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

    private:

    const int           _port;
    const std::string   _password;
};

#endif