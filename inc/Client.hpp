/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: odana <odana@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/19 20:41:52 by odana             #+#    #+#             */
/*   Updated: 2025/10/19 21:08:22 by odana            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <string>
# include <set>

enum ClientState
{
    CONNECTING,     // no data just connection
    AUTHENTICATING, // password validated, need nick/user
    REGISTERED      // fully authenticated and can use commands
};

class Client
{
    private:
    
    int         _fd;
    std::string _username;
    std::string _nickname;
    std::string _realname;
    std::string _hostname;
    
    ClientState _state;
    bool        _paswordVerified;
    bool        _isOperator;
    
    public:
    
    int         getFd() const;
    ClientState getState() const;

    const std::string&  getNickname() const;
    const std::string&  getUsername() const;
    const std::string&  getRealname() const;
    const std::string&  getHostname() const;
    
    bool    isPasswordVerified() const;
    bool    isOperator() const;
    
    void    setUsername(const std::string& username);
    void    setNickname(const std::string& nickname);
    void    setRealname(const std::string& realname);
    void    setHostname(const std::string& hostname);
    void    setState(ClientState state);
    void    setPasswordVerified(bool verified);
    void    setOperator(bool isOp);
    
    void    joinChannel(const std::string& channelName);
    void    leaveChannel(const std::string& channelName);
    bool    isInChannel(const std::string& channelName);
    const std::set<std::string>& getChannels() const;
    
    std::string getPrefix() const;
    
    // ... getters and setters?
};

#endif 