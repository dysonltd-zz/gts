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

#include "CaptureVideoToolWidget.h"

#include "ui_CaptureVideoToolWidget.h"

#include "CameraDescription.h"
#include "CameraHardware.h"
#include "VideoSequence.h"

#include "CameraPositionSchema.h"
#include "RoomLayoutSchema.h"
#include "VideoCaptureSchema.h"
#include "RunSchema.h"

#include "Message.h"

#include "RoomsCollection.h"
#include "CamerasCollection.h"
#include "CameraPositionsCollection.h"
#include "CameraTools.h"

#include "FileUtilities.h"
#include "FileDialogs.h"

#include "WbConfigTools.h"
#include "WbDefaultKeys.h"

#include <QtGui/QTableWidget>
#include <QtCore/QFileInfo>

#include <opencv/highgui.h>

#include <memory>
#include <sstream>
#include <iostream>

static const double rate[] = {VideoSource::FPS_3_75,
                              VideoSource::FPS_7_5,
                              VideoSource::FPS_15,
                              VideoSource::FPS_30,
                              VideoSource::FPS_40,
                              VideoSource::FPS_50,
                              VideoSource::FPS_60};

static const int SOURCE_COLUMN = 0;
static const int RATE_COLUMN = 1;

static const int NUM_COLS = 2;

/** @bug Should use QStyle::standardIcon to get the media player icons, can't do it in
 * Qt Designer though.
 */

/** @todo Need to auto-update frame-rate if user presses enter in the frame-rate spin box
 *
 * @param cameraHardware
 * @param parent
 */
CaptureVideoToolWidget::CaptureVideoToolWidget( CameraHardware& cameraHardware,
                                                QWidget* parent ) :
    Tool                ( parent, CreateSchema() ),
    m_ui                ( new Ui::CaptureVideoToolWidget ),
    m_cameraHardware    ( cameraHardware ),
    m_fps               ( -1.0 ),
    m_codec             ( AviWriter::CODEC_XVID ),
    m_fname             ( QString("video%1.avi") ),
    m_videoSourcesAdded ( false ),
    m_videoSourcesOpen  ( false )
{
    SetupUi();

    ConnectSignals();
}

CaptureVideoToolWidget::~CaptureVideoToolWidget()
{
    delete m_ui;
}

void CaptureVideoToolWidget::SetupUi()
{
    m_ui->setupUi( this );

    m_ui->m_videoTable->setColumnCount( NUM_COLS );
    m_ui->m_videoTable->setSortingEnabled( false );
}

void CaptureVideoToolWidget::ConnectSignals()
{
    QObject::connect( m_ui->m_captureLiveConnectDisconnectBtn,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( CaptureLiveConnectDisconnectButtonClicked() ) );
    QObject::connect( m_ui->m_captureLoadResetBtn,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( CaptureLoadResetButtonClicked() ) );
    QObject::connect( m_ui->m_recordBtn,
                      SIGNAL( clicked(bool) ),
                      this,
                      SLOT( RecordButtonClicked(const bool) ) );
    QObject::connect( m_ui->m_formatXVIDRadioBtn,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( FormatXVIDButtonClicked() ) );
    QObject::connect( m_ui->m_formatMP4RadioBtn,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( FormatMP4ButtonClicked() ) );
}

const QString CaptureVideoToolWidget::GetSubSchemaDefaultFileName() const
{
    return "capturedVideos.xml";
}

void CaptureVideoToolWidget::CaptureLoadResetButtonClicked()
{
    if( !m_videoSourcesAdded )
    {
#if defined(__MINGW32__) || defined(__GNUC__)
        SetUpVideoSources(MakeCallback( this,
                                            &CaptureVideoToolWidget::AddLiveSourcesForEachCameraPosition ) );
#else
        SetUpVideoSources([this](const QStringList& camPosIds) -> bool
                      {
                          return AddLiveSourcesForEachCameraPosition(camPosIds);
                      });
#endif
    }
    else
    {
        // reset
        StopUpdatingImages();
        RemoveAllVideoSources();

        m_ui->m_captureLoadResetBtn->setText("&Load");
        m_ui->m_captureLiveConnectDisconnectBtn->setEnabled(false);

        m_videoSourcesAdded = false;
    }
}

