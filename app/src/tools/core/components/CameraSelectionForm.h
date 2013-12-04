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

#ifndef CAMERASELECTIONFORM_H_
#define CAMERASELECTIONFORM_H_

#include "CameraApi.h"

#include <QtGui/QDialog>
#include <QtCore/QString>
#include <QtCore/QTimer>
#include <QtGui/QTableWidget>

#include <memory>

namespace Ui
{
    class CameraSelectionForm;
}

class CameraSelectionFormContents;

/**
  @brief Provides a dialog displaying a preview image for connected
  cameras and allows user to select from them.
**/
class CameraSelectionForm : private QDialog
{
    Q_OBJECT

public:
    /**
      @brief Create a dialog listing connected cameras to select from with a preview.
    **/
    CameraSelectionForm(QWidget* const parent);
    virtual ~CameraSelectionForm();

    /**
      @brief Display the dialog to select a camera.
      If the camera list is empty, then display a warning message and close.

      @param cameras List of a description of each camera to display.
      @param fps     The frame rate to use for the preview image (or as close as possible).
      @return A CameraDescription describing the camera selected by the user,
      or a an invalid description if the user does not select a camera, there
      are no cameras to select from, or the user cancels.
    **/
    const CameraDescription ChooseConnectedCamera(const CameraApi::CameraList& cameras);

private:
    Ui::CameraSelectionForm* m_ui;
    CameraSelectionFormContents* m_contents;
};

#endif // CAMERASELECTIONFORM_H_
