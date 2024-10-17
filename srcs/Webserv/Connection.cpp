/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dshatilo <dshatilo@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/17 10:16:03 by dshatilo          #+#    #+#             */
/*   Updated: 2024/10/17 10:17:13 by dshatilo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Connection.hpp"

Connection::Connection(size_t timeout) : timeout_(timeout) {}

Connection::~Connection() {}
