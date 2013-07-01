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

#include "RoomCollectionToolWidget.h"
#include <QtGui/QLabel>
#include <QtGui/QComboBox>
#include "SetUpRoomToolWidget.h"
#include "CreateFloorPlanToolWidget.h"
#include "CreateFloorMaskToolWidget.h"
#include "CameraHardware.h"

RoomCollectionToolWidget::RoomCollectionToolWidget( CameraHardware& cameraHardware,
                                                    QWidget* parent,
                                                    MainWindow& mainWindow  )
    :
    CollectionToolWidget( tr( "Room" ),
                          CreateCollectionSchema(),
                          CreateElementSchema(),
                          parent,
                          &mainWindow ),
    m_cameraHardware( cameraHardware )
{
    AddSubTool( new SetUpRoomToolWidget( this ) );
    AddSubTool( new CreateFloorPlanToolWidget ( cameraHardware, this ) );
    AddSubTool( new CreateFloorMaskToolWidget ( this ) );
}

RoomCollectionToolWidget::~RoomCollectionToolWidget()
{
}

const QString RoomCollectionToolWidget::Name() const
{
    return tr( "Rooms" );
}

const QString RoomCollectionToolWidget::GetSubSchemaDefaultFileName() const
{
    return "rooms/rooms.xml";
}

const WbSchema RoomCollectionToolWidget::CreateCollectionSchema()
{
    WbSchema roomsSchema( CreateWorkbenchSubSchema( KeyName( "rooms" ), tr( "Rooms" ) ) );
    return roomsSchema;
}

const WbSchema RoomCollectionToolWidget::CreateElementSchema()
{
    return CreateElementWorkbenchSubSchema( KeyName( "room" ), Unnamed( tr( "Room" ) ) );
}