void CaptureVideoToolWidget::FormatXVIDButtonClicked()
{
    m_codec = AviWriter::CODEC_XVID;
    m_fname = QString("video%1.avi");
}

void CaptureVideoToolWidget::FormatMP4ButtonClicked()
{
    m_codec = AviWriter::CODEC_FMP4;
    m_fname = QString("video%1.mp4");
}

void CaptureVideoToolWidget::RecordButtonClicked( const bool shouldRecord )
{
    if ( shouldRecord )
    {
        TryToGetOutputDirectoryAndStartRecording();
        m_ui->m_captureLiveConnectDisconnectBtn->setEnabled(false);
    }
    else
    {
        StopRecording();
        m_ui->m_captureLiveConnectDisconnectBtn->setEnabled(true);
    }
}

void CaptureVideoToolWidget::StopRecording()
{
    for (auto videoSource = m_videoSources.begin();
              videoSource != m_videoSources.end();
              ++videoSource)
    {
        (*videoSource)->videoSource->StopRecording();
    }
}

bool CaptureVideoToolWidget::AnyViewIsRecording() const
{
    for (auto videoSource = m_videoSources.begin();
              videoSource != m_videoSources.end();
              ++videoSource)
    {
        if ((*videoSource)->videoSource->IsRecording())
        {
            return true;
        }
    }
    return false;
}

void CaptureVideoToolWidget::AddVideoFileConfigKey(const KeyId& camPosId,
                                                   const QString& videoFileName,
                                                   const QString& timestampFileName)
{
    WbConfig config(GetCurrentConfig());

    const KeyId keyId = config.AddKeyValue(VideoCaptureSchema::cameraPositionIdKey, KeyValue::from(camPosId));

    WbConfigTools::SetFileName( config,
                                videoFileName,
                                VideoCaptureSchema::videoFileNameKey,
                                WbConfigTools::FileNameMode_RelativeInsideWorkbench,
                                keyId );

    WbConfigTools::SetFileName( config,
                                timestampFileName,
                                VideoCaptureSchema::timestampFileNameKey,
                                WbConfigTools::FileNameMode_RelativeInsideWorkbench,
                                keyId );
}

void CaptureVideoToolWidget::StartRecordingInDirectory( const QString& outputDirectoryName )
{
    QDir outputDirectory( outputDirectoryName );
    bool directoryExists = outputDirectory.exists();
    if ( !directoryExists )
    {
        directoryExists = QDir().mkpath( outputDirectory.absolutePath() );
    }

    if ( !directoryExists )
    {
        Message::Show( this,
                       tr( "Capture Video" ),
                       tr( "Error - Failed to create directory: %1!" )
                         .arg( outputDirectory.absolutePath() ),
                       Message::Severity_Critical );
        return;
    }

    int row = 0;

    for (auto videoSource = m_videoSources.begin(); videoSource != m_videoSources.end(); ++videoSource)
    {
        const QSize imageSize((*videoSource)->videoSource->GetImageSize());

        /// @todo This is slightly nasty - whilst the Xml file will always contain
        /// a correct mapping between videos and timestamps, they could have different
        /// enumerations if videos have ben manually copied from elsewhere : e.g. the
        /// timestamps for video1.avi could be in the file timestamps0.txt if there was
        /// a stray file named video0.avi already in the capture directory. Are we ok
        /// with this?
        const QString videoFileName(FileUtilities::GetUniqueFileName(
                                     outputDirectory.absoluteFilePath(m_fname)));

        const QString timestampFileName(FileUtilities::GetUniqueFileName(
                                            outputDirectory.absoluteFilePath("timestamps%1.txt")));

        // Identify frame rate...
        QComboBox* combo = (QComboBox*)m_ui->m_videoTable->cellWidget(row, RATE_COLUMN);
        double frameRate = rate[combo->currentIndex()];

        AddVideoFileConfigKey((*videoSource)->cameraPositionId, videoFileName, timestampFileName);

        const QFileInfo fileInfo( videoFileName );
        if ( !fileInfo.exists() || fileInfo.isWritable() )
        {
            (*videoSource)->videoSource->RecordTo(
                        std::unique_ptr< AviWriter >(new AviWriter( m_codec,
                                                                    imageSize.width(),
                                                                    imageSize.height(),
                                                                    videoFileName.toAscii(),
                                                                    timestampFileName.toAscii(),
                                                                    frameRate )));
        }
        else
        {
            StopRecording();
            Message::Show( this,
                           tr( "Capture Video" ),
                           tr( "Error - Cannot write video: %1!" )
                             .arg(videoFileName),
                           Message::Severity_Critical );
            return;
        }

        row++;
    }
}

