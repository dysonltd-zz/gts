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

#ifndef TRACKTHREAD_H
#define TRACKTHREAD_H

#include <memory>

#include <QtCore/QObject>
#include <QtCore/QMutex>
#include <QtCore/QThread>

class GtsScene;

/** @brief A class to manage video tracking using a separate thread.

    This allows to deal easily with APIs that block until an image is ready.
 **/
class TrackThread : public QObject
{
    Q_OBJECT

public:
    explicit TrackThread( GtsScene& scene );
    ~TrackThread();

    void Run();
    void Pause();
    void Stop();

    void TrackingOn();
    void TrackingOff();

    void SteppingOn();
    void SteppingOff();

    void RunForward();
    void RunBackward();

public slots:
    void Execute();

signals:
	void paused( bool );
    void finished();

	void position( double position );

private:

    bool ShouldTrack() const;
    bool ShouldPause() const;
    bool ShouldStop() const;

    void SetStepFlag();
    void ClearStepFlag();

    void SetPauseFlag();
    void ClearPauseFlag();

    void SetStopFlag();
    void ClearStopFlag();

    void SetTrackFlag();
    void ClearTrackFlag();

    void SetForward();
    void SetBackward();

    void Release();

    GtsScene& m_scene;

    bool m_step;
    bool m_stop;
    bool m_pause;
    bool m_track;
    bool m_paused;
    bool m_forward;

    std::unique_ptr<QThread> m_thread;

    mutable QMutex m_mutex;
};

#endif // TRACKTHREAD_H
