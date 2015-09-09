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

#include "ConfigureRobotToolWidget.h"

#include <iostream>
#include "RobotMetricsToolWidget.h"
#include "WbDefaultKeys.h"
#include <QtGui/QAction>
#include <QtGui/qfiledialog.h>
#include <QtGui/qapplication.h>
#include "CameraSelectionForm.h"
#include <QtCore/qstring.h>
#include "CameraHardware.h"
#include "CameraDescription.h"

namespace
{
    const KeyName robotsSchemaName( "robots" );

    const KeyName robotSchemaName ( "robot" );
}

ConfigureRobotToolWidget::ConfigureRobotToolWidget( QWidget* parent,
                                                    MainWindow& mainWindow )
    :
    CollectionToolWidget( tr( "Robot" ),
                          CreateCollectionSchema(),
                          CreateElementSchema(),
                          parent,
                          &mainWindow )
{
    AddSubTool( new RobotMetricsToolWidget( this ) );
}

ConfigureRobotToolWidget::~ConfigureRobotToolWidget()
{
}

const QString ConfigureRobotToolWidget::Name() const
{
    return tr( "Robots" );
}

const QString ConfigureRobotToolWidget::GetSubSchemaDefaultFileName() const
{
    return "robots/robots.xml";
}

const WbSchema ConfigureRobotToolWidget::CreateCollectionSchema()
{
    WbSchema robotsSchema( CreateWorkbenchSubSchema( robotsSchemaName, tr( "Robots" ) ) );
    return robotsSchema;
}

const WbSchema ConfigureRobotToolWidget::CreateElementSchema()
{
    WbSchema robotSchema( CreateElementWorkbenchSubSchema( robotSchemaName,
                                                           Unnamed( tr( "Robot" ) ) ) );
    return robotSchema;
}
