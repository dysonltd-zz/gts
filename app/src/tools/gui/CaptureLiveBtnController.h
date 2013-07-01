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

#ifndef CAPTURELIVEBTNCONTROLLER_H_
#define CAPTURELIVEBTNCONTROLLER_H_

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

class CaptureLiveBtnController : public QObject
{
#if defined(__MINGW32__) || defined(__GNUC__)
    typedef Callback_1< ImageView* const, const QSize& > CreateStreamViewCallback;
#else
    typedef std::function<ImageView* (const QSize&)> CreateStreamViewCallback;
#endif
    Q_OBJECT
public:
    CaptureLiveBtnController( QPushButton&    captureLiveBtn,
                              QPushButton&    captureCancelBtn,
                              Tool&           toolWidget,
                              CameraHardware& cameraHardware );

    const QString CaptureLiveBtnClicked( const WbConfig& cameraConfig,
                                         const QString& newImageFileNameFormat,
#if defined(__MINGW32__) || defined(__GNUC__)
										 CreateStreamViewCallback* createStreamView  );
#else
                                         CreateStreamViewCallback createStreamView);
#endif
    const void CaptureCancelBtnClicked();

    bool CurrentlyStreamingLiveSource() const;

private:
    void TryToStartStreamingLiveSource(
        const WbConfig& cameraConfig,
#if defined(__MINGW32__) || defined(__GNUC__)
		std::auto_ptr< CreateStreamViewCallback > createStreamView );
#else
        CreateStreamViewCallback createStreamView);
#endif

    const QString CaptureImageAndStopStreamingLiveSource(
                                const QString& newImageFileNameFormat );

    QPushButton& m_captureLiveBtn;
    QPushButton& m_captureCancelBtn;
    Tool&     m_toolWidget;
    bool m_capturingLive;
    CameraHardware& m_cameraHardware;
    std::auto_ptr< VideoSource > m_videoSource;
    ImageView* m_liveView;
};

#endif