void CaptureVideoToolWidget::TryToGetOutputDirectoryAndStartRecording()
{
    StartRecordingInDirectory( GetCurrentConfig().GetAbsoluteFileNameFor("capturedVideos/") );
}

namespace
{
    class MatchesAny
    {
    public:
        MatchesAny( const VideoSourcesCollection& videoSources ) :
            m_videoSources( videoSources ) {}

        bool operator() ( const CameraDescription& description ) const
        {
            for (auto videoSource = m_videoSources.begin();
                      videoSource != m_videoSources.end();
                      ++videoSource)
            {
                if ((*videoSource)->videoSource->IsFrom( description ))
                {
                    return true;
                }
            }
            return false;
        }

    private:
        const VideoSourcesCollection& m_videoSources;
    };
}

void CaptureVideoToolWidget::RemovePreviouslyChosenCameras( CameraApi::CameraList& connectedCameras )
{
    const CameraApi::CameraList::iterator newEnd =
        std::remove_if( connectedCameras.begin(),
                        connectedCameras.end(),
                        MatchesAny( m_videoSources ) );

    connectedCameras.erase( newEnd, connectedCameras.end() );
}

void CaptureVideoToolWidget::RemoveAllVideoSources()
{
    StopUpdatingImages();

    m_videoSources.clear();

    m_ui->m_imageGrid->Clear();

    m_ui->m_videoTable->clear();
    m_ui->m_videoTable->setRowCount(0);

    QStringList headerlabels;
    headerlabels << "Source" << "Rate";
    m_ui->m_videoTable->setHorizontalHeaderLabels( headerlabels );

    m_ui->m_recordBtn->setEnabled(false);

#if 0
    WbConfig config(GetCurrentConfig());
    config.RemoveOldKeys(VideoCaptureSchema::cameraPositionIdKey);
    config.RemoveOldKeys(VideoCaptureSchema::videoFileNameKey);
    config.RemoveOldKeys(VideoCaptureSchema::timestampFileNameKey);
#endif
}

void CaptureVideoToolWidget::ShowNoRoomError()
{
    Message::Show( this,
                   tr( "Capture Video" ),
                   tr( "Error - There is no room selected!" ),
                   Message::Severity_Critical );
}

void CaptureVideoToolWidget::ShowEmptyRoomError()
{
    Message::Show( this,
                   tr( "Capture Video" ),
                   tr( "Error - The selected room is empty!" ),
                   Message::Severity_Critical );
}

void CaptureVideoToolWidget::ShowNullCameraPosError()
{
    Message::Show( this,
                   tr( "Capture Video" ),
                   tr( "Error - One of the camera positions is invalid!" ),
                   Message::Severity_Critical );
}

void CaptureVideoToolWidget::ShowMissingCameraError(const QString& cameraPosDisplayName)
{
    Message::Show( this,
                   tr( "Capture Video" ),
                   tr( "Error - Camera position %1 is missing camera!" )
                      .arg(cameraPosDisplayName),
                   Message::Severity_Critical );
}

const KeyId CaptureVideoToolWidget::GetRoomIdToCapture() const
{
    const WbConfig capturedVideosConfig( GetCurrentConfig() );
    const WbConfig runConfig( capturedVideosConfig.GetParent() );
    const KeyId roomIdToCapture( runConfig.GetKeyValue( RunSchema::roomIdKey ).ToKeyId() );

    return roomIdToCapture;
}

