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

#include "TrackRobotToolWidget.h"

#include "ui_TrackRobotToolWidget.h"

#include "UnknownLengthProgressDlg.h"

#include "FileDialogs.h"
#include "FileUtilities.h"

#include "TrackRobotSchema.h"
#include "CameraPositionSchema.h"
#include "ExtrinsicCalibrationSchema.h"
#include "RoomLayoutSchema.h"
#include "FloorPlanSchema.h"
#include "TrackRobotSchema.h"
#include "RobotMetricsSchema.h"
#include "VideoCaptureSchema.h"
#include "RunSchema.h"
#include "TargetSchema.h"

#include "Message.h"

#include "Logging.h"

#include "RoomsCollection.h"
#include "CamerasCollection.h"
#include "CameraPositionsCollection.h"
#include "RobotsCollection.h"
#include "TargetsCollection.h"

#include <QtCore/QDir>
#include <QtGui/QMessageBox>
#include <QApplication>
#include <QShortcut>
#include <QtGlobal>

#include <sstream>

static const int BI_LEVEL_DEFAULT = 128;
static const double NCC_DEFAULT = 0.3;
static const double RESOLUTION_DEFAULT = 15.0;

TrackRobotToolWidget::TrackRobotToolWidget( QWidget* parent ) :
    Tool       ( parent, CreateSchema() ),
    m_ui       ( new Ui::TrackRobotToolWidget ),
    m_playing  ( false ),
    m_tracking ( false ),
    m_loaded   ( false ),
    m_fpsSet   ( false ),
    m_fps      ( 1 ),
    m_optimumRate ( 100 ),
    m_scanFwdIndex ( 0 ),
    m_scanBackIndex ( 0 )
{
    SetupUi();
    ConnectSignals();

    CreateMappers();

    RegisterCollectionCombo(m_ui->m_robotCombo, RobotsCollection());

    m_scanFwdIconRatePair.push_back(std::make_pair(":/scan.png", 1));
    m_scanFwdIconRatePair.push_back(std::make_pair(":/scan2x.png", 2));
    m_scanFwdIconRatePair.push_back(std::make_pair(":/scan4x.png", 4));
    m_scanFwdIconRatePair.push_back(std::make_pair(":/scan10x.png", 10));
    m_scanFwdIconRatePair.push_back(std::make_pair(":/scan20x.png", 20));

    m_scanBackIconRatePair.push_back(std::make_pair(":/revscan.png", 1));
    m_scanBackIconRatePair.push_back(std::make_pair(":/revscan2x.png", 2));
    m_scanBackIconRatePair.push_back(std::make_pair(":/revscan4x.png", 4));
    m_scanBackIconRatePair.push_back(std::make_pair(":/revscan10x.png", 10));
    m_scanBackIconRatePair.push_back(std::make_pair(":/revscan20x.png", 20));

}

void TrackRobotToolWidget::SetupUi()
{
    m_ui->setupUi( this );
    m_ui->m_videoTable->setColumnCount(2);
    m_ui->m_videoTable->setSortingEnabled( false );
}

void TrackRobotToolWidget::FillOutCameraCombo( QComboBox& comboBox )
{
    WbConfig config = GetCurrentConfig();

    Collection camPosCollection( CameraPositionsCollection() );
    Collection rooms ( RoomsCollection() );

    camPosCollection.SetConfig( config );
    rooms.SetConfig( config );

    comboBox.clear();

    // Get the run configuration...
    const WbConfig& runConfig = config.GetParent();

    // Get the room configuration (for this run)...
    const KeyId roomId = runConfig.GetKeyValue( RunSchema::roomIdKey ).ToKeyId();
    const WbConfig roomConfig = rooms.ElementById( roomId );

    // Get the room configuration...
    const WbConfig roomLayoutConfig( roomConfig.GetSubConfig( RoomLayoutSchema::schemaName ) );
    const QStringList camPosIds( roomLayoutConfig
            .GetKeyValue(RoomLayoutSchema::cameraPositionIdsKey)
             .ToQStringList() );

    for (auto camPosId = camPosIds.begin(); camPosId != camPosIds.end(); ++camPosId)
    {
        comboBox.addItem( WbConfigTools::DisplayNameOf(camPosCollection.ElementById(*camPosId)), QVariant( *camPosId ) );
    }
}

void TrackRobotToolWidget::ResetUi()
{
    m_ui->m_playBtn->setEnabled(false);
    m_ui->m_stepBtn->setEnabled(false);
    m_ui->m_stepBackBtn->setEnabled(false);
    m_ui->m_scanBackBtn->setEnabled(false);
    m_ui->m_scanFwdBtn->setEnabled(false);
    m_ui->m_stopBtn->setEnabled(false);

    SetButtonIcon(m_ui->m_scanBackBtn, QString::fromUtf8(m_scanBackIconRatePair[0].first.c_str()));
    SetButtonIcon(m_ui->m_scanFwdBtn, QString::fromUtf8(m_scanFwdIconRatePair[0].first.c_str()));
    m_scanBackIndex = 0;
    m_scanFwdIndex = 0;

    // enable global paramaters
    m_ui->m_nccThresholdSpinBox->setEnabled(true);
    m_ui->m_resolutionSpinBox->setEnabled(true);
    m_ui->m_trackerThresholdSpinBox->setEnabled(true);

    // enable camera specific parameters
    m_ui->m_camNccThresholdSpinBox->setEnabled(true);
    m_ui->m_camResolutionSpinBox->setEnabled(true);
    m_ui->m_camTrackerThresholdSpinBox->setEnabled(true);
}

