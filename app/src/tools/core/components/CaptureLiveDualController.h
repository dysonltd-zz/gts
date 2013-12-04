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

#ifndef CAPTURELIVEDUALCONTROLLER_H_
#define CAPTURELIVEDUALCONTROLLER_H_

#include <QtGui/QPushButton>

#include <memory>

#if defined(__MINGW32__) || defined(__GNUC__)
#include <Callback.h>
#else
#include <functional>
#endif

class WbConfig;
class Tool;
class ImageView;
class VideoSource;
class CameraHardware;

template< class ReturnType, class ArgType > class Callback_1;

class CaptureLiveDualController : public QObject
{
#if defined(__MINGW32__) || defined(__GNUC__)
    typedef Callback_1< ImageView* const, const QSize& > CreateStreamViewCallback;
#else
    typedef std::function<ImageView* (const QSize&)> CreateStreamViewCallback;
#endif
    Q_OBJECT
public:
    CaptureLiveDualController(QPushButton&    captureLiveBtn,
                               QPushButton&    captureCancelBtn,
                               Tool&           toolWidget,
                               CameraHardware& cameraHardware);

#if defined(__MINGW32__) || defined(__GNUC__)
    ~CaptureLiveDualController();
#endif

    bool CaptureLiveBtnClicked(const WbConfig& cameraConfig1,
	                                  const WbConfig& cameraConfig2,
                                      const QString& newImageFileNameFormat,
#if defined(__MINGW32__) || defined(__GNUC__)
									  CreateStreamViewCallback* createStreamView1,
									  CreateStreamViewCallback* createStreamView2);
#else
                                      CreateStreamViewCallback createStreamView1,
                                      CreateStreamViewCallback createStreamView2);
#endif
    const void CaptureCancelBtnClicked();

    bool CurrentlyStreamingLiveSource() const;

	const QString CapturedFileName1() const {return m_capturedFileName1;};
	const QString CapturedFileName2() const {return m_capturedFileName2;};

private:
    void TryToStartStreamingLiveSource(
        const WbConfig& cameraConfig1,
        const WbConfig& cameraConfig2,
#if defined(__MINGW32__) || defined(__GNUC__)
		std::unique_ptr< CreateStreamViewCallback > createStreamView1,
		std::unique_ptr< CreateStreamViewCallback > createStreamView2);
#else
        CreateStreamViewCallback createStreamView1,
        CreateStreamViewCallback createStreamView2);
#endif

    bool CaptureImageAndStopStreamingLiveSource(
                                const QString& newImageFileNameFormat);

    const QString CaptureImage(const QImage capturedImage,
                                const QString& newImageFileNameFormat);

    QPushButton& m_captureLiveBtn;
    QPushButton& m_captureCancelBtn;
    Tool& m_toolWidget;
    bool m_capturingLive;
    CameraHardware& m_cameraHardware;
    std::unique_ptr< VideoSource > m_videoSource1;
    std::unique_ptr< VideoSource > m_videoSource2;
    ImageView* m_liveView1;
    ImageView* m_liveView2;

	QString m_capturedFileName1;
	QString m_capturedFileName2;
};

#endif
