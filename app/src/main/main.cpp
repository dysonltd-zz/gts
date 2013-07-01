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

#include "MainWindow.h"

#include "Message.h"
#include "QtMessageHandler.h"
#include "CameraDescription.h"
#include "GTSApplication.h"
#include "Version.h"

#include "Logging.h"

#include <QVariant>

#include <iostream>

namespace
{
    void InitialiseMessageHandler( MainWindow& mainWindow )
    {
        Message::SetHandler( std::unique_ptr< MessageHandler >( new QtMessageHandler( mainWindow ) ) );
    }
}

int main( int argc, char** argv )
{
    GTSApplication app( argc, argv );

    QCoreApplication::setOrganizationName("GTS");
    QCoreApplication::setApplicationName("Ground Truth System");

    setupLogging();

    qRegisterMetaType<CameraDescription>("CameraDescription");

    MainWindow mainWindow;

    InitialiseMessageHandler( mainWindow );

    mainWindow.Start();

    LOG_TRACE("Application starting...");

    LOG_INFO(QObject::tr("Version: %1 Date: %2.").arg(GTS_GIT_REVID)
                                                 .arg(GTS_BUILD_DATE));
    return app.exec();
}

