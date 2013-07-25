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

#include "CaptureThread.h"

#include <iostream>

#include "VideoSequence.h"
#include "OpenCvTools.h"

#include <opencv/highgui.h>

#include <QtCore/qthread.h>
#include <QtCore/qmutex.h>
#include <QtGui/qapplication.h>

#include "Debugging.h"

/** @brief Create a CaptureThread Capturing using the specified VideoSequence
 *
 * @param videoSeq The VideoSequence to capture from.
 */
CaptureThread::CaptureThread( VideoSequence* const videoSeq ) :
    m_videoSeq      ( videoSeq ),
    m_stopCapturing ( false ),
    m_internalImage ( 0 ),
    m_image         (),
    m_capturingMutex(),
    m_thread        ( new QThread() )
{
    QObject::connect( m_thread.get(),
                      SIGNAL( started() ),
                      this,
                      SLOT( run() ) );

    qRegisterMetaType<timespec>("timespec");

    moveToThread( m_thread.get() );

    m_thread->start();
}

/** @brief Destroy after waiting for the actual capturing to stop
 *
 */
CaptureThread::~CaptureThread()
{
    StopCapturing();
}

/** @brief Slot to start and continuously capture new images from the VideoSequence.
 *
 *  And emit the GotImage() signal when we have a new image, and the finished()
 *  signal at the end.
 */
void CaptureThread::run()
{
    while ( !ShouldStopCapturing() )
    {
        if ( !m_videoSeq.get() )
        {
            continue;
        }
        if ( !m_videoSeq->IsSetup() )
        {
            continue;
        }
        if ( !m_videoSeq->ReadyNextFrame() )
        {
            continue;
        }

        /// @todo grab the system time here (just before
        /// the sequencer grabs a frame from the camera.)

        timespec tspec;

#if defined(__MINGW32__) || defined(_MSC_VER)
        timeval tval;
        clock_gettime( 0, &tval );
        tspec.tv_sec = tval.tv_sec;
        tspec.tv_nsec = tval.tv_usec * 1000;
#else
        clock_gettime( CLOCK_REALTIME, &tspec );
#endif

        m_internalImage = m_videoSeq->RetrieveNextFrame();
        double fps = m_videoSeq->GetFrameRate();

        UpdateQImage();

        emit GotImage( m_image, tspec, fps );
    }

    emit finished();
    m_thread->quit();
}

/** @brief Update the internal @a QImage with from the internal OpenCV @a IplImage.
 *
 */
void CaptureThread::UpdateQImage()
{
    if ( OpenCvTools::IsValid( m_internalImage ) )
    {
        const QSize internalImageSize( m_internalImage->width,
                                       m_internalImage->height );

        if ( m_image.isNull() || ( m_image.size() != internalImageSize ) )
        {
            m_image = QImage( internalImageSize, QImage::Format_RGB888 );
        }

        CvMat mtxWrapper;
        cvInitMatHeader( &mtxWrapper,
                         m_internalImage->height,
                         m_internalImage->width,
                         CV_8UC3,
                         m_image.bits() );

        const int DONT_FLIP = 0;
        int flipFlag = DONT_FLIP;
        cvConvertImage( m_internalImage, &mtxWrapper, flipFlag );
    }
}

/** @brief Actually set the mutex-protected #m_stopCapturing flag.
 */
void CaptureThread::SetStopCapturingFlag()
{
    QMutexLocker capturingMutexLock(&m_capturingMutex);
    m_stopCapturing = true;
}

/** @brief Stop Capturing images.
 *
 *  And wait until the capturing thread has stopped.  This must be called from the
 *  main application thread.  Sets a mutex-protected flag and waits for the capturing
 *  thread to check it and exit.
 */
void CaptureThread::StopCapturing()
{
    // Runs in main thread!!
    if ( m_thread && m_thread->isRunning() )
    {
        if (!m_stopCapturing) //already stopping
        {
            assert( QThread::currentThread() != m_thread.get() );
            SetStopCapturingFlag();
            thread()->wait();
        }
    }
}

/** @brief Check if we should stop capturing.
 *
 *  Checks a mutex-protected flag.
 *
 * @return Whether we should stop capturing.
 */
bool CaptureThread::ShouldStopCapturing() const
{
    QMutexLocker capturingMutexLock( &m_capturingMutex );
    return m_stopCapturing;
}

