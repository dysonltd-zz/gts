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

#include "SetUpCameraPositionsToolWidget.h"

#include "CamerasCollection.h"
#include "CalibratePositionToolWidget.h"
#include "WbConfigTools.h"
#include "CameraPositionSchema.h"
#include "RoomLayoutSchema.h"

#include <QtGui/QComboBox>
#include <QtGui/QLabel>

SetUpCameraPositionsToolWidget::SetUpCameraPositionsToolWidget( CameraHardware& cameraHardware,
                                                                QWidget* parent,
                                                                MainWindow& mainWindow ) :
    CollectionToolWidget( tr( "Position" ),
                          CreateCollectionSchema(),
                          CreateElementSchema(),
                          parent,
                          &mainWindow ),
    m_cameraComboBox( new QComboBox )
{
    AddSubTool( new CalibratePositionToolWidget( cameraHardware, this ) );
    AddToolDetail( new QLabel( tr( "Camera" ) ), m_cameraComboBox );
    AddMapper( CameraPositionSchema::cameraIdKey, m_cameraComboBox );

    RegisterCollectionCombo( m_cameraComboBox, CamerasCollection() );
}

SetUpCameraPositionsToolWidget::~SetUpCameraPositionsToolWidget()
{
}

const QString SetUpCameraPositionsToolWidget::Name() const
{
    return "Positions";
}

const QString SetUpCameraPositionsToolWidget::GetSubSchemaDefaultFileName() const
{
    return "cameraPositions/cameraPositions.xml";
}


const WbSchema SetUpCameraPositionsToolWidget::CreateCollectionSchema()
{
    WbSchema cameraPositionsSchema( CreateWorkbenchSubSchema( KeyName( "cameraPositions" ),
                                                              tr( "Positions" ) ) );
    return cameraPositionsSchema;
}

const WbSchema SetUpCameraPositionsToolWidget::CreateElementSchema()
{
    WbSchema schema( CreateElementWorkbenchSubSchema( CameraPositionSchema::schemaName,
                                                      Unnamed( tr( "CameraPos" ) ) ) );
    schema.AddSingleValueKey( CameraPositionSchema::cameraIdKey,
                              WbSchemaElement::Multiplicity::One );

    schema.AddDependant( RoomLayoutSchema::schemaName,
                         RoomLayoutSchema::cameraPositionIdsKey );

    return schema;
}
