/*
 * Copyright (C) 2007-2013 Dyson Technology Ltd, all rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef LOGGING_H
#define LOGGING_H

#include "log4qt/logger.h"

#include <QObject>

#define LOG_DEBUG Log4Qt::Logger::rootLogger()->debug
#define LOG_ERROR Log4Qt::Logger::rootLogger()->error
#define LOG_FATAL Log4Qt::Logger::rootLogger()->fatal
#define LOG_INFO  Log4Qt::Logger::rootLogger()->info
#define LOG_LOG   Log4Qt::Logger::rootLogger()->log
#define LOG_TRACE Log4Qt::Logger::rootLogger()->trace
#define LOG_WARN  Log4Qt::Logger::rootLogger()->warn

namespace Logging
{
    void SetupLogging();
}

#endif // LOGGING_H
