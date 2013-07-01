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

#include "CameraDescription.h"
#include "CameraHardware.h"

#include <QtCore/QStringBuilder>
#include <QtGui/QGridLayout>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>
#include <QtGui/qapplication.h>

namespace
{
    class MatchesAny
    {
    public:
        MatchesAny( const Collection& camerasCollection ) :
            m_camerasCollection( camerasCollection ) {}

        bool operator() ( const CameraDescription& description ) const
        {
            return m_camerasCollection.AnyElementHas( KeyName( "uniqueId" ),
                                                      KeyValue::from( description.UniqueId() )
                                                      );
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
    m_cameraHardware     ( cameraHardware ),
    m_cameraOrFileWidget ( new QWidget( this ) ),
    m_pages              ( new QStackedWidget( this ) ),
    m_cameraSelectionPage( 0 ),
    m_refreshPage        ( 0 ),
    m_fromLiveCameraBtn  (new QRadioButton(tr("&Live camera"))),
    m_fromFileCameraBtn  (new QRadioButton(tr("&Offline camera"))),
    m_camerasCollection  ( camerasCollection )
{
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget( m_cameraOrFileWidget );
    QVBoxLayout* cameraOrFileLayout = new QVBoxLayout;
    m_cameraOrFileWidget->setLayout(cameraOrFileLayout);
    cameraOrFileLayout->addWidget(m_fromLiveCameraBtn);
    cameraOrFileLayout->addWidget(m_pages);
    cameraOrFileLayout->addWidget(m_fromFileCameraBtn);
    m_fromLiveCameraBtn->setChecked(true);
    m_fromLiveCameraBtn->setToolTip(tr("Set up a connected camera so images can be saved from a live stream or loaded from files."));
    m_fromFileCameraBtn->setToolTip(tr("Load all camera images and videos from files."));

    QObject::connect( m_fromLiveCameraBtn,
                      SIGNAL( clicked() ),
                      this,
                      SIGNAL( completeChanged() ) );
    QObject::connect( m_fromFileCameraBtn,
                      SIGNAL( clicked() ),
                      this,
                      SIGNAL( completeChanged() ) );

    CreateAndAddCameraSelectionPage( *m_pages );
    CreateAndAddRefreshPage        ( *m_pages );

    setLayout( mainLayout );
}

void CamerasPage::initializePage()
{
    wizard()->setOption( QWizard::HaveHelpButton, false );
    wizard()->setOption( QWizard::HelpButtonOnRight, false );

    TryToDisplayCameras();
}

bool CamerasPage::isComplete() const
{
    return field( chosenCameraField ).value<CameraDescription>().IsValid();
}

void CamerasPage::cleanupPage()
{
    wizard()->setOption( QWizard::HaveCustomButton1, false );

    m_cameraSelectionPage->Shutdown();
}

void CamerasPage::TryToDisplayCameras()
{
    CameraApi::CameraList connectedCameras(m_cameraHardware.EnumerateConnectedCameras());
    RemovePreviouslyChosenCameras( connectedCameras );
    const bool startUpSucceeded = m_cameraSelectionPage->StartUp( connectedCameras, -1.0 );

    if ( startUpSucceeded )
    {
        m_pages->setCurrentWidget( m_cameraSelectionPage );
    }
    else
    {
        m_pages->setCurrentWidget( m_refreshPage );
    }
}

void CamerasPage::CreateAndAddCameraSelectionPage( QStackedWidget& stackedWidget )
{
    m_cameraSelectionPage = new CameraSelectionFormContents( &stackedWidget );

    registerField(chosenCameraField % mandatoryFieldSuffix, this,
                  "chosenCamera", SIGNAL(completeChanged()));

    QObject::connect( m_cameraSelectionPage,
                      SIGNAL( CameraChosen() ),
                      this,
                      SIGNAL( completeChanged() ) );

    stackedWidget.addWidget( m_cameraSelectionPage );
}

void CamerasPage::AddSpacer( QGridLayout& gridLayout,
                             const CameraSelectionPageRow row,
                             const CameraSelectionPageColumn column,
                             const QSizePolicy::Policy& horizPolicy,
                             const QSizePolicy::Policy& vertPolicy )
{
    QSpacerItem* verticalSpacer = new QSpacerItem( 20, 20, horizPolicy, vertPolicy );

    const int oneCellSpan = 1;
    gridLayout.addItem( verticalSpacer, row, column, oneCellSpan, oneCellSpan );
}

const CameraDescription CamerasPage::GetChosenCamera() const
{
    if (m_fromLiveCameraBtn->isChecked())
    {
        return m_cameraSelectionPage->GetChosenCamera();
    }
    else
    {
        return CameraDescription::CreateOffline();
    }
}

void CamerasPage::AddVerticalSpacer( QGridLayout& gridLayout,
                                     const CameraSelectionPageRow row )
{
    AddSpacer( gridLayout,
               row,
               buttonColumn,
               QSizePolicy::Minimum, QSizePolicy::Expanding );
}

void CamerasPage::AddHorizontalSpacer( QGridLayout& gridLayout,
                                       const CameraSelectionPageColumn column )
{
    AddSpacer( gridLayout,
               buttonRow,
               column,
               QSizePolicy::Expanding, QSizePolicy::Minimum );
}

void CamerasPage::CreateAndAddRefreshPage( QStackedWidget& stackedWidget )
{
    m_refreshPage = new QWidget( &stackedWidget );

    QGridLayout* refreshPageLayout = new QGridLayout;

    AddVerticalSpacer( *refreshPageLayout, topSpacerRow );
    AddVerticalSpacer( *refreshPageLayout, middleSpacerRow );
    AddVerticalSpacer( *refreshPageLayout, bottomSpacerRow );

    AddHorizontalSpacer( *refreshPageLayout, leftSpacerColumn1 );
    AddHorizontalSpacer( *refreshPageLayout, leftSpacerColumn2 );
    AddHorizontalSpacer( *refreshPageLayout, rightSpacerColumn1 );
    AddHorizontalSpacer( *refreshPageLayout, rightSpacerColumn2 );

    QPushButton* const refreshButton = new QPushButton( tr( "&Try Again" ),
                                                        m_refreshPage );

    QLabel* iconLabel = new QLabel();
    iconLabel->setPixmap( QApplication::style()
                            ->standardIcon( QStyle::SP_MessageBoxCritical )
                            .pixmap( 32, 32, QIcon::Normal, QIcon::On ) );

    QLabel* const infoLabel = new QLabel( tr( "There are no available connected cameras."
                                              " This may be because of a faulty"
                                              " connection, or because all the"
                                              " connected cameras are already registered."
                                              " If you expect to see a camera that is not "
                                              " already registered, check its connections and try again." ) );
    infoLabel->setWordWrap( true );
    QFont labelFont( infoLabel->font() );
    labelFont.setWeight( QFont::DemiBold );
    infoLabel->setFont( labelFont );

    QHBoxLayout* labelLayout = new QHBoxLayout;
    labelLayout->addWidget( iconLabel, 0, Qt::AlignTop );
    labelLayout->addWidget( infoLabel );

    const int singleRowSpan = 1;
    refreshPageLayout->addItem( labelLayout,
                                labelRow,
                                labelStartColumn,
                                singleRowSpan,
                                labelColumnSpan );

    refreshPageLayout->addWidget( refreshButton, buttonRow, buttonColumn );

    m_refreshPage->setLayout( refreshPageLayout );

    QObject::connect( refreshButton,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( TryToDisplayCameras() ) );

    stackedWidget.addWidget( m_refreshPage );
}

void CamerasPage::RemovePreviouslyChosenCameras( CameraApi::CameraList& connectedCameras )
{
    const CameraApi::CameraList::iterator newEnd =
        std::remove_if( connectedCameras.begin(),
                        connectedCameras.end(),
                        MatchesAny( m_camerasCollection ) );

    connectedCameras.erase( newEnd, connectedCameras.end() );
}

