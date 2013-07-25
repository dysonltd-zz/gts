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

#include "CamerasPage.h"

#include "CameraSelectionFormContents.h"

#include "CameraSchema.h"
#include "CameraDescription.h"
#include "CameraHardware.h"

#include <QtCore/QStringBuilder>
#include <QtGui/QGridLayout>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>
#include <QtGui/QGroupBox>

namespace
{
    class MatchesAny
    {
    public:
        MatchesAny( const Collection& camerasCollection ) :
            m_camerasCollection( camerasCollection )
        {
        }

        bool operator() ( const CameraDescription& description ) const
        {
            return m_camerasCollection.AnyElementHas( CameraSchema::uniqueIdKey,
                                                      KeyValue::from( description.UniqueId() ));
        }

    private:
        const Collection& m_camerasCollection;
    };
}

const QString CamerasPage::chosenCameraField( "chosenCamera" );

/** @bug sometimes doesn't allow finish to be pressed when camera is default-selected
 * until you double-click the camera
 */
CamerasPage::CamerasPage( CameraHardware&   cameraHardware,
                          const Collection& camerasCollection ) :
    m_cameraHardware         ( cameraHardware ),
    m_camerasCollection      ( camerasCollection ),
    m_cameraPageWidget       ( new QWidget( this ) ),
    m_fromLiveCameraBtn      ( new QRadioButton( tr("&Physical") ) ),
    m_fromFileCameraBtn      ( new QRadioButton( tr("&Virtual") ) ),
    m_cameraSelectionContent ( 0 )
{
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget( m_cameraPageWidget );

    QVBoxLayout* cameraPageLayout = new QVBoxLayout;
    m_cameraPageWidget->setLayout(cameraPageLayout);

    QGroupBox *cameraOrFileGroup = new QGroupBox(m_cameraPageWidget);
    QVBoxLayout* cameraOrFileLayout = new QVBoxLayout;
    cameraOrFileGroup->setLayout(cameraOrFileLayout);

    cameraOrFileLayout->addWidget(m_fromLiveCameraBtn);
    cameraOrFileLayout->addWidget(m_fromFileCameraBtn);

    cameraPageLayout->addWidget(cameraOrFileGroup);

    QGroupBox *cameraSelectGroup = new QGroupBox(m_cameraPageWidget);
    QVBoxLayout* cameraSelectLayout = new QVBoxLayout;
    cameraSelectGroup->setLayout(cameraSelectLayout);

    AddCameraSelectionPage( cameraSelectLayout );

    cameraPageLayout->addWidget(cameraSelectGroup);

    m_fromLiveCameraBtn->setChecked(true);
    m_fromLiveCameraBtn->setToolTip(tr("Stream video direct from a connected camera."));
    m_fromFileCameraBtn->setToolTip(tr("Load all camera images and videos from file."));

    m_fromLiveCameraBtn->setEnabled(true);
    m_fromFileCameraBtn->setEnabled(false);

    QObject::connect( m_fromLiveCameraBtn,
                      SIGNAL( clicked() ),
                      this,
                      SIGNAL( completeChanged() ) );
    QObject::connect( m_fromFileCameraBtn,
                      SIGNAL( clicked() ),
                      this,
                      SIGNAL( completeChanged() ) );

    setLayout( mainLayout );
}

void CamerasPage::AddCameraSelectionPage( QLayout* layout )
{
    m_cameraSelectionContent = new CameraSelectionFormContents();

    registerField(chosenCameraField % mandatoryFieldSuffix,
                  this,
                  "chosenCamera",
                  SIGNAL(completeChanged()));

    QObject::connect( m_cameraSelectionContent,
                      SIGNAL( CameraChosen() ),
                      this,
                      SIGNAL( completeChanged() ) );

    layout->addWidget( m_cameraSelectionContent );
}

void CamerasPage::initializePage()
{
    // Try to display connected (but unused) cameras...
    CameraApi::CameraList connectedCameras(m_cameraHardware.EnumerateConnectedCameras());
    RemovePreviouslyChosenCameras( connectedCameras );
    m_cameraSelectionContent->StartUp( connectedCameras );

    wizard()->setOption( QWizard::HaveHelpButton, false );
    wizard()->setOption( QWizard::HelpButtonOnRight, false );
}

void CamerasPage::RemovePreviouslyChosenCameras( CameraApi::CameraList& connectedCameras )
{
    const CameraApi::CameraList::iterator newEnd =
        std::remove_if( connectedCameras.begin(),
                        connectedCameras.end(),
                        MatchesAny( m_camerasCollection ) );

    connectedCameras.erase( newEnd, connectedCameras.end() );
}

void CamerasPage::cleanupPage()
{
    wizard()->setOption( QWizard::HaveCustomButton1, false );

    m_cameraSelectionContent->Shutdown();
}

bool CamerasPage::isComplete() const
{
    return field( chosenCameraField ).value<CameraDescription>().IsValid();
}

const CameraDescription CamerasPage::GetChosenCamera() const
{
    if (m_fromLiveCameraBtn->isChecked())
    {
        return m_cameraSelectionContent->GetChosenCamera();
    }
    else
    {
        return CameraDescription::CreateOffline();
    }
}
