/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   NickCommand.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: odana <odana@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/19 21:05:28 by odana             #+#    #+#             */
/*   Updated: 2025/10/19 21:18:50 by odana            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef NICK_COMMAND_HPP
# define NICK_COMMAND_HPP

// TODO @yitani

/* FORMAT: NICK <nickname>
** 
** check if nickname provided -> no password parameter -> error 461
** check nickname format (alphanumeric, within max lengh, no spaces) -> already registered -> error 462
** check if nickname taken --> taken --> error 463
** update nickname in user registry
** if fully registered now (pass + nick + user) --> AUTHENTICATING to REGISTERED --> send welcome message
*/

#endif