void TrackRobotToolWidget::ConnectSignals()
{
    QObject::connect( m_ui->m_playBtn,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( PlayPauseButtonClicked() ) );
    QObject::connect( m_ui->m_stepBtn,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( StepButtonClicked() ) );
    QObject::connect( m_ui->m_stepBackBtn,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( StepBackButtonClicked() ) );
    QObject::connect( m_ui->m_scanBackBtn,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( ScanBackButtonClicked() ) );
    QObject::connect( m_ui->m_scanFwdBtn,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( ScanForwardButtonClicked() ) );
    QObject::connect( m_ui->m_stopBtn,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( StopButtonClicked() ) );

    QObject::connect( m_ui->m_loadBtn,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( TrackLoadButtonClicked() ) );
    QObject::connect( m_ui->m_saveBtn,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( TrackSaveButtonClicked() ) );
    QObject::connect( m_ui->m_resetBtn,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( TrackResetButtonClicked() ) );

    QObject::connect( m_ui->m_positionCombo,
                      SIGNAL( currentIndexChanged (int) ),
                      this,
                      SLOT( CameraComboChanged() ) );

    QObject::connect( m_ui->m_useGlobal,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( UseGlobalBtnClicked() ) );

    QObject::connect( m_ui->btnSave,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( SaveBtnClicked() ) );

#if 0
    // Keyboard shorcuts
    QShortcut *playPauseTrackKey = new QShortcut(Qt::Key_Space, this);
    QShortcut *stepBackKey = new QShortcut(Qt::Key_Left, this);
    QShortcut *stepForwardKey = new QShortcut(Qt::Key_Right, this);
    QShortcut *scanBackKey = new QShortcut(QKeySequence(Qt::CTRL+Qt::Key_Left), this);
    QShortcut *stopKey = new QShortcut(Qt::Key_Return, this);

    QObject::connect( playPauseTrackKey,
             SIGNAL( activated() ),
             this,
             SLOT( PlayPauseTrackButtonClicked() ) );
    QObject::connect( stepBackKey,
             SIGNAL( activated() ),
             this,
             SLOT( StepBackButtonClicked() ) );
    QObject::connect( stepForwardKey,
             SIGNAL( activated() ),
             this,
             SLOT( StepButtonClicked() ) );
    QObject::connect( scanBackKey,
             SIGNAL( activated() ),
             this,
             SLOT( ScanBackButtonClicked() ) );
    QObject::connect( stopKey,
             SIGNAL( activated() ),
             this,
             SLOT( StopButtonClicked() ) );
#endif
}

void TrackRobotToolWidget::CreateMappers()
{
    AddMapper( TrackRobotSchema::robotIdKey, m_ui->m_robotCombo );

    using namespace TrackRobotSchema::GlobalTrackingParams;

    AddMapper(biLevelThreshold, m_ui->m_trackerThresholdSpinBox);
    AddMapper(nccThreshold,     m_ui->m_nccThresholdSpinBox);
    AddMapper(resolution,       m_ui->m_resolutionSpinBox);
}

const QString TrackRobotToolWidget::GetCameraId() const
{
    QComboBox* const cameraCombo = m_ui->m_positionCombo;
    const int newTargetIndex = cameraCombo->currentIndex();
    const QString cameraId( cameraCombo->itemData( newTargetIndex ).toString() );
    return cameraId;
}

void TrackRobotToolWidget::PopulateCameraParams()
{
}

TrackRobotToolWidget::~TrackRobotToolWidget()
{
    delete m_ui;
}

const QString TrackRobotToolWidget::GetSubSchemaDefaultFileName() const
{
    return "trackerOutput.xml";
}

void TrackRobotToolWidget::ReloadCurrentConfigToolSpecific()
{
    const WbConfig& config = GetCurrentConfig();

#if 0
    if ( m_ui->m_robotCombo->count() == 1 )
    {
        m_ui->m_robotCombo->setCurrentIndex( 0 );
    }
#endif

    FillOutCameraCombo( *m_ui->m_positionCombo );

    // Get the selected room name...
    const WbConfig& runConfig = config.GetParent();
    const WbConfig& videoConfig = runConfig.GetSubConfig( VideoCaptureSchema::schemaName );

    // Get id keys for all the camera positions in the selected room:
    KeyId roomId = GetRoomIdToCapture();
    const WbKeyValues::ValueIdPairList positions = GetCameraPositionPairList( roomId );
    const WbKeyValues::ValueIdPairList capturedVideos = videoConfig.GetKeyValues( VideoCaptureSchema::cameraPositionIdKey );

    // Create a map from positions to lists of video file names:
    m_ui->m_videoTable->clear();
    m_ui->m_videoTable->setRowCount(0);
    m_mapPositionsToFiles.clear();

    for (auto p = positions.begin(); p != positions.end(); ++p)
    {
        WbKeyValues::ValueIdPair const &key = *p;

        // For each camera position in the room...
        const QStringList& positionStrings = key.value.ToQStringList();
        for (auto ps = positionStrings.begin(); ps != positionStrings.end(); ++ps)
        {
            QString const &positionString = *ps;

            // For every captured video...
            for (auto cv = capturedVideos.begin(); cv != capturedVideos.end(); ++cv)
            {
                WbKeyValues::ValueIdPair const &v = *cv;

                // check if it was captured at the current camera position:
                if (  v.value.ToQString() == positionString )
                {
                    // If it was add it to the map of position -> videoFiles
                    QString videoFilename = videoConfig.GetKeyValue( VideoCaptureSchema::videoFileNameKey, v.id ).ToQString();
                    QString timestampFilename = videoConfig.GetKeyValue( VideoCaptureSchema::timestampFileNameKey, v.id ).ToQString();

                    m_mapPositionsToFiles[ positionString ].first << videoFilename;
                    m_mapPositionsToFiles[ positionString ].second = timestampFilename;
                }
            }
        }
    }

    Collection camPosCollection(CameraPositionsCollection());
    camPosCollection.SetConfig(config);

    // Add to mappings to the table view so that user can
    // select video file for each position from combo box.
    foreach( QString pos, m_mapPositionsToFiles.keys() )
    {
        const QStringList& videos = m_mapPositionsToFiles.value( pos ).first;

        const QString position = WbConfigTools::DisplayNameOf(camPosCollection.ElementById(pos));

        AddTableRow( position, videos );
    }

    if ( !m_loaded )
    {
        ResetUi();
    }
}

