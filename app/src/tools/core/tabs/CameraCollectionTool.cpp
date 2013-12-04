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

#include "CameraCollectionTool.h"

#include "CameraHardware.h"
#include "CameraCalibrationWidget.h"
#include "CameraSelectionFormContents.h"
#include "NewElementWizard.h"
#include "WbDefaultKeys.h"
#include "CamerasPage.h"
#include "CameraSchema.h"
#include "CameraPositionSchema.h"

#include <QtGui/QLabel>
#include <QtGui/QComboBox>
#include <QtGui/QWizard>
#include <QtGui/QHBoxLayout>
#include <QtCore/QStringBuilder>
#include <QtGui/QStackedWidget>
#include <QtGui/QPushButton>

#include "Debugging.h"

namespace
{
    const KeyName camerasSchemaName("cameras");
}

CameraCollectionTool::CameraCollectionTool(CameraHardware& cameraHardware,
                                            QWidget* parent,
                                            MainWindow& mainWindow ) :
    CollectionToolWidget(tr("Camera"),
                          CreateCollectionSchema(),
                          CreateElementSchema(),
                          parent,
                          &mainWindow),
    m_cameraHardware(cameraHardware)
{
    AddSubTool(new CameraCalibrationWidget(cameraHardware, this));
}

CameraCollectionTool::~CameraCollectionTool()
{
}

const QString CameraCollectionTool::Name() const
{
    return tr("Cameras");
}

const QString CameraCollectionTool::GetSubSchemaDefaultFileName() const
{
    return "cameras/cameras.xml";
}

void CameraCollectionTool::AddExtraNewElementWizardPages(NewElementWizard* const wizard)
{
    wizard->addPage(new CamerasPage(m_cameraHardware, GetCollection()));
}


const WbSchema CameraCollectionTool::CreateCollectionSchema()
{
    WbSchema schema(CreateWorkbenchSubSchema(camerasSchemaName, tr("Cameras")));
    return schema;
}

void CameraCollectionTool::SetToolSpecificConfigItems(WbConfig newElement, NewElementWizard& wizard)
{
    const CameraDescription chosenCamera(wizard.field(CamerasPage::chosenCameraField)
                                                      .value<CameraDescription>());
    newElement.SetKeyValue(WbDefaultKeys::descriptionKey, KeyValue::from(chosenCamera.ToPlainText()));
    newElement.SetKeyValue(CameraSchema::uniqueIdKey,
                           KeyValue::from(QString::fromStdWString(chosenCamera.UniqueId())));
}

const WbSchema CameraCollectionTool::CreateElementSchema()
{
    WbSchema schema(CreateElementWorkbenchSubSchema(CameraSchema::schemaName, Unnamed("Camera")));
    schema.AddSingleValueKey(CameraSchema::uniqueIdKey, WbSchemaElement::Multiplicity::One);

    schema.AddDependant(CameraPositionSchema::schemaName,
                         CameraPositionSchema::cameraIdKey);

    return schema;
}

