/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MessageProcessor.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: odana <odana@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/12 02:40:25 by odana             #+#    #+#             */
/*   Updated: 2025/10/12 06:05:11 by odana            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MESSAGE_PROCESSOR_HPP
# define MESSAGE_PROCESSOR_HPP

# include <string>
# include <vector>
# include <iomanip>
# include <sstream>

struct IRCMessage
{
    std::string prefix;
    std::string command;
    std::vector<std::string> params;
    std::string trailing;
    
    IRCMessage();
};

class MessageProcessor
{
public:
    static IRCMessage parse(const std::string& rawMessage);
    
    static std::string buildNumericReply(int code, const std::string& target, 
        const std::string& message);
    
    static std::string buildMessage(const IRCMessage& message);
};

#endif