void TrackRobotToolWidget::CameraComboChanged()
{
    const WbConfig& config = GetCurrentConfig();

    const KeyId cameraId = KeyId(GetCameraId());

    const WbKeyValues::ValueIdPairList cameraMappingIds =
        config.GetKeyValues( TrackRobotSchema::PerCameraTrackingParams::positionIdKey );

    bool useGlobalParams = true;
    int biLevelThreshold = config.GetKeyValue(TrackRobotSchema::GlobalTrackingParams::biLevelThreshold).ToInt();
    double nccThreshold = config.GetKeyValue(TrackRobotSchema::GlobalTrackingParams::nccThreshold).ToDouble();
    int resolution = config.GetKeyValue(TrackRobotSchema::GlobalTrackingParams::resolution).ToInt();

    for (WbKeyValues::ValueIdPairList::const_iterator it = cameraMappingIds.begin(); it != cameraMappingIds.end(); ++it)
    {
        const KeyId keyId( config.GetKeyValue( TrackRobotSchema::PerCameraTrackingParams::positionIdKey, it->id ).ToKeyId() );

        if (keyId == cameraId)
        {
            useGlobalParams = config.GetKeyValue(TrackRobotSchema::PerCameraTrackingParams::useGlobalParams, it->id).ToBool();

            if (!useGlobalParams)
            {
                biLevelThreshold = config.GetKeyValue(TrackRobotSchema::PerCameraTrackingParams::biLevelThreshold, it->id).ToInt();
                nccThreshold = config.GetKeyValue(TrackRobotSchema::PerCameraTrackingParams::nccThreshold, it->id).ToDouble();
                resolution = config.GetKeyValue(TrackRobotSchema::PerCameraTrackingParams::resolution, it->id).ToInt();
            }

            break;
        }
    }

    m_ui->m_useGlobal->setChecked(useGlobalParams);

    m_ui->m_camTrackerThresholdSpinBox->setValue(biLevelThreshold);
    m_ui->m_camNccThresholdSpinBox->setValue(nccThreshold);
    m_ui->m_camResolutionSpinBox->setValue(resolution);

    m_ui->m_camTrackerThresholdSpinBox->setEnabled(!useGlobalParams);
    m_ui->m_camNccThresholdSpinBox->setEnabled(!useGlobalParams);
    m_ui->m_camResolutionSpinBox->setEnabled(!useGlobalParams);
}

void TrackRobotToolWidget::UseGlobalBtnClicked()
{
    if ( m_ui->m_useGlobal->isChecked() )
	{
	    WbConfig config = GetCurrentConfig();

	    m_ui->m_camTrackerThresholdSpinBox->setValue
	        (config.GetKeyValue(TrackRobotSchema::GlobalTrackingParams::biLevelThreshold).ToInt());
	    m_ui->m_camNccThresholdSpinBox->setValue
	        (config.GetKeyValue(TrackRobotSchema::GlobalTrackingParams::nccThreshold).ToDouble());
	    m_ui->m_camResolutionSpinBox->setValue
	        (config.GetKeyValue(TrackRobotSchema::GlobalTrackingParams::resolution).ToInt());

	    m_ui->m_camTrackerThresholdSpinBox->setEnabled(false);
	    m_ui->m_camNccThresholdSpinBox->setEnabled(false);
	    m_ui->m_camResolutionSpinBox->setEnabled(false);
	}
	else
	{
	    m_ui->m_camTrackerThresholdSpinBox->setValue(BI_LEVEL_DEFAULT);
	    m_ui->m_camNccThresholdSpinBox->setValue(NCC_DEFAULT);
	    m_ui->m_camResolutionSpinBox->setValue(RESOLUTION_DEFAULT);

        if (!m_loaded)
        {
            m_ui->m_camTrackerThresholdSpinBox->setEnabled(true);
            m_ui->m_camNccThresholdSpinBox->setEnabled(true);
            m_ui->m_camResolutionSpinBox->setEnabled(true);
        }

	}
}

