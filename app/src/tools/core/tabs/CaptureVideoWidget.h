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

#ifndef CAPTUREVIDEOWIDGET_H
#define CAPTUREVIDEOWIDGET_H

#include <QtGui/QWidget>
#include <QtGui/QTableWidget>
#include <QtCore/QTimer>

#include "Tool.h"

#include <functional>
#include <vector>
#include <memory>

#include "CameraApi.h"
#include "VideoSource.h"
#include "AviWriter.h"

#if defined(__MINGW32__) || defined(__GNUC__)
#include <Callback.h>
#else
#include <functional>
#endif

namespace Ui
{
    class CaptureVideoWidget;
}

class CameraHardware;
class CameraDescription;
class ImageView;

struct VideoSourceAndCameraPosition
{
    std::auto_ptr<VideoSource> videoSource;
    KeyId                      cameraPositionId;

    VideoSourceAndCameraPosition( VideoSource* vs, const KeyId& camPosId ) :
        videoSource( vs ),
        cameraPositionId( camPosId )
    {}
};

typedef std::vector<std::unique_ptr<VideoSourceAndCameraPosition> > VideoSourcesCollection;


/** @brief A tool which allows the user to capture video from live sources,
 *  or alternatively, select pre-recorded video sequences.
 *
 *  @bug Can click on record button with no video sources to capture.
 *
 *  @bug [user-friendliness] Sometimes takes multiple updates to update frame
 *  rate on cameras.
 *  @todo Make a 'return-to-default' option for frame rate
 *  @todo display actual frame-rate somewhere
 *
 *  @note all cameras use the same frame rate ( or as close as possible ).
 *
 *  @todo Improve this:
 *  @note If one (or more) camera doesn't show a moving image,
 *  lower the frame rate & click update.
 *  Check that the frame rate actually updates. May need to try a few values to
 *  make it work.
 *
 */
class CaptureVideoWidget : public Tool
{
    Q_OBJECT

#if defined(__MINGW32__) || defined(__GNUC__)
    typedef Callback_1< bool const, const QStringList& > GetVideoSourcesForCallback;
#else
    typedef std::function<bool (const QStringList&)> GetVideoSourcesForCallback;
#endif

public:
    explicit CaptureVideoWidget( CameraHardware& cameraHardware, QWidget* parent = 0 );
    ~CaptureVideoWidget();

    virtual const QString Name() const { return tr( "Video Source" ); }
    virtual bool CanClose() const;
    const QString CannotCloseReason() const;

private slots:
    void RecordButtonClicked( const bool shouldRecord );
    void CaptureLiveConnectDisconnectButtonClicked();
    void CaptureLoadResetButtonClicked();

    void FormatXVIDButtonClicked();
    void FormatMP4ButtonClicked();

private:
    void SetupUi();
    void ConnectSignals();
    void AddLiveVideo( const CameraDescription& chosenCamera, const KeyId& camPosId );

    const KeyId GetRoomIdToCapture() const;
    const QStringList GetCameraPositionIds(const KeyId& roomIdToCapture);
    const bool AddLiveSourcesForEachCameraPosition(const QStringList& cameraPositionIds);
    bool TryToAddLiveVideoFor(const KeyId& camPosId);

    void StartUpdatingImages();
    void StopUpdatingImages();

    void StartVideoSources();
    void StopVideoSources();

    void TryToGetOutputDirectoryAndStartRecording();
    void StartRecordingInDirectory( const QString& outputDirectoryName );
    void StopRecording();

    bool AnyViewIsRecording() const;

#if defined(__MINGW32__) || defined(__GNUC__)
    void SetUpVideoSources(GetVideoSourcesForCallback* getVideoSourcesFor);
#else
    void SetUpVideoSources(GetVideoSourcesForCallback getVideoSourcesFor);
#endif
    void AddVideoFileConfigKey(const KeyId& camPosId, const QString& videoFileName, const QString &timestampFileName);
    void AddTableRow(QTableWidgetItem* tableItem);
    void RemovePreviouslyChosenCameras( CameraApi::CameraList& connectedCameras );
    void RemoveAllVideoSources();

    void ShowNoRoomError();
    void ShowEmptyRoomError();
    void ShowNullCameraPosError();
    void ShowMissingCameraError(const QString& cameraPosDisplayName);

    virtual const QString GetSubSchemaDefaultFileName() const;

    static const WbSchema CreateSchema();

    Ui::CaptureVideoWidget*  m_ui;
    CameraHardware&              m_cameraHardware;
    double                       m_fps;
    VideoSourcesCollection       m_videoSources;

    AviWriter::codecType         m_codec;
    QString                      m_fname;
    QString                      m_tname;

    bool                         m_videoSourcesAdded;
    bool                         m_videoSourcesOpen;
};

#endif // CAPTUREVIDEOTOOLWIDGET_H
