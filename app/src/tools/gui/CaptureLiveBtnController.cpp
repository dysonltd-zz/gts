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

#include "CaptureLiveBtnController.h"

#include "Message.h"
#include "Tool.h"
#include "ImageView.h"
#include "CameraHardware.h"
#include "CameraDescription.h"
#include "VideoSource.h"
#include "CameraTools.h"
#include "FileUtilities.h"

#include <QtCore/qdir.h>

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

CaptureLiveBtnController::CaptureLiveBtnController( QPushButton&    captureLiveBtn,
                                                    QPushButton&    captureCancelBtn,
                                                    Tool&           toolWidget,
                                                    CameraHardware& cameraHardware ) :
    m_captureLiveBtn  ( captureLiveBtn ),
    m_captureCancelBtn( captureCancelBtn ),
    m_toolWidget      ( toolWidget ),
    m_capturingLive   ( false ),
    m_cameraHardware  ( cameraHardware ),
    m_videoSource     (),
    m_liveView        ( 0 )
{
}

void CaptureLiveBtnController::TryToStartStreamingLiveSource(
    const WbConfig& cameraConfig,
#if defined(__MINGW32__) || defined(__GNUC__)
	std::auto_ptr< Callback_1< ImageView* const, const QSize& > > createStreamView )
#else
    CreateStreamViewCallback createStreamView)
#endif
{
    CameraDescription camera( CameraTools::GetCameraForStreamingIfOk( m_cameraHardware,
                                                                      cameraConfig ) );
    if ( camera.IsValid() )
    {
#if defined(__MINGW32__) || defined(__GNUC__)
        m_liveView = (*createStreamView)( camera.GetImageSize() );
#else
        m_liveView = createStreamView( camera.GetImageSize() );
#endif
        if ( m_liveView )
        {
            m_videoSource.reset( new VideoSource( camera, *m_liveView ) );
            m_videoSource->StartUpdatingImage( -1.0 );
        }
    }
}

const QString CaptureLiveBtnController::CaptureLiveBtnClicked( const WbConfig& cameraConfig,
                                                               const QString& newImageFileNameFormat,
#if defined(__MINGW32__) || defined(__GNUC__)
															   CreateStreamViewCallback* createStreamView  )
#else
                                                               CreateStreamViewCallback createStreamView  )
#endif
{
    QString capturedFileName;

    if ( CurrentlyStreamingLiveSource() )
    {
        capturedFileName =
                CaptureImageAndStopStreamingLiveSource( newImageFileNameFormat );
    }
    else
    {
#if defined(__MINGW32__) || defined(__GNUC__)
        TryToStartStreamingLiveSource( cameraConfig,
                                       std::auto_ptr< CreateStreamViewCallback >(
                                                       createStreamView ) );
#else
        TryToStartStreamingLiveSource(cameraConfig, createStreamView);
#endif
    }

    // NB: we need to check if Starting succeeded
    if ( CurrentlyStreamingLiveSource() )
    {
        m_captureLiveBtn.setText( tr( "&Capture" ) );
        DisableAllChildrenExcept( QList<QWidget*>() << &m_captureLiveBtn << &m_captureCancelBtn << m_liveView,
                                  &m_toolWidget );
    }
    else
    {
        m_captureLiveBtn.setText( tr( "Li&ve" ) );
        EnableAllChildren( &m_toolWidget );
    }

    return capturedFileName;
}

const void CaptureLiveBtnController::CaptureCancelBtnClicked()
{
    if ( CurrentlyStreamingLiveSource() )
    {
        m_videoSource.reset();
        m_liveView->SetCaption( "No Camera Streaming" );
        m_captureLiveBtn.setText( tr( "Li&ve" ) );
    }
}

const QString CaptureLiveBtnController::CaptureImageAndStopStreamingLiveSource(
                                            const QString& newImageFileNameFormat )
{
    QString capturedFileName;
    m_videoSource.reset();
    m_liveView->SetCaption( "" );

    if( m_liveView )
    {
        const QImage capturedImage = m_liveView->GetCurrentImage();
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
                           tr( "Capture Live (Btn) Controller" ),
                           tr( "Error - Cannot write to: %1!" )
                               .arg( fileNameToCapture ),
                           Message::Severity_Critical );
        }

    }
    m_toolWidget.ReloadCurrentConfig();

    return capturedFileName;
}



bool CaptureLiveBtnController::CurrentlyStreamingLiveSource() const
{
    return m_videoSource.get() != 0;
}
