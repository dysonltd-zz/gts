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

#ifndef VIDEOSOURCE_H
#define VIDEOSOURCE_H

#include "CameraDescription.h"
#include <memory>

#include <QtGui/qimage.h>
#include <opencv/cv.h>
#include <QtGui/QTableWidget>
#include <QtCore/QTime>
#include <opencv/highgui.h>
#include <QtCore/qobject.h>
#include <QLineEdit>

#if defined(__MINGW32__) || defined(_MSC_VER)
    #include <WinTime.h>
#endif

class AviWriter;
class ImageView;
class VideoSequence;

class CaptureThread;

/** @brief Class to encapsulate capturing from a camera
 *  and displaying the image in an ImageView.
 *
 *  Also displays the frame rate in the ImageView caption.
 */
class VideoSource : public QObject
{
    Q_OBJECT
public:
    VideoSource(const CameraDescription& camera, ImageView& imageView , QLineEdit* recordingTimer = 0);
    ~VideoSource();

    void RecordTo(std::unique_ptr<AviWriter>&& videoWriter);
    void StopRecording();
    bool IsRecording() const;

    void StartUpdatingImage(const double fps = -1.0);
    void StopUpdatingImage();

    const QSize GetImageSize() const;

    bool IsFrom( const CameraDescription& cameraDescription ) const;

private slots:
    void ResetCapture();
    void UpdateDisplayedImage(const QImage newImage , timespec stamp);

private:

    void SetImageAndUpdateFpsDisplay();
    void UpdateRecordingTimer();

    static const size_t NUM_FRAMES_TO_AVERAGE = 10;
    CameraDescription              m_camera;
    std::unique_ptr<CaptureThread> m_captureThread;
    ImageView&                     m_imageView;
    QImage                         m_displayedImage;
    QTime                          m_framesTimer;
    int                            m_frameNumber;
    double                         m_frameDurationsMs[ NUM_FRAMES_TO_AVERAGE ];
    std::unique_ptr< AviWriter >   m_videoWriter;

    QTime                          m_startTime;
    QLineEdit*                     m_recordingTimer;
};

#endif