void TrackRobotToolWidget::SaveBtnClicked()
{
    WbConfig config = GetCurrentConfig();

    const KeyId cameraId = KeyId(GetCameraId());

    using namespace TrackRobotSchema::PerCameraTrackingParams;

    Collection camPosCollection( CameraPositionsCollection() );
    Collection rooms ( RoomsCollection() );

    camPosCollection.SetConfig( config );
    rooms.SetConfig( config );

    std::vector< KeyId > idsToKeep;

    // Get the run configuration...
    const WbConfig& runConfig = config.GetParent();

    // Get the room configuration (for this run)...
    const KeyId roomId = runConfig.GetKeyValue( RunSchema::roomIdKey ).ToKeyId();
    const WbConfig roomConfig = rooms.ElementById( roomId );

    // Get the room configuration...
    const WbConfig roomLayoutConfig( roomConfig.GetSubConfig( RoomLayoutSchema::schemaName ) );
    const QStringList camPosIds( roomLayoutConfig
            .GetKeyValue(RoomLayoutSchema::cameraPositionIdsKey)
             .ToQStringList() );

    for (auto camPosId = camPosIds.begin(); camPosId != camPosIds.end(); ++camPosId)
    {
        if ( KeyId(*camPosId) != cameraId )
        {

            const WbKeyValues::ValueIdPairList cameraMappingIds = config.GetKeyValues( positionIdKey );

            for (WbKeyValues::ValueIdPairList::const_iterator it = cameraMappingIds.begin(); it != cameraMappingIds.end(); ++it)
            {
                const KeyId positionId( config.GetKeyValue( positionIdKey, it->id ).ToKeyId() );

                if (positionId == KeyId(*camPosId))
                {
                    idsToKeep.push_back( it->id );
                    break;
                }
            }
        }
    }

    config.RemoveOldKeys( positionIdKey, idsToKeep );
    config.RemoveOldKeys( useGlobalParams, idsToKeep );
    config.RemoveOldKeys( biLevelThreshold, idsToKeep );
    config.RemoveOldKeys( nccThreshold, idsToKeep );
    config.RemoveOldKeys( resolution, idsToKeep );

    const KeyId cameraKeyId = config.AddKeyValue( positionIdKey,
                                                  KeyValue::from( cameraId ) );

    config.SetKeyValue( useGlobalParams,
                        KeyValue::from( m_ui->m_useGlobal->isChecked() ),
                        cameraKeyId );
    config.SetKeyValue( biLevelThreshold,
                        KeyValue::from( QString().setNum( m_ui->m_camTrackerThresholdSpinBox->value() ) ),
                        cameraKeyId );
    config.SetKeyValue( nccThreshold,
                        KeyValue::from( QString().setNum( m_ui->m_camNccThresholdSpinBox->value() ) ),
                        cameraKeyId );
    config.SetKeyValue( resolution,
                        KeyValue::from( QString().setNum( m_ui->m_camResolutionSpinBox->value() ) ),
                        cameraKeyId );
}

void TrackRobotToolWidget::AddVideoFileConfigKey( const QString& videoFileName,
                                                  const KeyId& camPosId)
{
    WbConfig config(GetCurrentConfig());
    const KeyId keyId = config.AddKeyValue(TrackRobotSchema::videoFileNameKey, KeyValue::from(videoFileName));
    config.SetKeyValue(TrackRobotSchema::cameraPositionIdKey, KeyValue::from(camPosId), keyId);
}

const bool TrackRobotToolWidget::CreateVideoDirectory( const QString& videoDirectoryName )
{
    QDir outputDirectory( videoDirectoryName );
    bool directoryExists = outputDirectory.exists();

    if ( !directoryExists )
    {
        directoryExists = QDir().mkpath( outputDirectory.absolutePath() );
    }

    if ( !directoryExists )
    {
        Message::Show( this,
                       tr( "Track Robot" ),
                       tr( "Error - Failed to create directory: %1!" )
                         .arg( outputDirectory.absolutePath() ),
                       Message::Severity_Critical );
        return false;
    }

    return true;
}

void TrackRobotToolWidget::ShowNoRoomError()
{
    Message::Show( this,
                   tr( "Track Robot" ),
                   tr( "Error - There is no room selected!" ),
                   Message::Severity_Critical );
}

void TrackRobotToolWidget::ShowEmptyRoomError()
{
    Message::Show( this,
                   tr( "Track Robot" ),
                   tr( "Error - The selected room is empty!" ),
                   Message::Severity_Critical );
}

void TrackRobotToolWidget::ShowNullCameraPosError()
{
    Message::Show( this,
                   tr( "Track Robot" ),
                   tr( "Error - One of the camera positions is invalid!" ),
                   Message::Severity_Critical );
}

void TrackRobotToolWidget::ShowMissingCameraError(const QString& cameraPosDisplayName)
{
    Message::Show( this,
                   tr( "Track Robot" ),
                   tr( "Camera position %1 is missing camera!" )
                     .arg(cameraPosDisplayName),
                   Message::Severity_Critical );
}

const KeyId TrackRobotToolWidget::GetRoomIdToCapture() const
{
    const WbConfig capturedVideosConfig( GetCurrentConfig() );
    const WbConfig runConfig( capturedVideosConfig.GetParent() );
    const KeyId roomIdToCapture( runConfig.GetKeyValue( RunSchema::roomIdKey ).ToKeyId() );

    return roomIdToCapture;
}

const WbConfig TrackRobotToolWidget::GetRoomLayoutConfig( const KeyId& roomIdToCapture )
{
    Collection roomsCollection( RoomsCollection() );
    roomsCollection.SetConfig(GetCurrentConfig());
    const WbConfig roomConfig( roomsCollection.ElementById( roomIdToCapture ) );
    return roomConfig.GetSubConfig( RoomLayoutSchema::schemaName );
}

const WbKeyValues::ValueIdPairList TrackRobotToolWidget::GetCameraPositionPairList(const KeyId& roomIdToCapture)
{
    const WbConfig roomLayoutConfig = GetRoomLayoutConfig(roomIdToCapture);
    return roomLayoutConfig.GetKeyValues( RoomLayoutSchema::cameraPositionIdsKey );
}

const QStringList TrackRobotToolWidget::GetCameraPositionIds(const KeyId& roomIdToCapture)
{
    const WbConfig roomLayoutConfig = GetRoomLayoutConfig(roomIdToCapture);
    const QStringList cameraPositionIds(
        roomLayoutConfig.GetKeyValue(RoomLayoutSchema::cameraPositionIdsKey)
        .ToQStringList() );
    return cameraPositionIds;
}

