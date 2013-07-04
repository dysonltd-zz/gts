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

#include "TrackThread.h"

#include <iostream>

#include "GtsScene.h"

#include "OpenCvTools.h"

#include <opencv/highgui.h>

#include <QtCore/QTime>
#include <QtCore/qthread.h>
#include <QtCore/qmutex.h>
#include <QtGui/qapplication.h>

#include "Debugging.h"

/** @brief Create a TrackThread tracking using the specified VideoSequence
 *
 * @param videoSeq The VideoSequence to capture from.
 */
TrackThread::TrackThread( GtsScene& scene ) :
    m_scene   ( scene ),
    m_step    ( false ),
    m_stop    ( false ),
    m_pause   ( false ),
    m_track   ( true ),
    m_paused  ( false ),
    m_forward ( true ),
    m_thread  ( new QThread() ),
    m_mutex   ()
{
    QObject::connect( m_thread.get(),
                      SIGNAL( started() ),
                      this,
                      SLOT( Execute() ) );

    moveToThread( m_thread.get() );

    m_thread->start();
}

/** @brief Destroy after waiting for the actual tracking to stop
 *
 */
TrackThread::~TrackThread()
{
    Stop();
}

/** @brief Slot to start and continuously capture new images from the VideoSequence.
 *
 *  And emit the GotImage() signal when we have a new image, and the finished()
 *  signal at the end.
 */
void TrackThread::Execute()
{
    while ( !ShouldStop() )
    {
        GtsScene::TrackStatus status = m_scene.StepTrackers( m_forward );

        bool trackingLost = ( status.numTrackersActive ==
                              status.numTrackersLost );

        bool gameOver = status.eof;

        emit position( status.videoPosition );

        if ( ShouldPause() || ( ShouldTrack() && trackingLost) )
        {
            m_paused = true;

            emit paused( trackingLost );
        }

		while ( m_paused )
		{
			if ( ShouldStop() ) { break; }
		}

		if (gameOver)
		{
		    break;
		}
	}

    emit finished();
    m_thread->quit();
}

void TrackThread::SetStepFlag()
{
    QMutexLocker mutexLock( &m_mutex );
    m_step = true;
}

void TrackThread::ClearStepFlag()
{
    QMutexLocker mutexLock( &m_mutex );
    m_step = false;
}

void TrackThread::SetPauseFlag()
{
    QMutexLocker mutexLock( &m_mutex );
    m_pause = true;
}

void TrackThread::ClearPauseFlag()
{
    QMutexLocker mutexLock( &m_mutex );
    m_pause = false;
}

void TrackThread::SetStopFlag()
{
    QMutexLocker mutexLock( &m_mutex );
    m_stop = true;
}

void TrackThread::ClearStopFlag()
{
    QMutexLocker mutexLock( &m_mutex );
    m_stop = false;
}

void TrackThread::SetTrackFlag()
{
    QMutexLocker mutexLock( &m_mutex );
    m_track = true;
}

void TrackThread::ClearTrackFlag()
{
    QMutexLocker mutexLock( &m_mutex );
    m_track = false;
}

void TrackThread::SetForward()
{
    QMutexLocker mutexLock( &m_mutex );
    m_forward = true;
}

void TrackThread::SetBackward()
{
    QMutexLocker mutexLock( &m_mutex );
    m_forward = false;
}


void TrackThread::Release()
{
    QMutexLocker mutexLock( &m_mutex );
    m_paused = false;
}


void TrackThread::Run()
{
    ClearPauseFlag();

    Release();
}

void TrackThread::Pause()
{
    SetPauseFlag();
}

/** @brief Stop Tracking images.
 *
 *  And wait until the tracking thread has stopped.  This must be called from the
 *  main application thread.  Sets a mutex-protected flag and waits for the tracking
 *  thread to check it and exit.
 */
void TrackThread::Stop()
{
    // Runs in main thread!!
    if ( m_thread && m_thread->isRunning() )
    {
        if (!m_stop) //already stopping
        {
            assert( QThread::currentThread() != m_thread.get() );

            SetStopFlag();
            thread()->wait();
        }
    }
}


void TrackThread::TrackingOn()
{
    SetTrackFlag();
}

void TrackThread::TrackingOff()
{
    ClearTrackFlag();
}

void TrackThread::SteppingOn()
{
    SetStepFlag();
}

void TrackThread::SteppingOff()
{
    ClearStepFlag();
}

void TrackThread::RunForward()
{
    SetForward();
}

void TrackThread::RunBackward()
{
    SetBackward();
}


bool TrackThread::ShouldPause() const
{
    QMutexLocker mutexLock( &m_mutex );
    return m_pause || m_step;
}

/** @brief Check if we should stop tracking.
 *
 * Checks a mutex-protected flag.
 *
 * @return Whether we should stop tracking.
 */
bool TrackThread::ShouldStop() const
{
    QMutexLocker mutexLock( &m_mutex );
    return m_stop;
}

bool TrackThread::ShouldTrack() const
{
    QMutexLocker mutexLock( &m_mutex );
    return m_track;
}
