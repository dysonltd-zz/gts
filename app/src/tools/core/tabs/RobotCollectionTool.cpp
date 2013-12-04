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

#include "RobotCollectionTool.h"

#include "RobotMetricsWidget.h"
#include "WbDefaultKeys.h"
#include "CameraHardware.h"
#include "CameraDescription.h"

#include <QtGui/QAction>
#include <QtGui/QFileDialog>
#include <QtGui/QApplication>
#include <QtCore/QString>

#include <iostream>

namespace
{
    const KeyName robotsSchemaName("robots");

    const KeyName robotSchemaName ("robot");
}

RobotCollectionTool::RobotCollectionTool(QWidget* parent,
                                        MainWindow& mainWindow) :
    CollectionToolWidget(tr("Robot"),
                          CreateCollectionSchema(),
                          CreateElementSchema(),
                          parent,
                          &mainWindow)
{
    AddSubTool(new RobotMetricsWidget(this));
}

RobotCollectionTool::~RobotCollectionTool()
{
}

const QString RobotCollectionTool::Name() const
{
    return tr("Robots");
}

const QString RobotCollectionTool::GetSubSchemaDefaultFileName() const
{
    return "robots/robots.xml";
}

const WbSchema RobotCollectionTool::CreateCollectionSchema()
{
    WbSchema robotsSchema(CreateWorkbenchSubSchema(robotsSchemaName, tr("Robots")));
    return robotsSchema;
}

const WbSchema RobotCollectionTool::CreateElementSchema()
{
    WbSchema robotSchema(CreateElementWorkbenchSubSchema(robotSchemaName,
                                                           Unnamed(tr("Robot"))));
    return robotSchema;
}

