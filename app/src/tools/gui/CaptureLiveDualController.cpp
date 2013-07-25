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

#include "CaptureLiveDualController.h"

#include "Message.h"
#include "Tool.h"
#include "ImageView.h"
#include "CameraHardware.h"
#include "CameraDescription.h"
#include "VideoSource.h"
#include "CameraTools.h"
#include "FileUtilities.h"

#include <QtCore/QDir>

#if defined(__MINGW32__) || defined(__GNUC__)
#include <Callback.h>
#else
#include <functional>
#endif

namespace
{
    void DisableAllChildrenExcept( QList<QWidget*> const childrenToKeepEnabled,
                                   QWidget* const parentWidget )
    {
        if ( parentWidget )
        {
            foreach( QWidget* childWidget, parentWidget->findChildren< QWidget* >() )
            {
                childWidget->setEnabled( false );
            }
            foreach( QWidget* widgetToReEnable, childrenToKeepEnabled )
            {
                while ( widgetToReEnable != 0 )
                {
                    widgetToReEnable->setEnabled( true );
                    widgetToReEnable = widgetToReEnable->parentWidget();
                }
            }
        }
    }

    void EnableAllChildren( QWidget* const parentWidget )
    {
        if ( parentWidget )
        {
            foreach( QWidget* childWidget, parentWidget->findChildren< QWidget* >() )
            {
                childWidget->setEnabled( true );
            }
        }
    }
}

CaptureLiveDualController::CaptureLiveDualController( QPushButton&    captureLiveBtn,
                                                      QPushButton&    captureCancelBtn,
                                                      Tool&           toolWidget,
                                                      CameraHardware& cameraHardware ) :
    m_captureLiveBtn  ( captureLiveBtn ),
    m_captureCancelBtn( captureCancelBtn ),
    m_toolWidget      ( toolWidget ),
    m_capturingLive   ( false ),
    m_cameraHardware  ( cameraHardware ),
    m_videoSource1    (),
    m_videoSource2    (),
    m_liveView1       ( 0 ),
    m_liveView2       ( 0 )
{
}

#if defined(__MINGW32__) || defined(__GNUC__)
    CaptureLiveDualController::~CaptureLiveDualController() = default;
#endif

bool CaptureLiveDualController::CaptureLiveBtnClicked( const WbConfig& cameraConfig1,
                                                             const WbConfig& cameraConfig2,
                                                             const QString& newImageFileNameFormat,
#if defined(__MINGW32__) || defined(__GNUC__)
															 CreateStreamViewCallback* createStreamView1,
															 CreateStreamViewCallback* createStreamView2 )
#else
                                                             CreateStreamViewCallback createStreamView1,
                                                             CreateStreamViewCallback createStreamView2 )
#endif
{

    bool captured = false;

    if ( CurrentlyStreamingLiveSource() )
    {
        captured = CaptureImageAndStopStreamingLiveSource( newImageFileNameFormat );
    }
    else
    {
#if defined(__MINGW32__) || defined(__GNUC__)
        TryToStartStreamingLiveSource( cameraConfig1,
		                               cameraConfig2,
                                       std::unique_ptr< CreateStreamViewCallback >(
                                                       createStreamView1 ),
                                       std::unique_ptr< CreateStreamViewCallback >(
                                                       createStreamView2 ) );
#else
        TryToStartStreamingLiveSource(cameraConfig1,
                                      cameraConfig2,
		                              createStreamView1,
		                              createStreamView2);
#endif
    }

    // NB: we need to check if Starting succeeded
    if ( CurrentlyStreamingLiveSource() )
    {
        m_captureLiveBtn.setText( tr( "&Capture" ) );
    }
    else
    {
        m_captureLiveBtn.setText( tr( "Li&ve" ) );
        m_captureLiveBtn.setEnabled(false);
    }

    return captured;
}