const QStringList CaptureVideoToolWidget::GetCameraPositionIds(const KeyId& roomIdToCapture)
{
    Collection roomsCollection( RoomsCollection() );
    roomsCollection.SetConfig(GetCurrentConfig());

    const WbConfig roomConfig( roomsCollection.ElementById( roomIdToCapture ) );
    const WbConfig roomLayoutConfig( roomConfig.GetSubConfig( RoomLayoutSchema::schemaName ) );
    const QStringList cameraPositionIds(
        roomLayoutConfig.GetKeyValue(RoomLayoutSchema::cameraPositionIdsKey)
        .ToQStringList() );

    return cameraPositionIds;
}

bool CaptureVideoToolWidget::TryToAddLiveVideoFor(const KeyId& camPosId)
{
    const bool SUCCESS = true;
    const bool FAILURE = false;

    Collection cameraPositionsCollection(CameraPositionsCollection());
    cameraPositionsCollection.SetConfig(GetCurrentConfig());
    const WbConfig camPosConfig(cameraPositionsCollection.ElementById(camPosId));

    if (camPosConfig.IsNull())
    {
        ShowNullCameraPosError();
        return FAILURE;
    }

    const QString camPosDisplayName(WbConfigTools::DisplayNameOf(camPosConfig));

    const KeyId cameraId(camPosConfig.GetKeyValue(CameraPositionSchema::cameraIdKey).ToKeyId());
    if (cameraId.isEmpty())
    {
        ShowMissingCameraError(camPosDisplayName);
        return FAILURE;
    }

    Collection camerasCollection(CamerasCollection());
    camerasCollection.SetConfig(GetCurrentConfig());
    const WbConfig cameraConfig(camerasCollection.ElementById(cameraId));
    const CameraDescription camera(CameraTools::GetCameraForStreamingIfOk(m_cameraHardware, cameraConfig));

    if (!camera.IsValid())
    { /** @todo show error */
        return FAILURE;
    }

    AddLiveVideo(camera, camPosId);

    return SUCCESS;
}

const bool CaptureVideoToolWidget::AddLiveSourcesForEachCameraPosition(const QStringList& cameraPositionIds)
{
    bool successfulSoFar = true;

    Collection camerasCollection(CamerasCollection());
    camerasCollection.SetConfig(GetCurrentConfig());

    for (auto camPosId = cameraPositionIds.begin();
         successfulSoFar && (camPosId != cameraPositionIds.end());
         ++camPosId)
    {
        successfulSoFar = TryToAddLiveVideoFor(*camPosId);
    }

    return successfulSoFar;
}

#if defined(__MINGW32__) || defined(__GNUC__)
void CaptureVideoToolWidget::SetUpVideoSources(GetVideoSourcesForCallback* getVideoSourcesFor)
#else
void CaptureVideoToolWidget::SetUpVideoSources(GetVideoSourcesForCallback getVideoSourcesFor)
#endif
{
    RemoveAllVideoSources();

    const KeyId roomIdToCapture(GetRoomIdToCapture());
    if (roomIdToCapture.isEmpty()) { ShowNoRoomError(); return; }

    const QStringList cameraPositionIds(GetCameraPositionIds(roomIdToCapture));
    if (cameraPositionIds.size() == 0) { ShowEmptyRoomError(); return; }

#if defined(__MINGW32__) || defined(__GNUC__)
    const bool success = (*getVideoSourcesFor)(cameraPositionIds);
#else
    const bool success = getVideoSourcesFor(cameraPositionIds);
#endif
    if ( success )
    {
        m_ui->m_captureLoadResetBtn->setText("&Clear");

        m_videoSourcesAdded = true;
        m_ui->m_captureLiveConnectDisconnectBtn->setEnabled(true);
    }
}

