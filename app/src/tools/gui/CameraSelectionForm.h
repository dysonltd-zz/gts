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

#include <QtGui/QDialog>
#include <QtCore/QString>
#include <QtCore/QTimer>
#include <QtGui/QTableWidget>

#include "CameraApi.h"

#include <memory>

namespace Ui
{
    class CameraSelectionForm;
}

class CameraSelectionFormContents;

/** @brief Provides a dialog displaying a preview image for connected
 *  cameras and allows user to select from them.
 */
class CameraSelectionForm : private QDialog
{
    Q_OBJECT

public:
    CameraSelectionForm( QWidget* const parent );
    virtual ~CameraSelectionForm();

    const CameraDescription ChooseConnectedCamera( const CameraApi::CameraList& cameras );

private:
    Ui::CameraSelectionForm* m_ui;

    CameraSelectionFormContents* m_contents;
};

#endif // CAMERASELECTIONFORM_H_
