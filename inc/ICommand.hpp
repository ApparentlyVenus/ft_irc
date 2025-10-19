/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ICommand.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: odana <odana@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/19 20:36:39 by odana             #+#    #+#             */
/*   Updated: 2025/10/19 20:52:47 by odana            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ICOMMAND_HPP
# define ICOMMAND_HPP

# include "Client.hpp"
# include "MessageProcessor.hpp"

class ICommand 
{
    public:

    virtual ~ICommand() {}
    virtual void execute(Client* client, const IRCMessage& msg) = 0;
    virtual bool requiresAuth() const = 0;
    
};

#endif