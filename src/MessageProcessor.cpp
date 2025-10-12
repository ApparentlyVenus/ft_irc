/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MessageProcessor.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: odana <odana@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/12 02:40:22 by odana             #+#    #+#             */
/*   Updated: 2025/10/12 06:08:33 by odana            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/MessageProcessor.hpp"

/*
** parse(const std::string& rawMessage)
** Parses raw IRC message string into structured IRCMessage object.
**
** Format: [:prefix] <command> [params] [:trailing]\r\n
**
** Returns: IRCMessage with extracted components
**          Empty command indicates invalid message
*/
IRCMessage MessageProcessor::parse(const std::string& rawMessage)
{
    IRCMessage msg;
    std::string input = rawMessage;
    
    if (rawMessage.empty() || rawMessage.size() < 2)
        return (msg);
    
    if (input.size() >= 2 && input.substr(input.size() - 2) == "\r\n")
        input = input.substr(0, input.size() - 2);

    if (input.empty())
        return (msg);
        
    size_t pos = 0;
    if (!input.empty() && input[0] == ':')
    {
        size_t space = input.find(' ');
        if (space != std::string::npos)
        {
            msg.prefix = input.substr(1, space - 1);
            pos = space + 1;
        }
    }
    size_t space = input.find(' ', pos);
    if (space != std::string::npos)
    {
        msg.command = input.substr(pos, space - pos);
        pos = space + 1;
    }
    else
    {
        msg.command = input.substr(pos);
        return (msg);
    }
    while (pos < input.length())
    {
        if (input[pos] == ':')
        {
            msg.trailing = input.substr(pos + 1);
            break ;
        }
        space = input.find(' ', pos);
        if (space != std::string::npos)
        {
            msg.params.push_back(input.substr(pos, space - pos));
            pos = space + 1;
        }
        else
        {
            msg.params.push_back(input.substr(pos));
            break ;
        }
    }
    return (msg);
}

/*
** buildNumericReply(int code, const std::string& target, const std::string& message)
** Builds IRC numeric reply (server responses with 3-digit codes).
**
** Format: :server CODE target :message\r\n
**
** Parameters:
**   code    - Numeric code (001, 433, etc.)
**   target  - Client nickname
**   message - Reply message
**
** Returns: Formatted IRC numeric reply string
*/
std::string MessageProcessor::buildNumericReply(int code, const std::string& target, const std::string& message)
{
    std::string result; 
    result = ":ircserv ";

    std::stringstream ss; 
    ss << std::setfill('0') << std::setw(3) << code;
    result += ss.str();
    result += " ";

    result += target;
    result += " ";

    result += ":";
    result += message;

    result += "\r\n";
    return (result);
}

/*
** buildMessage(const IRCMessage& message)
** Builds IRC message from IRCMessage structure.
**
** Format: :prefix COMMAND params :trailing\r\n
**
** Parameters:
**   message - IRCMessage struct with all components
**
** Returns: Formatted IRC message string
*/
std::string MessageProcessor::buildMessage(const IRCMessage& message)
{
    std::string result; 
    if (!message.prefix.empty())
    {
        result = ":";
        result += message.prefix;
        result += " ";
    }

    result += message.command;

    for (size_t i = 0; i < message.params.size(); i++)
    {
        result += " ";
        result += message.params[i];
    }

    if (!message.trailing.empty())
    {
        result += ":";
        result += message.trailing;
    }
    result += "\r\n";
    return (result);
}