/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dshatilo <dshatilo@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/17 10:16:03 by dshatilo          #+#    #+#             */
/*   Updated: 2024/10/17 16:08:19 by dshatilo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Connection.hpp"

Connection::Connection(size_t timeout)
    : last_active_(std::chrono::steady_clock::now()),
      timeout_(timeout) {}

Connection::~Connection() {}

bool Connection::HasTimedOut() const {
  timepoint now = std::chrono::steady_clock::now();
  auto duration = std::chrono::duration_cast<sec>(now - last_active_);
  if (duration.count() >= timeout_)
    return true;
  return false;
}