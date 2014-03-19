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

#ifndef CAMERASELECTIONFORMCONTENTS_H
#define CAMERASELECTIONFORMCONTENTS_H

#include "CameraApi.h"
#include "CameraDescription.h"

#include <QtGui/QWidget>
#include <QtGui/QTableWidget>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QPushButton>

#include <memory>

namespace Ui
{
    class CameraSelectionFormContentsClass;
}

class ImageView;
class VideoSource;

class CameraSelectionFormContents : public QWidget
{
    Q_OBJECT

public:
    CameraSelectionFormContents( QWidget* parent = 0 );
    virtual ~CameraSelectionFormContents();

    void AddHelpBtn( QPushButton* const button );

    const CameraDescription GetChosenCamera() const;

    bool StartUp( const CameraApi::CameraList& cameras );
    void Shutdown();

    bool HasCameras() const;

signals:
    void ChosenCameraChanged();
    void CameraChosen();

private slots:
    void SelectedCameraChanged( QTableWidgetItem* current, QTableWidgetItem* previous );

private:
    void FillOutCameraList();

    QTableWidgetItem* const CreateTableItemForCamera( const size_t cameraIndex ) const;

    void UpdateChosenCamera( QTableWidgetItem* current );
    void UpdatePreview();

    void InvalidateChosenCamera();
    bool ChosenCameraIsValid() const;

    std::auto_ptr< Ui::CameraSelectionFormContentsClass > m_ui; ///< The UI class created by Qt designer.
    size_t                       m_chosenCamera;  ///< The index of the currently-selected camera.
    CameraApi::CameraList        m_cameras;       ///< The list of camera to select from.
    std::auto_ptr< VideoSource > m_preview;       ///< The object which is obtaining camera images for the preview.
    QImage                       m_previewImage;  ///< The QImage where the preview is being drawn.
};

#endif // CAMERASELECTIONFORMCONTENTS_H
