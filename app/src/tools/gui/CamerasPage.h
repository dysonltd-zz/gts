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

#include <QtGui/QStackedWidget>
#include <QtGui/QPushButton>
#include <QtGui/QGridLayout>
#include <QtGui/QRadioButton>

class CameraHardware;
class CameraSelectionFormContents;

class CamerasPage : public NewElementWizardPage
{
    Q_OBJECT
    Q_PROPERTY( CameraDescription chosenCamera READ GetChosenCamera USER true )

public:
    CamerasPage( CameraHardware& cameraHardware, const Collection& camerasCollection );

    virtual void initializePage();
    virtual void cleanupPage();
    virtual bool isComplete() const;

    static const QString chosenCameraField;

private slots:
    void TryToDisplayCameras();

private:
    enum CameraSelectionPageRow
    {
        topSpacerRow,
        labelRow,
        middleSpacerRow,
        buttonRow,
        bottomSpacerRow
    };

    enum CameraSelectionPageColumn
    {
        leftSpacerColumn1,
        leftSpacerColumn2,
        buttonColumn,
        rightSpacerColumn1,
        rightSpacerColumn2
    };

    static const CameraSelectionPageColumn labelStartColumn = leftSpacerColumn2;
    static const int labelColumnSpan = ((int)rightSpacerColumn1-(int)labelStartColumn)+1;

    void RemovePreviouslyChosenCameras( CameraApi::CameraList& connectedCameras );

    void CreateAndAddCameraSelectionPage( QStackedWidget& stackedWidget );
    void CreateAndAddRefreshPage        ( QStackedWidget& stackedWidget );
    void AddVerticalSpacer              ( QGridLayout& gridLayout,
                                          const CameraSelectionPageRow row );
    void AddHorizontalSpacer            ( QGridLayout& gridLayout,
                                          const CameraSelectionPageColumn column );
    void AddSpacer                      ( QGridLayout& gridLayout,
                                          const CameraSelectionPageRow row,
                                          const CameraSelectionPageColumn column,
                                          const QSizePolicy::Policy& horizPolicy,
                                          const QSizePolicy::Policy& vertPolicy );
    const CameraDescription GetChosenCamera() const;

    CameraHardware&              m_cameraHardware;
    QWidget*                     m_cameraOrFileWidget;
    QStackedWidget*              m_pages;
    CameraSelectionFormContents* m_cameraSelectionPage;
    QWidget*                     m_refreshPage;
    QRadioButton*                m_fromLiveCameraBtn;
    QRadioButton*                m_fromFileCameraBtn;
    Collection                   m_camerasCollection;
};