void TrackRobotToolWidget::AddTableRow( const QString& roomPosition, const QStringList& videoFileNames )
{
    const int ROOM_POSITION_COLUMN = 0;
    const int FILE_COLUMN = 1;
    const int newAppendedRow = m_ui->m_videoTable->rowCount();
    m_ui->m_videoTable->insertRow( newAppendedRow );
    if ( m_ui->m_videoTable->columnCount() < 2 )
    {
        m_ui->m_videoTable->insertColumn( FILE_COLUMN );
    }

    QTableWidgetItem* positionTableItem = new QTableWidgetItem( roomPosition );
    positionTableItem->setToolTip(tr("Position: ") + roomPosition);
    m_ui->m_videoTable->setItem(newAppendedRow, ROOM_POSITION_COLUMN, positionTableItem );

    QComboBox* videoSpinBox = new QComboBox();
    videoSpinBox->addItems( videoFileNames );
    m_ui->m_videoTable->setCellWidget( newAppendedRow, FILE_COLUMN, videoSpinBox );

    QStringList headerlabels;
    headerlabels << "Position" << "Source";
    m_ui->m_videoTable->setHorizontalHeaderLabels( headerlabels );
}

const WbSchema TrackRobotToolWidget::CreateSchema()
{
    using namespace TrackRobotSchema;

    WbSchema schema( CreateWorkbenchSubSchema( schemaName, tr( "Tracked Runs" ) ) );

    schema.AddSingleValueKey( robotIdKey, WbSchemaElement::Multiplicity::One );

    {
        using namespace GlobalTrackingParams;

        schema.AddKeyGroup(group,
                           WbSchemaElement::Multiplicity::One,
                           KeyNameList() <<
                               biLevelThreshold <<
                               nccThreshold <<
                               resolution,
                           DefaultValueMap()
                               .WithDefault(biLevelThreshold, KeyValue::from(BI_LEVEL_DEFAULT))
                               .WithDefault(nccThreshold,     KeyValue::from(NCC_DEFAULT))
                               .WithDefault(resolution,       KeyValue::from(RESOLUTION_DEFAULT)));
    }

    {
        using namespace PerCameraTrackingParams;

        schema.AddKeyGroup(group,
                           WbSchemaElement::Multiplicity::Many,
                           KeyNameList() <<
                               positionIdKey <<
                               useGlobalParams <<
                               biLevelThreshold <<
                               nccThreshold <<
                               resolution);
    }

    return schema;
}

void TrackRobotToolWidget::StepBackButtonClicked()
{
    TrackRun( m_optimumRate, false, true, false );

    // set play button icon
    SetButtonIcon(m_ui->m_playBtn,QString::fromUtf8(":/play.png"));

    // set step forward button icon
    SetButtonIcon(m_ui->m_stepBtn,QString::fromUtf8(":/step.png"));

    m_tracking = false;

}

void TrackRobotToolWidget::ScanBackButtonClicked()
{
    // disable buttons
    m_ui->m_scanFwdBtn->setEnabled(false);
    m_ui->m_stepBtn->setEnabled(false);
    m_ui->m_stepBackBtn->setEnabled(false);
    m_ui->m_stopBtn->setEnabled(false);

    // increment scan back index for vector
    if (m_scanBackIndex == m_scanBackIconRatePair.size() - 1)
    {
        m_scanBackIndex = 0; // loop back
    }
    else
    {
        m_scanBackIndex++;
    }

    // cycle icon
    SetButtonIcon(m_ui->m_scanBackBtn, QString::fromUtf8(m_scanBackIconRatePair[m_scanBackIndex].first.c_str()));

    // reverse * scan multiplier rate and DO NOT track
    int rate = m_optimumRate * m_scanBackIconRatePair[m_scanBackIndex].second;
    TrackRun( rate, false, false, false );

    // set pause button icon
    SetButtonIcon(m_ui->m_playBtn,QString::fromUtf8(":/pause.png"));

    m_playing = true;
}

void TrackRobotToolWidget::ScanForwardButtonClicked()
{
    // disable buttons
    m_ui->m_scanBackBtn->setEnabled(false);
    m_ui->m_stepBtn->setEnabled(false);
    m_ui->m_stepBackBtn->setEnabled(false);
    m_ui->m_stopBtn->setEnabled(false);

    // increment scan forward index for vector
    if (m_scanFwdIndex == m_scanFwdIconRatePair.size() - 1)
    {
        m_scanFwdIndex = 0; // loop back
    }
    else
    {
        m_scanFwdIndex++;
    }

    // cycle icon
    SetButtonIcon(m_ui->m_scanFwdBtn, QString::fromUtf8(m_scanFwdIconRatePair[m_scanFwdIndex].first.c_str()));

    // forward * scan multiplier rate and DO NOT track
    int rate = m_optimumRate * m_scanFwdIconRatePair[m_scanFwdIndex].second;
    TrackRun(rate, false, false, true );

    // set pause button icon
    SetButtonIcon(m_ui->m_playBtn,QString::fromUtf8(":/pause.png"));

    m_playing = true;
}


