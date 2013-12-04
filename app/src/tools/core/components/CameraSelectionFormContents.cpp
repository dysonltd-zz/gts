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

#include "CameraSelectionFormContents.h"

#include "ui_CameraSelectionFormContents.h"

#include "VideoSource.h"
#include "Message.h"

CameraSelectionFormContents::CameraSelectionFormContents(QWidget* parent) :
    QWidget       (parent),
    m_ui          (new Ui::CameraSelectionFormContentsClass),
    m_chosenCamera(0),
    m_cameras     ()
{
    m_ui->setupUi(this);
    InvalidateChosenCamera();

    QObject::connect(m_ui->m_cameraTableWidget,
                     SIGNAL(currentItemChanged(QTableWidgetItem*, QTableWidgetItem*)),
                     this,
                     SLOT(SelectedCameraChanged(QTableWidgetItem*, QTableWidgetItem*)));

    QObject::connect(m_ui->m_cameraTableWidget,
                     SIGNAL(itemDoubleClicked(QTableWidgetItem*)),
                     this,
                     SIGNAL(CameraChosen()));
}

CameraSelectionFormContents::~CameraSelectionFormContents()
{
}

const CameraDescription CameraSelectionFormContents::GetChosenCamera() const
{
    CameraDescription camera;

    if(ChosenCameraIsValid())
    {
        camera = m_cameras.at(m_chosenCamera);
    }

    return camera;
}

bool CameraSelectionFormContents::StartUp(const CameraApi::CameraList& cameras)
{
    m_cameras = cameras;
    FillOutCameraList();

    return HasCameras();
}

void CameraSelectionFormContents::Shutdown()
{
    InvalidateChosenCamera();
    m_preview.reset();
}

bool CameraSelectionFormContents::HasCameras() const
{
    return !m_cameras.empty();
}

void CameraSelectionFormContents::SelectedCameraChanged(QTableWidgetItem* current, QTableWidgetItem* previous)
{
    Q_UNUSED(previous);

    UpdateChosenCamera(current);
    UpdatePreview();
}

void CameraSelectionFormContents::FillOutCameraList()
{
    QTableWidget* const tablewidget = m_ui->m_cameraTableWidget;
    const int ONLY_COLUMN = 0;

    tablewidget->setSortingEnabled(false);
    tablewidget->setRowCount(m_cameras.size());
    tablewidget->setColumnCount(1);

    if (m_cameras.size() > 0)
    {
        m_ui->m_cameraLabel->setText(""); // clear label
    }
    for (size_t cameraIndex = 0; cameraIndex < m_cameras.size(); ++cameraIndex)
    {
        tablewidget->setItem(cameraIndex, ONLY_COLUMN,
                              CreateTableItemForCamera(cameraIndex));
    }

    tablewidget->setCurrentCell(0, 0);
}

QTableWidgetItem* const CameraSelectionFormContents::CreateTableItemForCamera(const size_t cameraIndex) const
{
    const CameraDescription& thisCamera(m_cameras.at(cameraIndex));


    QTableWidgetItem* newItem = new QTableWidgetItem(QString::fromStdWString(thisCamera.Description()));
    newItem->setData(Qt::UserRole, QVariant((int) cameraIndex));
    newItem->setToolTip(thisCamera.ToRichText());
    return newItem;
}

void CameraSelectionFormContents::UpdateChosenCamera(QTableWidgetItem* newItem)
{
    if (newItem)
    {
        m_chosenCamera = newItem->data(Qt::UserRole).value< size_t >();
    }
    else
    {
        InvalidateChosenCamera();
    }
    emit ChosenCameraChanged();
}

void CameraSelectionFormContents::UpdatePreview()
{
    if (ChosenCameraIsValid())
    {
        // clear camera to free up old resources
        m_preview.reset();
        CameraDescription& camera = m_cameras.at(m_chosenCamera);

        // before creating a new one
        m_preview.reset(new VideoSource(camera, *m_ui->m_previewImageWidget));

        m_preview->StartUpdatingImage();
    }
}

void CameraSelectionFormContents::InvalidateChosenCamera()
{
    m_ui->m_cameraTableWidget->clearSelection();
    m_chosenCamera = m_cameras.size();
}

bool CameraSelectionFormContents::ChosenCameraIsValid() const
{
    return m_chosenCamera < m_cameras.size();
}