void CaptureVideoToolWidget::CaptureLiveConnectDisconnectButtonClicked()
{
    if ( !m_videoSourcesOpen  )
    {
        StartVideoSources();

        if( CanClose() )
        {
            m_ui->m_captureLiveConnectDisconnectBtn->setText("&Disconnect");
        }
    }
    else
    {
        StopVideoSources();

        m_ui->m_captureLiveConnectDisconnectBtn->setText("&Connect");
        m_ui->m_time->setText( QString("00:00:00:000") );
    }
}

void CaptureVideoToolWidget::StartUpdatingImages()
{
    int row = 0;

    for (auto videoSource = m_videoSources.begin();
              videoSource != m_videoSources.end();
              ++videoSource)
    {
        QComboBox* combo = (QComboBox*)m_ui->m_videoTable->cellWidget(row, RATE_COLUMN);

        (*videoSource)->videoSource->StartUpdatingImage( rate[combo->currentIndex()]);

        row++;
    }
}

void CaptureVideoToolWidget::StopUpdatingImages()
{
    for (auto videoSource = m_videoSources.begin();
              videoSource != m_videoSources.end();
              ++videoSource)
    {
        (*videoSource)->videoSource->StopUpdatingImage();
    }
}

/*
 * @brief Start live feed in window
 */
void CaptureVideoToolWidget::StartVideoSources()
{
    StartUpdatingImages();

    m_ui->m_recordBtn->setEnabled(true);
    m_ui->m_captureLoadResetBtn->setEnabled(false);

    m_videoSourcesOpen = true;
}

/*
 * @brief Stop live feed in window but do not remove loaded cameras from list
 */
void CaptureVideoToolWidget::StopVideoSources()
{
    StopUpdatingImages();

    m_ui->m_recordBtn->setEnabled(false);
    m_ui->m_captureLoadResetBtn->setEnabled(true);

    m_videoSourcesOpen = false;
}

void CaptureVideoToolWidget::AddTableRow(QTableWidgetItem* tableItem)
{
    const int newAppendedRow = m_ui->m_videoTable->rowCount();

    m_ui->m_videoTable->insertRow( newAppendedRow );
    m_ui->m_videoTable->setItem(newAppendedRow, SOURCE_COLUMN, tableItem);

    QComboBox* rateSpinBox = new QComboBox();
    rateSpinBox->addItem( "3.75" );
    rateSpinBox->addItem( "7.5" );
    rateSpinBox->addItem( "15" );
    rateSpinBox->addItem( "30" );
    rateSpinBox->addItem( "40" );
    rateSpinBox->addItem( "50" );
    rateSpinBox->addItem( "60" );

    m_ui->m_videoTable->setCellWidget( newAppendedRow, RATE_COLUMN, rateSpinBox );
}

void CaptureVideoToolWidget::AddLiveVideo( const CameraDescription& chosenCamera, const KeyId& camPosId )
{
    ImageView* const addedImageView(m_ui->m_imageGrid->AddBlankImage(chosenCamera.GetImageSize()));

    if ( addedImageView )
    {
        VideoSource* newVideoSource = new VideoSource( chosenCamera, *addedImageView, m_ui->m_time );
        AddTableRow(chosenCamera.CreateTableItem());
        m_videoSources.push_back(std::unique_ptr<VideoSourceAndCameraPosition>(
                                     new VideoSourceAndCameraPosition(newVideoSource, camPosId)));
    }
}

bool CaptureVideoToolWidget::CanClose() const
{
    if ( AnyViewIsRecording() )
    {
        return false;
    }

    return Tool::CanClose();
}

const QString CaptureVideoToolWidget::CannotCloseReason() const
{
    return tr("The video is currently being recorded. Please stop recording before switching tabs.");
}

const WbSchema CaptureVideoToolWidget::CreateSchema()
{
    using namespace VideoCaptureSchema;

    WbSchema schema( CreateWorkbenchSubSchema( schemaName,
                                               tr( "Captured Videos" ) ) );

    schema.AddKeyGroup( capturedVideoGroup,
                        WbSchemaElement::Multiplicity::Many,
                        KeyNameList() << cameraPositionIdKey
                                      << videoFileNameKey
                                      << timestampFileNameKey );

    return schema;
}