void TrackRobotToolWidget::PlayPauseButtonClicked()
{
    if (!m_playing) // play pressed
    {
        if ( m_tracking )
        {
            // play video AND track
            TrackRun( m_optimumRate, true, false, true );

            // switch to tracking/record icon
            SetButtonIcon(m_ui->m_playBtn,QString::fromUtf8(":/pauseTrack.png"));
        }
        else
        {
            // just play video
            TrackRun( m_optimumRate, false, false, true );

            // switch to pause icon
            SetButtonIcon(m_ui->m_playBtn,QString::fromUtf8(":/pause.png"));
        }

        // disable << |<< >>| []
        m_ui->m_scanBackBtn->setEnabled(false);
        m_ui->m_scanFwdBtn->setEnabled(false);
        m_ui->m_stepBackBtn->setEnabled(false);
        m_ui->m_stepBtn->setEnabled(false);
        m_ui->m_stopBtn->setEnabled(false);
    }
    else // paused pressed
    {
        m_scanFwdIndex = 0;
        m_scanBackIndex = 0;
        SetButtonIcon(m_ui->m_scanFwdBtn, QString::fromUtf8(m_scanFwdIconRatePair[0].first.c_str()));
        SetButtonIcon(m_ui->m_scanBackBtn, QString::fromUtf8(m_scanBackIconRatePair[0].first.c_str()));

        if ( m_tracking )
        {
            SetButtonIcon(m_ui->m_playBtn, QString::fromUtf8(":/playTrack.png"));
            SetButtonIcon(m_ui->m_stepBtn, QString::fromUtf8(":/stepTrack.png"));
        }
        else
        {
            SetButtonIcon(m_ui->m_playBtn, QString::fromUtf8(":/play.png"));
            SetButtonIcon(m_ui->m_stepBtn, QString::fromUtf8(":/step.png"));
        }

        // pause tracking
        TrackPause();

        // enable << |< >| >> []
        m_ui->m_scanBackBtn->setEnabled(true);
        m_ui->m_scanFwdBtn->setEnabled(true);
        m_ui->m_stepBackBtn->setEnabled(true);
        m_ui->m_stepBtn->setEnabled(true);
        m_ui->m_stopBtn->setEnabled(true);

    }

    // switch state of icon
    m_playing = !m_playing;
}

void TrackRobotToolWidget::StepButtonClicked()
{
    TrackRun( m_optimumRate, true, true, true );
}

void TrackRobotToolWidget::StopButtonClicked()
{
    TrackStop();

    ResetUi();
}

void TrackRobotToolWidget::TrackLoadButtonClicked()
{
    const WbConfig& config = GetCurrentConfig();

    LOG_TRACE("Track Load...");

    const WbConfig runConfig( config.GetParent() );

    bool successful = CreateRunResultDirectory( runConfig );

    if (successful)
    {
        UnknownLengthProgressDlg* const progressDialog = new UnknownLengthProgressDlg( this );
        progressDialog->Start( tr( "Loading..." ), tr( "" ) );

        ExitStatus::Flags exitCode = TrackLoad( config,
                                                m_ui->m_imageGrid,
                                                RobotTracker::KLT_TRACKER );

        successful = ( exitCode == ExitStatus::OK_TO_CONTINUE );

        if ( successful )
        {
            progressDialog->ForceClose();
        }
        else
        {
            progressDialog->ForceClose();

            Message::Show( 0,
                           tr( "Robot Tracking Tool" ),
                           tr( "See the log for details!" ),
                           Message::Severity_Critical );
        }
    }

    if (successful)
    {
        m_ui->m_loadBtn->setEnabled(false);
        m_ui->m_playBtn->setEnabled(true);
        m_ui->m_stepBtn->setEnabled(true);

        // enable global paramaters
        m_ui->m_nccThresholdSpinBox->setEnabled(false);
        m_ui->m_resolutionSpinBox->setEnabled(false);
        m_ui->m_trackerThresholdSpinBox->setEnabled(false);

        // enable camera specific parameters
        m_ui->m_camNccThresholdSpinBox->setEnabled(false);
        m_ui->m_camResolutionSpinBox->setEnabled(false);
        m_ui->m_camTrackerThresholdSpinBox->setEnabled(false);

        m_loaded = true;
    }
}

void TrackRobotToolWidget::TrackSaveButtonClicked()
{
    const WbConfig& config = GetCurrentConfig();

    LOG_TRACE("Track Post Process...");

    const WbConfig runConfig( config.GetParent() );

    Collection m_rooms = RoomsCollection();

    m_rooms.SetConfig( runConfig );

    const KeyValue roomId = runConfig.GetKeyValue( RunSchema::roomIdKey );
    const WbConfig roomConfig = m_rooms.ElementById( roomId.ToKeyId() );

    const WbConfig roomLayoutConfig(
                roomConfig.GetSubConfig( RoomLayoutSchema::schemaName ) );

    const QString floorPlanName(
                roomLayoutConfig.GetAbsoluteFileNameFor( "floor_plan.png" ) );

    const QString trackerResultsTxtName(
         runConfig.GetAbsoluteFileNameFor( "results/track_result_raw.txt" ) );
    const QString trackerResultsCsvName(
         runConfig.GetAbsoluteFileNameFor( "results/track_result_raw.csv" ) );
    const QString trackerResultsImgName(
         runConfig.GetAbsoluteFileNameFor( "results/track_result_img_raw.png" ) );

    const QString pixelOffsetsName(
         runConfig.GetAbsoluteFileNameFor( "results/pixel_offsets.txt" ) );

    UnknownLengthProgressDlg* const progressDialog = new UnknownLengthProgressDlg( this );
    progressDialog->Start( tr( "Saving..." ), tr( "" ) );

    ExitStatus::Flags exitCode = TrackPostProcess( floorPlanName.toAscii().data(),
                                                   trackerResultsTxtName.toAscii().data(),
                                                   trackerResultsCsvName.toAscii().data(),
                                                   trackerResultsImgName.toAscii().data(),
                                                   pixelOffsetsName.toAscii().data() );

    bool successful = ( exitCode == ExitStatus::OK_TO_CONTINUE );

    if ( successful )
    {
        progressDialog->Complete( tr( "Processing Successful" ),
                                  tr( "Results have been saved.\n" ) );
        m_ui->m_saveBtn->setEnabled(false);
    }
    else
    {
        progressDialog->ForceClose();
    }
}

