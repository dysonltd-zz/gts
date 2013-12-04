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

#ifndef CAPTURETHREAD_H
#define CAPTURETHREAD_H

#include <opencv/cv.h>

#include <QtGui/QImage>
#include <QtCore/QObject>
#include <QtCore/QMutex>
#include <QtCore/QThread>

#include <memory>

#if defined(__MINGW32__) || defined(_MSC_VER)
    #include <WinTime.h>
#else
    #include <time.h>
#endif

class VideoSequence;

/** @brief A class to manage capturing video using a separate thread.
 *
 *  This allows to deal easily with APIs that blocl until an image is ready.
 */
class CaptureThread : public QObject
{
    Q_OBJECT
public:
    CaptureThread(VideoSequence* const videoSeq);
    ~CaptureThread();

    void StopCapturing();

public slots:
    void run();

signals:
    /** @brief indicate that the capturing has stopped.
     */
    void finished();
    /** @brief Signal the arrival of a new image.
     *
     * @param newImage The newly received image.
     * @param system timestamp of image
     */
    //void GotImage(const QImage& newImage, const QTime& stamp);
    void GotImage(const QImage& newImage, const timespec stamp, const double fps);
private:
    bool ShouldStopCapturing() const;
    void UpdateQImage();
    void SetStopCapturingFlag();

    std::unique_ptr<VideoSequence> m_videoSeq;

    bool m_stopCapturing;

    const IplImage* m_internalImage;
    QImage  m_image;

    mutable QMutex m_capturingMutex;

    std::unique_ptr<QThread> m_thread;
};

#endif // CAPTURETHREAD_H
