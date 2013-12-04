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

#include "TargetCollectionTool.h"

#include "TargetsWidget.h"

#include "RobotMetricsSchema.h"

#include <QtGui/QLabel>
#include <QtGui/QComboBox>

namespace
{
    const KeyName targetsSchemaName("targets");

    const KeyName targetSchemaName ("target");
}

TargetCollectionTool::TargetCollectionTool(QWidget* parent,
                                            MainWindow& mainWindow) :
    CollectionToolWidget(tr("Target"),
                          CreateCollectionSchema(),
                          CreateElementSchema(),
                          parent,
                          &mainWindow)
{
    AddSubTool(new TargetsWidget(this));
}

TargetCollectionTool::~TargetCollectionTool()
{
}

const QString TargetCollectionTool::Name() const
{
    return tr("Targets");
}

const QString TargetCollectionTool::GetSubSchemaDefaultFileName() const
{
    return "targets/targets.xml";
}

const WbSchema TargetCollectionTool::CreateCollectionSchema()
{
    WbSchema targetSchema(CreateWorkbenchSubSchema(targetsSchemaName, tr("Targets")));

    return targetSchema;
}

const WbSchema TargetCollectionTool::CreateElementSchema()
{
    WbSchema schema(CreateElementWorkbenchSubSchema(targetSchemaName, Unnamed(tr("Target"))));

    schema.AddDependant(RobotMetricsSchema::schemaName,
                         RobotMetricsSchema::targetTypeKey);

    return schema;
}
