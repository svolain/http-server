/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/01 14:01:10 by dshatilo          #+#    #+#             */
/*   Updated: 2024/10/04 16:34:27 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOGGER_H_
#define LOGGER_H_

#include <string>

void logInfo(const std::string& message);
void logDebug(const std::string& message, bool flag = true);
void logError(const std::string& message);

#endif // LOGGER_H