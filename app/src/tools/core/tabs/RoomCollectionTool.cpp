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

#include "RoomCollectionTool.h"

#include "SetUpRoomWidget.h"
#include "CreateFloorPlanWidget.h"
#include "CreateFloorMaskWidget.h"
#include "MeasureFloorWidget.h"
#include "CameraHardware.h"
#include "RunSchema.h"

#include <QtGui/QLabel>
#include <QtGui/QComboBox>

RoomCollectionTool::RoomCollectionTool(CameraHardware& cameraHardware,
                                        QWidget* parent,
                                        MainWindow& mainWindow ) :
    CollectionToolWidget(tr("Room"),
                          CreateCollectionSchema(),
                          CreateElementSchema(),
                          parent,
                          &mainWindow),
    m_cameraHardware(cameraHardware)
{
    AddSubTool(new SetUpRoomWidget(this));
    AddSubTool(new CreateFloorPlanWidget (cameraHardware, this));
    AddSubTool(new CreateFloorMaskWidget (this));
    AddSubTool(new MeasureFloorWidget (this));
}

RoomCollectionTool::~RoomCollectionTool()
{
}

const QString RoomCollectionTool::Name() const
{
    return tr("Rooms");
}

const QString RoomCollectionTool::GetSubSchemaDefaultFileName() const
{
    return "rooms/rooms.xml";
}

const WbSchema RoomCollectionTool::CreateCollectionSchema()
{
    WbSchema roomsSchema(CreateWorkbenchSubSchema(KeyName("rooms"), tr("Rooms")));
    return roomsSchema;
}

const WbSchema RoomCollectionTool::CreateElementSchema()
{
    WbSchema schema = CreateElementWorkbenchSubSchema(KeyName("room"), Unnamed(tr("Room")));

    schema.AddDependant(RunSchema::schemaName,
                         RunSchema::roomIdKey);

    return schema;
}
