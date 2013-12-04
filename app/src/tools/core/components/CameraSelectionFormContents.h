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
    CameraSelectionFormContents(QWidget* parent = 0);
    virtual ~CameraSelectionFormContents();

    /**
     * @brief Check if camera is valid and return it
     * @return Camera
     */
    const CameraDescription GetChosenCamera() const;

    /**
      @brief Create a contents listing connected cameras to select from with a preview.

      @param cameras List of a description of each camera to display.
      @param fps     The frame rate to use for the preview image (or as close as possible).
      @return @a true if the widget is able to start correctly (there are valid cameras),
      otherwise @a false.
    **/
    bool StartUp(const CameraApi::CameraList& cameras);

    /**
      @brief Invalidate chosen camera and turn off preview
    **/
    void Shutdown();

    /**
      @brief Check if the are any cameras registered
      @return True if there are cameras registered
     **/
    bool HasCameras() const;

signals:
    void ChosenCameraChanged();
    void CameraChosen();

private slots:
    /**
      @brief Handle the user selecting a new camera.
    **/
    void SelectedCameraChanged(QTableWidgetItem* current, QTableWidgetItem* previous);

private:
    /**
      @brief Populate the table widget with the details of the cameras
    **/
    void FillOutCameraList();

    /**
      @brief Create a table widget item for a single camera.

      @param  cameraIndex The index in @a m_cameras of the camera to create an item for.
      @return The new QTableWidgetItem. The caller takes ownership.
    **/
    QTableWidgetItem* const CreateTableItemForCamera(const size_t cameraIndex) const;

    /**
      @brief Change chosen camera from a newly-selected QTableWidgetItem.
      If the newly select item is null, invalidate the m_chosenCamera index.
      Otherwise (normally) retrieve the appropriate camera index from the item.
      @param newItem The newly selected QTableWidgetItem, or null if none selected
    **/
    void UpdateChosenCamera(QTableWidgetItem* current);

    /**
     @brief Switch preview to newly-selected camera.
     Close current camera and open new one. Start updating
     the preview at the frame rate as passed to the constructor.
    **/
    void UpdatePreview();

    /**
      @brief Set the @a m_chosenCamera member to an invalid value.
    **/
    void InvalidateChosenCamera();

    /**
      @brief Test if @a m_chosenCamera is valid
      @return @a true if there is any selected camera, @a false otherwise.
    **/
    bool ChosenCameraIsValid() const;

    std::auto_ptr< Ui::CameraSelectionFormContentsClass > m_ui; ///< The UI class created by Qt designer.
    size_t                       m_chosenCamera;  ///< The index of the currently-selected camera.
    CameraApi::CameraList        m_cameras;       ///< The list of camera to select from.
    std::auto_ptr< VideoSource > m_preview;       ///< The object which is obtaining camera images for the preview.
    QImage                       m_previewImage;  ///< The QImage where the preview is being drawn.
};

#endif // CAMERASELECTIONFORMCONTENTS_H
