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

#include "CameraCollectionToolWidget.h"

#include "CameraHardware.h"
#include "CalibrateCameraToolWidget.h"
#include "CameraSelectionFormContents.h"
#include "NewElementWizard.h"
#include "WbDefaultKeys.h"
#include "CamerasPage.h"
#include "CameraSchema.h"

#include <QtGui/QLabel>
#include <QtGui/QComboBox>
#include <QtGui/QWizard>
#include <QtGui/QHBoxLayout>
#include <QtCore/qstringbuilder.h>
#include <QtGui/QStackedWidget>
#include <QtGui/QPushButton>

#include "Debugging.h"

namespace
{
    const KeyName camerasSchemaName( "cameras" );
}

CameraCollectionToolWidget::CameraCollectionToolWidget( CameraHardware& cameraHardware,
                                                        QWidget* parent,
                                                        MainWindow& mainWindow  )
    :
    CollectionToolWidget( tr( "Camera" ),
                          CreateCollectionSchema(),
                          CreateElementSchema(),
                          parent,
                          &mainWindow ),
    m_cameraHardware( cameraHardware )
{
    AddSubTool( new CalibrateCameraToolWidget( cameraHardware, this ) );
}

CameraCollectionToolWidget::~CameraCollectionToolWidget()
{
}

const QString CameraCollectionToolWidget::Name() const
{
    return tr( "Cameras" );
}

const QString CameraCollectionToolWidget::GetSubSchemaDefaultFileName() const
{
    return "cameras/cameras.xml";
}

void CameraCollectionToolWidget::AddExtraNewElementWizardPages(NewElementWizard* const wizard)
{
    wizard->addPage(new CamerasPage(m_cameraHardware, GetCollection()));
}


const WbSchema CameraCollectionToolWidget::CreateCollectionSchema()
{
    WbSchema schema(CreateWorkbenchSubSchema(camerasSchemaName, tr("Cameras")));
    return schema;
}

void CameraCollectionToolWidget::SetToolSpecificConfigItems(WbConfig newElement, NewElementWizard& wizard)
{
    const CameraDescription chosenCamera( wizard.field( CamerasPage::chosenCameraField )
                                                      .value<CameraDescription>() );
    newElement.SetKeyValue(WbDefaultKeys::descriptionKey, KeyValue::from(chosenCamera.ToPlainText()));
    newElement.SetKeyValue(CameraSchema::uniqueIdKey,
                           KeyValue::from( QString::fromStdWString(chosenCamera.UniqueId())));
}

const WbSchema CameraCollectionToolWidget::CreateElementSchema()
{
    WbSchema elementSchema(CreateElementWorkbenchSubSchema(CameraSchema::schemaName, Unnamed("Camera")));
    elementSchema.AddSingleValueKey(CameraSchema::uniqueIdKey, WbSchemaElement::Multiplicity::One);
    return elementSchema;
}

