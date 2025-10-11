/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IRCServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: odana <odana@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/08 16:36:30 by odana             #+#    #+#             */
/*   Updated: 2025/09/08 16:40:32 by odana            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/IRCServer.hpp"

IRCServer::IRCServer(int port, const std::string password) : _port(port), _password(password)
{
    if (port <= 0 || port > 65535)
        throw std::runtime_error("Port must be between 1 and 65535");
    if (password.empty())
        throw std::runtime_error("Password cannot be empty");
}