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

#include "AnalysisToolWidget.h"

#include "AnalyseResultsWidget.h"

#include <QtGui/QComboBox>

#include "WbConfigTools.h"
#include "ResultsCollection.h"
#include "ResultsSchema.h"

AnalysisToolWidget::AnalysisToolWidget( QWidget* parent,
                                        MainWindow& mainWindow ) :
    CollectionToolWidget( tr( "Analysis" ),
                          CreateCollectionSchema(),
                          CreateElementSchema(),
                          parent,
                          &mainWindow )
{
    AddSubTool( new AnalyseResultsWidget( this ) );
}

AnalysisToolWidget::~AnalysisToolWidget()
{
}

const QString AnalysisToolWidget::Name() const
{
    return tr( "Analysis" );
}

const QString AnalysisToolWidget::GetSubSchemaDefaultFileName() const
{
    return "analysis/analysis.xml";
}

const WbSchema AnalysisToolWidget::CreateCollectionSchema()
{
    WbSchema schema( CreateWorkbenchSubSchema( KeyName( "analysis" ), tr( "Analysis" ) ) );
    return schema;
}

const WbSchema AnalysisToolWidget::CreateElementSchema()
{
    using namespace ResultsSchema;
    WbSchema elementSchema(CreateElementWorkbenchSubSchema( schemaName, Unnamed( tr( "Results" ) ) ) );
    return elementSchema;
}
