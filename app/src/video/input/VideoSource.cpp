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

#include "VideoSource.h"

#include <memory>
#include <numeric>
#include <algorithm>
#include <iostream>

#include <opencv/highgui.h>

#include "CaptureThread.h"
#include "VideoSequence.h"
#include "ImageView.h"
#include "AviWriter.h"

#include <QtCore/QTime>
#include <QtCore/QThread>
#include <QtGui/QTableWidget>
#include <QTime>

#include "Debugging.h"

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4351)
#endif

const double VideoSource::FPS_7_5  = 7.5;
const double VideoSource::FPS_15   = 15.0;
const double VideoSource::FPS_30   = 30.0;
const double VideoSource::FPS_60   = 60.0;

/** @brief Create a VideoSource.
 *
 * @param camera    The camera to display images from.
 * @param imageView The ImageView to use to display the images.
 * @param recordingTimer The QLineEdit item to update with time
 */
VideoSource::VideoSource(const CameraDescription& camera, ImageView& imageView, QLineEdit* recordingTimer) :
    m_camera(camera),
    m_captureThread(),
    m_imageView(imageView),
    m_displayedImage(),
    m_framesTimer(),
    m_frameNumber(0),
    m_frameDurationsMs(),
    m_videoWriter(),
    m_startTime(),
    m_recordingTimer(recordingTimer)
{
    m_framesTimer.start();
    std::fill_n(m_frameDurationsMs, NUM_FRAMES_TO_AVERAGE, 0.0);
}

#ifdef _MSC_VER
#pragma warning (pop)
#endif

/** @brief Destroy the VideoSource after waiting for the CaptureThread to finish
 *
 */
VideoSource::~VideoSource()
{
    StopUpdatingImage();
}

/** @brief Get the size of the image returned by the camera.
 *
 * @return The camera image size.
 */
const QSize VideoSource::GetImageSize() const
{
    return m_imageView.GetImageSize();
}

/** @brief Start recording images to file each frame.
 *
 *  The VideoSource takes ownership of the AviWriter.
 *
 * @param videoWriter The AviWriter to record to.
 */
void VideoSource::RecordTo(std::unique_ptr< AviWriter >&& videoWriter)
{
    m_startTime.start();
    m_videoWriter.reset(videoWriter.release());
}

/** @brief Stop recording images to file.
 *
 */
void VideoSource::StopRecording()
{
    m_videoWriter.reset();
}

/** @brief Start updating the ImageView with camera images.
 *
 * This also allocates a camera device from the API.
 *
 * @param fps The frame rate to use for the camera.
 */
void VideoSource::StartUpdatingImage(double fps)
{
    StopUpdatingImage();

    CaptureThread* capture = new CaptureThread(m_camera.CreateVideoSequence(fps));

    QObject::connect(capture,
                      SIGNAL(GotImage(const QImage&, const timespec, const double)),
                      this,
                      SLOT(UpdateDisplayedImage(const QImage, const timespec, const double)),
                      Qt::AutoConnection);
    QObject::connect(capture,
                      SIGNAL(finished()),
                      this,
                      SLOT(ResetCapture()),
                      Qt::AutoConnection);

    m_captureThread.reset(capture);
}

/** @brief Clear and delete the CaptureThread
 *
 */
void VideoSource::ResetCapture()
{
    m_captureThread.reset();
}

/** @brief Stop updating images from the camera.
 *
 * And release API camera resources.
 */
void VideoSource::StopUpdatingImage()
{
    if (m_captureThread)
    {
        m_captureThread->StopCapturing();
    }

    ResetCapture();
}

/** @brief Update the ImageView with a camera image.
 *
 * Also record the image to AVI if we have started recording.
 * This is a Qt slot getting images from the Capture Thread.
 *
 * @param newImage The new image received from the camera.
 */
void VideoSource::UpdateDisplayedImage(const QImage newImage, const timespec stamp, const double fps)
{
    if (m_videoWriter.get())
    {
        m_videoWriter->addFrame(reinterpret_cast< const char* >(newImage.constBits()),
                                 newImage.width(),
                                 newImage.height(),
                                 stamp);
        if (m_recordingTimer)
        {
            UpdateRecordingTimer();
        }
    }

    m_displayedImage = newImage.rgbSwapped();
    SetImageAndUpdateFpsDisplay(fps);
    m_imageView.update();
}

bool VideoSource::IsRecording() const
{
    return (m_videoWriter.get() != 0);
}

/** @brief Actually set the new and update the caption with the frame rate.
 *
 *  The frame rate is very approximate & based on the time between calls to this function.
 */
void VideoSource::SetImageAndUpdateFpsDisplay(double devFps)
{
    if (!m_displayedImage.isNull())
    {
        m_imageView.SetImage(m_displayedImage);

        static const double MSEC_PER_SEC = 1000.0;
        const double frameDurationMs = (double)m_framesTimer.restart();

        m_frameDurationsMs[ m_frameNumber%NUM_FRAMES_TO_AVERAGE ] = frameDurationMs;
        m_frameNumber++;

        const double avgFrameDurationMs = std::accumulate(m_frameDurationsMs,
                                                           m_frameDurationsMs+NUM_FRAMES_TO_AVERAGE,
                                                           0.0) / NUM_FRAMES_TO_AVERAGE;

        const double avgFps = MSEC_PER_SEC / avgFrameDurationMs;

        m_imageView.SetCaption(QString("%1x%2@%3(%4)").arg(m_displayedImage.width())
                                                         .arg(m_displayedImage.height())
                                                         .arg(devFps, 5, 'f', 2)
                                                         .arg(avgFps, 5, 'f', 2));
    }
}

/** @brief Update the recording timer in GUI
 *
 */
void VideoSource::UpdateRecordingTimer()
{
    int delta_ms = m_startTime.elapsed();

    QTime time = QTime(0,0);
    m_recordingTimer->setText(time.addMSecs(delta_ms).toString("hh:mm:ss:zzz"));
}

/** @brief Is the VideoSource getting images from the specified camera?
 *
 * @param cameraDescription The camera to check against.
 * @return @a true if we're getting images from the camera described by @a cameraDescription,
 *         @a false otherwise.
 */
bool VideoSource::IsFrom(const CameraDescription& cameraDescription) const
{
    if (m_camera.UniqueId() == cameraDescription.UniqueId())
    {
        return true;
    }

    return false;
}
