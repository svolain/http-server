/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIConnection.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dshatilo <dshatilo@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/18 14:07:41 by dshatilo          #+#    #+#             */
/*   Updated: 2024/10/18 14:21:01 by dshatilo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGIConnection.hpp"

CGIConnection::CGIConnection(int fd, pid_t child_pid, std::fstream& file)
    : Connection(fd, 10), child_pid_(child_pid), file_(file) {}

int CGIConnection::ReceiveData(pollfd& poll) {
  (void)poll;
}

int CGIConnection::SendData(pollfd& poll) {
  (void)poll;
}

void  CGIConnection::CleanupConnection() {}