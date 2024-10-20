/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dshatilo <dshatilo@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/24 17:38:49 by klukiano          #+#    #+#             */
/*   Updated: 2024/10/19 18:25:14 by dshatilo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <chrono>
#include <iostream>
#include <cstddef>
#include <poll.h>


class Connection
{
 private:
 using timepoint = std::chrono::steady_clock::time_point;
 using sec = std::chrono::seconds;

 public:
  Connection(int fd, size_t timeout);
  Connection(const Connection& other) = delete;
  Connection(Connection&& other)      = default;
  Connection& operator=(const Connection& other) = delete;
  Connection& operator=(Connection&& other)      = delete;

  virtual ~Connection() = 0;

  virtual int   ReceiveData(pollfd& poll) = 0;
  virtual int   SendData(pollfd& poll)    = 0;
  bool          HasTimedOut() const;
  void          UpdateLastActive();

  int       fd_;
  timepoint last_active_;

 protected:
  const long  timeout_;
};

#endif //CONNECTION_HPP