/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dshatilo <dshatilo@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/24 17:38:49 by klukiano          #+#    #+#             */
/*   Updated: 2024/10/17 10:44:05 by dshatilo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <cstddef>
#include <poll.h>


class Connection
{
 public:
  Connection(size_t timeout);
  Connection(const Connection& other) = delete;
  Connection(Connection&& other)      = default;
  Connection& operator=(const Connection& other) = delete;
  Connection& operator=(Connection&& other)      = delete;

  virtual ~Connection() = 0;

  virtual int ReceiveData(pollfd& poll) = 0;
  virtual int SendData(pollfd& poll)    = 0;

  size_t  last_active_;

 protected:
  size_t  timeout_;
};



#endif //CONNECTION_HPP