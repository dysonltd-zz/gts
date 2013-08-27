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

#include "RunCollectionTool.h"

#include "CaptureVideoWidget.h"
#include "TrackRobotWidget.h"
#include "PostProcessWidget.h"
#include "TrackResultsWidget.h"

#include "WbConfigTools.h"
#include "RoomsCollection.h"
#include "RunSchema.h"

#include <QtGui/QLabel>
#include <QtGui/QComboBox>

RunCollectionTool::RunCollectionTool( CameraHardware& cameraHardware,
                                      QWidget* parent,
                                      MainWindow& mainWindow  ) :
    CollectionToolWidget( tr( "Run" ), CreateCollectionSchema(), CreateElementSchema(), parent, &mainWindow ),
    m_roomCombo( new QComboBox )
{
    AddToolDetail( new QLabel( tr( "Room" ) ), m_roomCombo );
    AddMapper( RunSchema::roomIdKey, m_roomCombo );

    AddSubTool( new CaptureVideoWidget( cameraHardware, this ) );
    AddSubTool( new TrackRobotWidget( this ) );
    AddSubTool( new PostProcessWidget( this ) );
    AddSubTool( new TrackResultsWidget( this ) );

    RegisterCollectionCombo( m_roomCombo, RoomsCollection() );
}

RunCollectionTool::~RunCollectionTool()
{
}

const QString RunCollectionTool::Name() const
{
    return tr("Runs");
}

const QString RunCollectionTool::GetSubSchemaDefaultFileName() const
{
    return "runs/runs.xml";
}

const WbSchema RunCollectionTool::CreateCollectionSchema()
{
    WbSchema runsSchema( CreateWorkbenchSubSchema( KeyName( "runs" ), tr( "Runs" ) ) );
    return runsSchema;
}

const WbSchema RunCollectionTool::CreateElementSchema()
{
    using namespace RunSchema;
    WbSchema runSchema( CreateElementWorkbenchSubSchema( schemaName, Unnamed( tr( "Run" ) ) ) );
    runSchema.AddSingleValueKey( roomIdKey, WbSchemaElement::Multiplicity::One );
    return runSchema;
}
