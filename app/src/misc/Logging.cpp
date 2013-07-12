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

#include "Logging.h"

#include "log4qt/logger.h"
#include "log4qt/logmanager.h"
#include "log4qt/rollingfileappender.h"
#include "log4qt/simplelayout.h"
#include "log4qt/level.h"

namespace Logging
{
    void SetupLogging()
    {
        // Create a layout
        Log4Qt::LogManager::rootLogger();
        Log4Qt::SimpleLayout *p_layout = new Log4Qt::SimpleLayout();
        p_layout->activateOptions();

        // Create an appender
        Log4Qt::RollingFileAppender *p_appender = new Log4Qt::RollingFileAppender(p_layout, "gts.log");
        p_appender->activateOptions();

       // Set appender on root logger
        Log4Qt::Logger::rootLogger()->addAppender(p_appender);
	    Log4Qt::Logger::rootLogger()->setLevel(Log4Qt::Level::ALL_INT);
    }
}
