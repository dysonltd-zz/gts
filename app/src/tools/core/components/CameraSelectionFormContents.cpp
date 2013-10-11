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

CameraSelectionFormContents::CameraSelectionFormContents( QWidget* parent ) :
    QWidget       ( parent ),
    m_ui          ( new Ui::CameraSelectionFormContentsClass ),
    m_chosenCamera( 0 ),
    m_cameras     ()
{
    m_ui->setupUi( this );

    InvalidateChosenCamera();

    QObject::connect( m_ui->m_cameraTableWidget,
                      SIGNAL( currentItemChanged( QTableWidgetItem*,
                                                  QTableWidgetItem* ) ),
                      this,
                      SLOT( SelectedCameraChanged( QTableWidgetItem*,
                                                   QTableWidgetItem*) ) );

    QObject::connect( m_ui->m_cameraTableWidget,
                      SIGNAL( itemDoubleClicked(QTableWidgetItem*) ),
                      this,
                      SIGNAL( CameraChosen() ) );
}

CameraSelectionFormContents::~CameraSelectionFormContents()
{
}

/** @brief Create a contents listing connected cameras to select from with a preview.
 *
 *  @param cameras List of a description of each camera to display.
 *  @param fps     The frame rate to use for the preview image (or as close as possible).
 *  @return @a true if the widget is able to start correctly (there are valid cameras),
 *    otherwise @a false.
**/
bool CameraSelectionFormContents::StartUp( const CameraApi::CameraList& cameras )
{
    m_cameras = cameras;
    FillOutCameraList();

    return HasCameras();
}

/** @brief Set the @a m_chosenCamera member to an invalid value.
 *
 *
**/
void CameraSelectionFormContents::InvalidateChosenCamera()
{
    m_ui->m_cameraTableWidget->clearSelection();
    m_chosenCamera = m_cameras.size();
}

/** @brief Test if @a m_chosenCamera is valid
 *
 *  @return @a true if there is any selected camera, @a false otherwise.
**/
bool CameraSelectionFormContents::ChosenCameraIsValid() const
{
    return m_chosenCamera < m_cameras.size();
}

/**
 * @brief Populate the table widget with the details of the cameras
 *
**/
void CameraSelectionFormContents::FillOutCameraList()
{
    QTableWidget* const tablewidget = m_ui->m_cameraTableWidget;
    const int ONLY_COLUMN = 0;

    tablewidget->setSortingEnabled( false );
    tablewidget->setRowCount( m_cameras.size() );
    tablewidget->setColumnCount( 1 );

    if (m_cameras.size() > 0)
    {
        m_ui->m_cameraLabel->setText(""); // clear label
    }
    for ( size_t cameraIndex = 0; cameraIndex < m_cameras.size(); ++cameraIndex )
    {
        tablewidget->setItem( cameraIndex, ONLY_COLUMN,
                              CreateTableItemForCamera( cameraIndex ) );
    }

    tablewidget->setCurrentCell( 0, 0 );
}

/** @brief Create a table widget item for a single camera.
 *
 * @param  cameraIndex The index in @a m_cameras of the camera to create an item for.
 * @return The new QTableWidgetItem. The caller takes ownership.
**/
QTableWidgetItem* const CameraSelectionFormContents::CreateTableItemForCamera( const size_t cameraIndex ) const
{
    const CameraDescription& thisCamera( m_cameras.at( cameraIndex ) );


    QTableWidgetItem* newItem = new QTableWidgetItem( QString::fromStdWString( thisCamera.Description() ) );
    newItem->setData( Qt::UserRole, QVariant( (int) cameraIndex ) );
    newItem->setToolTip( thisCamera.ToRichText() );
    return newItem;
}

/** @brief Handle the user selecting a new camera.
 *
 *
**/
void CameraSelectionFormContents::SelectedCameraChanged( QTableWidgetItem* current, QTableWidgetItem* previous )
{
    Q_UNUSED(previous);

    UpdateChosenCamera( current );
    UpdatePreview();
}

/** @brief Change chosen camera from a newly-selected QTableWidgetItem.
 *
 *  If the newly select item is null, invalidate the m_chosenCamera index.
 *  Otherwise (normally) retrieve the appropriate camera index from the item.
 *  @param newItem The newly selected QTableWidgetItem, or null if none selected
**/
void CameraSelectionFormContents::UpdateChosenCamera( QTableWidgetItem* newItem )
{
    if ( newItem )
    {
        m_chosenCamera = newItem->data( Qt::UserRole ).value< size_t >();
    }
    else
    {
        InvalidateChosenCamera();
    }
    emit ChosenCameraChanged();
}

/** @brief Switch preview to newly-selected camera.
 *
 *  Close current camera and open new one. Start updating
 *  the preview at the frame rate as passed to the constructor.
**/
void CameraSelectionFormContents::UpdatePreview()
{
    if ( ChosenCameraIsValid() )
    {
        // clear camera to free up old resources
        m_preview.reset();
        CameraDescription& camera = m_cameras.at( m_chosenCamera );

        // before creating a new one
        m_preview.reset( new VideoSource( camera, *m_ui->m_previewImageWidget ) );

        m_preview->StartUpdatingImage();
    }
}

const CameraDescription CameraSelectionFormContents::GetChosenCamera() const
{
    CameraDescription camera;

    if( ChosenCameraIsValid() )
    {
        camera = m_cameras.at( m_chosenCamera );
    }

    return camera;
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