const void CaptureLiveDualController::CaptureCancelBtnClicked()
{
    if ( CurrentlyStreamingLiveSource() )
    {
        m_videoSource1.reset();
        m_videoSource2.reset();
        m_liveView1->SetCaption( "" );
        m_liveView2->SetCaption( "" );

        m_captureLiveBtn.setText( tr( "Li&ve" ) );
    }
}

void CaptureLiveDualController::TryToStartStreamingLiveSource(
    const WbConfig& cameraConfig1,
    const WbConfig& cameraConfig2,
#if defined(__MINGW32__) || defined(__GNUC__)
	std::unique_ptr< Callback_1< ImageView* const, const QSize& > > createStreamView1,
	std::unique_ptr< Callback_1< ImageView* const, const QSize& > > createStreamView2 )
#else
    CreateStreamViewCallback createStreamView1,
    CreateStreamViewCallback createStreamView2 )
#endif
{
    CameraDescription camera1( CameraTools::GetCameraForStreamingIfOk( m_cameraHardware,
                                                                       cameraConfig1 ) );
    CameraDescription camera2( CameraTools::GetCameraForStreamingIfOk( m_cameraHardware,
                                                                       cameraConfig2 ) );
    if ( camera1.IsValid() && camera2.IsValid() )
    {
#if defined(__MINGW32__) || defined(__GNUC__)
        m_liveView1 = (*createStreamView1)( camera1.GetImageSize() );
        m_liveView2 = (*createStreamView2)( camera2.GetImageSize() );
#else
        m_liveView1 = createStreamView1( camera1.GetImageSize() );
        m_liveView2 = createStreamView2( camera2.GetImageSize() );
#endif
        if ( m_liveView1 && m_liveView2 )
        {
            m_videoSource1.reset( new VideoSource( camera1, *m_liveView1 ) );
            m_videoSource2.reset( new VideoSource( camera2, *m_liveView2 ) );

            m_videoSource1->StartUpdatingImage();
            m_videoSource2->StartUpdatingImage();
        }
    }
}

const QString CaptureLiveDualController::CaptureImage( const QImage capturedImage,
                                                       const QString& newImageFileNameFormat )
{
    QString capturedFileName;

    const QString fileNameToCapture(
                    FileUtilities::GetUniqueFileName( newImageFileNameFormat ) );

    const QFileInfo fileInfo( fileNameToCapture );
    const QString fileDirPath( fileInfo.absolutePath() );
    const bool mkPathSuccessful = QDir().mkpath( fileDirPath );
    bool saveSuccessful = false;

    if ( mkPathSuccessful )
    {
        saveSuccessful = capturedImage.save( fileNameToCapture );
    }

    if ( saveSuccessful )
    {
        capturedFileName = fileNameToCapture;
    }
    else
    {
        Message::Show( &m_toolWidget,
                       tr( "Capture Live (Dual) Controller" ),
                       tr( "Error - Cannot write to: %1!")
                           .arg( fileNameToCapture ),
                       Message::Severity_Critical );
    }

    return capturedFileName;
}

bool CaptureLiveDualController::CaptureImageAndStopStreamingLiveSource(
                                            const QString& newImageFileNameFormat )
{
    m_videoSource1.reset();
    m_videoSource2.reset();
    m_liveView1->SetCaption( "" );
    m_liveView2->SetCaption( "" );

    if ( m_liveView1 && m_liveView2 )
    {
        const QImage capturedImage1 = m_liveView1->GetCurrentImage();
        const QImage capturedImage2 = m_liveView2->GetCurrentImage();

        m_capturedFileName1 = CaptureImage( capturedImage1,
                                            newImageFileNameFormat );

        m_capturedFileName2 = CaptureImage( capturedImage2,
                                            newImageFileNameFormat );
    }

    m_toolWidget.ReloadCurrentConfig();

    return !m_capturedFileName1.isEmpty() &&
           !m_capturedFileName2.isEmpty();
}

bool CaptureLiveDualController::CurrentlyStreamingLiveSource() const
{
    return m_videoSource1.get() != 0 ||
           m_videoSource2.get() != 0;
}