void TrackRobotToolWidget::TrackResetButtonClicked()
{
    m_loaded = false;

    SetPosition(0);

    TrackReset( m_ui->m_imageGrid );

    m_ui->m_loadBtn->setEnabled(true);
    m_ui->m_saveBtn->setEnabled(false);
    m_ui->m_resetBtn->setEnabled(false);

    // disable global params
    m_ui->m_nccThresholdSpinBox->setEnabled(true);
    m_ui->m_resolutionSpinBox->setEnabled(true);
    m_ui->m_trackerThresholdSpinBox->setEnabled(true);

    // disable camera specific params
    m_ui->m_camNccThresholdSpinBox->setEnabled(true);
    m_ui->m_camResolutionSpinBox->setEnabled(true);
    m_ui->m_camTrackerThresholdSpinBox->setEnabled(true);

    m_fpsSet = false;
}

void TrackRobotToolWidget::Playing()
{
#if 0
    m_ui->m_pauseBtn->setEnabled(true);
    m_ui->m_stopBtn->setEnabled(true);
#endif
}

void TrackRobotToolWidget::Paused()
{
    m_ui->m_stepBackBtn->setEnabled(true);
    m_ui->m_scanBackBtn->setEnabled(true);
    m_ui->m_scanFwdBtn->setEnabled(true);
    m_ui->m_playBtn->setEnabled(true);
    m_ui->m_stepBtn->setEnabled(true);
    m_ui->m_stopBtn->setEnabled(true);

    if (m_tracking)
    {
        // switch to tracking icons
        SetButtonIcon(m_ui->m_playBtn,QString::fromUtf8(":/playTrack.png"));
        SetButtonIcon(m_ui->m_stepBtn,QString::fromUtf8(":/stepTrack.png"));
    }
    else
    {
        // switch to (non-tracking) icons
        SetButtonIcon(m_ui->m_playBtn, QString::fromUtf8(":/play.png"));
        SetButtonIcon(m_ui->m_stepBtn, QString::fromUtf8(":/step.png"));
    }
}

void TrackRobotToolWidget::Stopped()
{
    // tracking finished
    m_ui->m_stopBtn->setEnabled(false);
    m_ui->m_resetBtn->setEnabled(true);
    m_ui->m_saveBtn->setEnabled(true);

    // switch to (non-tracking) icons
    SetButtonIcon(m_ui->m_playBtn,QString::fromUtf8(":/play.png"));
    SetButtonIcon(m_ui->m_stepBtn,QString::fromUtf8(":/step.png"));
}

void TrackRobotToolWidget::SetButtonIcon(QToolButton* button, QString iconImage)
{
    QIcon icon;
    icon.addFile(iconImage, QSize(), QIcon::Normal, QIcon::Off);
    button->setIcon(icon);
}

void TrackRobotToolWidget::SelectTrack( int id, int x, int y )
{
    m_scene.SetTrackPosition( id, x, y );

    if (!m_tracking)
    {
        // switch to tracking icons
        SetButtonIcon(m_ui->m_playBtn,QString::fromUtf8(":/playTrack.png"));
        SetButtonIcon(m_ui->m_stepBtn,QString::fromUtf8(":/stepTrack.png"));
    }

    m_tracking = true;
}

void TrackRobotToolWidget::ClearTrack( int id )
{
    m_scene.ClrTrackPosition( id );

    if (m_tracking)
    {
        // switch to tracking icons
        SetButtonIcon(m_ui->m_playBtn,QString::fromUtf8(":/play.png"));
        SetButtonIcon(m_ui->m_stepBtn,QString::fromUtf8(":/step.png"));
    }

    m_tracking = false;
}

void TrackRobotToolWidget::ThreadPaused( bool trackingLost )
{
    m_running = false;
    m_playing = false;

    m_tracking = m_tracking && !trackingLost;

    Paused();
}

void TrackRobotToolWidget::ThreadFinished()
{
    m_running = false;
    m_playing = false;
    m_tracking = false;

    Stopped();
}

void TrackRobotToolWidget::ImageUpdate( int id, const QImage& image, double fps )
{
    // Set optimum fps rate if not already set or
    // new fps is less than old smaller > larger.

    if ( (fps > 0 && !m_fpsSet) || (m_fpsSet && fps < m_fps) )
    {
        m_fps = fps;
        m_optimumRate = (int) (1000/m_fps);
        m_fpsSet = true;
    }

    emit UpdateImage( id, image, fps );
}

void TrackRobotToolWidget::ImageSet( int id, const QImage& image, double fps )
{
    emit SetImage( id, image, fps );
}

void TrackRobotToolWidget::SetPosition( double position )
{
    QTime time = QTime(0,0);
    m_ui->m_timeLineEdit->setText( time.addMSecs(position).toString("hh:mm:ss:zzz") );
}

// ----------------------------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------------------

