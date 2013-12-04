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

#include "NewElementWizard.h"
#include "Collection.h"
#include "CameraApi.h"
#include "CameraDescription.h"

#include <QtGui/QPushButton>
#include <QtGui/QGridLayout>
#include <QtGui/QRadioButton>

class CameraHardware;
class CameraSelectionFormContents;

class CamerasPage : public NewElementWizardPage
{
    Q_OBJECT
    Q_PROPERTY(CameraDescription chosenCamera READ GetChosenCamera USER true)

public:
    static const QString chosenCameraField;

    /**
     @bug sometimes doesn't allow finish to be pressed when camera is default-selected
     until you double-click the camera
     */
    CamerasPage(CameraHardware& cameraHardware, const Collection& camerasCollection);

    virtual void initializePage();
    virtual void cleanupPage();
    virtual bool isComplete() const;

private:
    void RemovePreviouslyChosenCameras(CameraApi::CameraList& connectedCameras);
    void AddCameraSelectionPage(QLayout* layout);

    const CameraDescription GetChosenCamera() const;

    Collection                   m_camerasCollection;
    CameraHardware&              m_cameraHardware;
    QWidget*                     m_cameraPageWidget;
    CameraSelectionFormContents* m_cameraSelectionContent;
};

