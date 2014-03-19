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

#include "AnalysisTool.h"

#include "CollateResultsWidget.h"
#include "WbConfigTools.h"
#include "ResultsCollection.h"
#include "RoomsCollection.h"
#include "ResultsSchema.h"

#include <QtGui/QComboBox>

AnalysisTool::AnalysisTool( QWidget* parent,
                                        MainWindow& mainWindow ) :
    CollectionToolWidget( tr( "Analysis" ),
                          CreateCollectionSchema(),
                          CreateElementSchema(),
                          parent,
                          &mainWindow ),
    m_roomCombo( new QComboBox )
{
    AddToolDetail( new QLabel( tr( "Room" ) ), m_roomCombo );
    AddMapper( ResultsSchema::roomIdKey, m_roomCombo );
    AddSubTool( new CollateResultsWidget( this ) );
    RegisterCollectionCombo( m_roomCombo, RoomsCollection() );
}

AnalysisTool::~AnalysisTool()
{
}

const QString AnalysisTool::Name() const
{
    return tr( "Analysis" );
}

const QString AnalysisTool::GetSubSchemaDefaultFileName() const
{
    return "analysis/analysis.xml";
}

const WbSchema AnalysisTool::CreateCollectionSchema()
{
    WbSchema schema( CreateWorkbenchSubSchema( KeyName( "analysis" ), tr( "Analysis" ) ) );
    return schema;
}

const WbSchema AnalysisTool::CreateElementSchema()
{
    using namespace ResultsSchema;
    WbSchema elementSchema(CreateElementWorkbenchSubSchema( schemaName, Unnamed( tr( "Results" ) ) ) );
    elementSchema.AddSingleValueKey( roomIdKey, WbSchemaElement::Multiplicity::One );
    return elementSchema;
}