const ExitStatus::Flags TrackRobotToolWidget::TrackLoad( const WbConfig&           trackConfig,
											             ImageGrid*                imageGrid,
											             RobotTracker::trackerType tracker )
{
    ExitStatus::Flags exitStatus = ExitStatus::OK_TO_CONTINUE;

    bool successful = true;

    Collection rooms  ( RoomsCollection() );
    Collection cameras ( CamerasCollection() );
    Collection camerasPositions ( CameraPositionsCollection() );
    Collection robots ( RobotsCollection() );
    Collection targets ( TargetsCollection() );

    rooms.SetConfig( trackConfig );
    cameras.SetConfig( trackConfig );
    camerasPositions.SetConfig( trackConfig );
    robots.SetConfig( trackConfig );
    targets.SetConfig( trackConfig );

    // Get the run configuration...
    const WbConfig& runConfig = trackConfig.GetParent();

    // Get the room configuration (for this run)...
    const KeyId roomId = runConfig.GetKeyValue( RunSchema::roomIdKey ).ToKeyId();
    const WbConfig roomConfig = rooms.ElementById( roomId );

    // Get the robot configuration (for this run)...
    const KeyId robotId = trackConfig.GetKeyValue( TrackRobotSchema::robotIdKey ).ToKeyId();
    const WbConfig& robotConfig = robots.ElementById( robotId );

    // Get the robot metrics configuration...
    const WbConfig& metricsConfig = robotConfig.GetSubConfig( RobotMetricsSchema::schemaName );

    // Get the target configuration (for this robot)...
    const KeyId targetId = metricsConfig.GetKeyValue( RobotMetricsSchema::targetTypeKey ).ToKeyId();
    const WbConfig& targetConfig = targets.ElementById( targetId );

    // Get the target params configuration...
    const WbConfig& paramsConfig = targetConfig.GetSubConfig( TargetSchema::schemaName );

    // Get the room configuration...
    const WbConfig roomLayoutConfig( roomConfig.GetSubConfig( RoomLayoutSchema::schemaName ) );
    const QStringList cameraPositionIds( roomLayoutConfig
            .GetKeyValue(RoomLayoutSchema::cameraPositionIdsKey)
             .ToQStringList() );

    // Multi-camera ground truth system
    m_scene.LoadTarget( paramsConfig );

    for ( int i = 0; i < cameraPositionIds.size() && successful; ++i )
    {
        const KeyId camPosId( cameraPositionIds.at( i ) );
        const WbConfig camPosConfig(
                    camerasPositions.ElementById( camPosId ) );

        const KeyId cameraId( camPosConfig.GetKeyValue(
                    CameraPositionSchema::cameraIdKey )
                        .ToQString() );
        const WbConfig cameraConfig( cameras.ElementById( cameraId ) );

        const VideoCaptureEntry& captureEntry = m_mapPositionsToFiles.value( camPosId );

        const QString videoFileName = trackConfig.GetAbsoluteFileNameFor( captureEntry.first.at(0) );
        const QString timestampFileName = trackConfig.GetAbsoluteFileNameFor( captureEntry.second );

        successful = m_scene.LoadCameraConfig( camPosId,
                                               videoFileName.toAscii().data(),
                                               timestampFileName.toAscii().data(),
                                               cameraConfig,
                                               camPosConfig,
                                               roomConfig,
                                               robotConfig,
                                               trackConfig,
                                               tracker );
    }

    if (successful)
    {
        m_scene.SetupViewWindows( this, imageGrid );

        m_scene.SetupThread( this );
    }

    if (!successful)
    {
        exitStatus = ExitStatus::ERRORS_OCCURRED;
    }

    return exitStatus;
}

const ExitStatus::Flags TrackRobotToolWidget::TrackRun( double rate, bool trackingActive,
                                                                     bool singleStep,
                                                                     bool runForward )
{
    ExitStatus::Flags exitStatus = ExitStatus::OK_TO_CONTINUE;

    Playing();

    m_running = true;

    m_scene.StartThread( rate, trackingActive, singleStep, runForward );

    return exitStatus;
}

const ExitStatus::Flags TrackRobotToolWidget::TrackPause()
{
    ExitStatus::Flags exitStatus = ExitStatus::OK_TO_CONTINUE;

    m_scene.PauseThread();

    return exitStatus;
}

const ExitStatus::Flags TrackRobotToolWidget::TrackStop()
{
    ExitStatus::Flags exitStatus = ExitStatus::OK_TO_CONTINUE;
    m_scene.StopThread();

    return exitStatus;
}

const ExitStatus::Flags TrackRobotToolWidget::TrackPostProcess( char* floorPlanFile,
                                                                char* trackerResultsTxtFile,
                                                                char* trackerResultsCsvFile,
                                                                char* trackerResultsImgFile,
                                                                char* pixelOffsetsFile )
{
    ExitStatus::Flags exitStatus = ExitStatus::OK_TO_CONTINUE;

    m_scene.PostProcess( floorPlanFile,
                         trackerResultsTxtFile,
                         trackerResultsCsvFile,
                         trackerResultsImgFile,
                         pixelOffsetsFile );

    return exitStatus;
}

const ExitStatus::Flags TrackRobotToolWidget::TrackReset( ImageGrid* imageGrid )
{
    ExitStatus::Flags exitStatus = ExitStatus::OK_TO_CONTINUE;

    m_scene.DestroyViewWindows( imageGrid );

    m_scene.Reset();

    return exitStatus;
}

// ----------------------------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------------------

bool TrackRobotToolWidget::CreateRunResultDirectory(const WbConfig& config)
{
    QDir m_resultDir( config.GetAbsoluteFileNameFor( "results" ) );

    const QString resultDirName = m_resultDir.dirName();

    QDir resultDirParent( m_resultDir );

    if ( !resultDirParent.cdUp() )
    {
        Message::Show( this,
                       tr( "Robot Tracking Tool" ),
                       tr( "Error - Save Workbench!" ),
                       Message::Severity_Critical );

        return false;
    }

    if (resultDirParent.exists( resultDirName ))
    {
        QMessageBox mb;
        mb.setText(QObject::tr("Robot Tracking Tool"));
        mb.setInformativeText(QObject::tr( "Query - Overwrite data?"));
        mb.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        int ret = mb.exec();

        if (ret == QMessageBox::Cancel)
        {
            return false;
        }

        FileUtilities::DeleteDirectory( resultDirParent.absoluteFilePath(resultDirName) );
    }

    if ( !resultDirParent.mkdir( resultDirName ) || !resultDirParent.cd( resultDirName ))
    {
        Message::Show( this,
                       tr( "Robot Tracking Tool" ),
                       tr( "Error - Missing folder!" ),
                       Message::Severity_Critical );

        return false;
    }

    return true;
}

