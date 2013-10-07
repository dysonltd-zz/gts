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

#include "TrackRobotWidget.h"

#include "ui_TrackRobotWidget.h"

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

#include "RoomsCollection.h"
#include "CamerasCollection.h"
#include "CameraPositionsCollection.h"
#include "RobotsCollection.h"
#include "TargetsCollection.h"

#include "Message.h"
#include "Logging.h"
#include "UnknownLengthProgressDlg.h"
#include "FileDialogs.h"
#include "FileUtilities.h"

#include <QtCore/QDir>
#include <QtGui/QMessageBox>
#include <QApplication>
#include <QShortcut>
#include <QtGlobal>

#include <sstream>

static const int BI_LEVEL_DEFAULT = 128;
static const double NCC_DEFAULT = 0.3;
static const double RESOLUTION_DEFAULT = 15.0;

static const int ROOM_POSITION_COLUMN = 0;
static const int FILE_COLUMN = 1;

static const int NUM_COLS = 2;

TrackRobotWidget::TrackRobotWidget( QWidget* parent ) :
    Tool       ( parent, CreateSchema() ),
    m_ui       ( new Ui::TrackRobotWidget ),
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

    // forward button
    m_scanFwdIconRatePair.push_back(std::make_pair(":/scan.png", 1));
    m_scanFwdIconRatePair.push_back(std::make_pair(":/scan2x.png", 2));
    m_scanFwdIconRatePair.push_back(std::make_pair(":/scan4x.png", 4));
    m_scanFwdIconRatePair.push_back(std::make_pair(":/scan10x.png", 10));
    m_scanFwdIconRatePair.push_back(std::make_pair(":/scan20x.png", 20));

    // reverse button
    m_scanBackIconRatePair.push_back(std::make_pair(":/revscan.png", 1));
    m_scanBackIconRatePair.push_back(std::make_pair(":/revscan2x.png", 2));
    m_scanBackIconRatePair.push_back(std::make_pair(":/revscan4x.png", 4));
    m_scanBackIconRatePair.push_back(std::make_pair(":/revscan10x.png", 10));
    m_scanBackIconRatePair.push_back(std::make_pair(":/revscan20x.png", 20));
}

void TrackRobotWidget::SetupUi()
{
    m_ui->setupUi( this );

    m_ui->m_videoTable->setColumnCount( NUM_COLS );
    m_ui->m_videoTable->setSortingEnabled( false );
}

void TrackRobotWidget::FillOutCameraCombo( QComboBox& comboBox )
{
    WbConfig config = GetCurrentConfig();

    Collection camPosCollection( CameraPositionsCollection() );
    Collection rooms ( RoomsCollection() );

    camPosCollection.SetConfig( config );
    rooms.SetConfig( config );

    comboBox.clear();

    // Get the run configuration
    const WbConfig& runConfig = config.GetParent();

    // Get the room configuration (for this run)
    const KeyId roomId = runConfig.GetKeyValue( RunSchema::roomIdKey ).ToKeyId();
    const WbConfig roomConfig = rooms.ElementById( roomId );

    // Get the room configuration
    const WbConfig roomLayoutConfig( roomConfig.GetSubConfig( RoomLayoutSchema::schemaName ) );
    const QStringList camPosIds( roomLayoutConfig
            .GetKeyValue(RoomLayoutSchema::cameraPositionIdsKey)
             .ToQStringList() );

    for (auto camPosId = camPosIds.begin(); camPosId != camPosIds.end(); ++camPosId)
    {
        comboBox.addItem( WbConfigTools::DisplayNameOf(camPosCollection.ElementById(*camPosId)), QVariant( *camPosId ) );
    }
}

void TrackRobotWidget::ResetUi()
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

void TrackRobotWidget::ConnectSignals()
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
    QObject::connect( m_ui->m_useGlobalCheckBox,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( UseGlobalBtnClicked() ) );
    QObject::connect( m_ui->btnSave,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( SaveBtnClicked() ) );
}

void TrackRobotWidget::SetupKeyboardShortcuts()
{
    // Keyboard shorcuts
    QShortcut *playPauseTrackKey = new QShortcut(Qt::Key_Space, this);
    QShortcut *stepBackKey = new QShortcut(Qt::Key_Left, this);
    QShortcut *stepForwardKey = new QShortcut(Qt::Key_Right, this);
    QShortcut *scanBackKey = new QShortcut(QKeySequence(Qt::CTRL+Qt::Key_Left), this);
    QShortcut *stopKey = new QShortcut(Qt::Key_Return, this);

    QObject::connect( playPauseTrackKey,
             SIGNAL( activated() ),
             this,
             SLOT( PlayPauseButtonClicked() ) );
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
}

void TrackRobotWidget::CreateMappers()
{
    AddMapper( TrackRobotSchema::robotIdKey, m_ui->m_robotCombo );

    using namespace TrackRobotSchema::GlobalTrackingParams;

    AddMapper(biLevelThreshold, m_ui->m_trackerThresholdSpinBox);
    AddMapper(nccThreshold,     m_ui->m_nccThresholdSpinBox);
    AddMapper(resolution,       m_ui->m_resolutionSpinBox);
}

const QString TrackRobotWidget::GetCameraId() const
{
    QComboBox* const cameraCombo = m_ui->m_positionCombo;
    const int newTargetIndex = cameraCombo->currentIndex();
    const QString cameraId( cameraCombo->itemData( newTargetIndex ).toString() );
    return cameraId;
}

void TrackRobotWidget::PopulateCameraParams()
{
}

TrackRobotWidget::~TrackRobotWidget()
{
    delete m_ui;
}

const QString TrackRobotWidget::GetSubSchemaDefaultFileName() const
{
    return "trackerOutput.xml";
}

void TrackRobotWidget::ReloadCurrentConfigToolSpecific()
{
    const WbConfig& config = GetCurrentConfig();

    FillOutCameraCombo( *m_ui->m_positionCombo );

    // Get the selected room name
    const WbConfig& runConfig = config.GetParent();
    const WbConfig& videoConfig = runConfig.GetSubConfig( VideoCaptureSchema::schemaName );

    // Get id keys for all the camera positions in the selected room:
    KeyId roomId = GetRoomIdToCapture();
    const WbKeyValues::ValueIdPairList positions = GetCameraPositionPairList( roomId );
    const WbKeyValues::ValueIdPairList capturedVideos = videoConfig.GetKeyValues( VideoCaptureSchema::cameraPositionIdKey );
    if ( !capturedVideos.empty() )
    {
        m_ui->m_loadBtn->setEnabled(true);
    }

    // Create a map from positions to lists of video file names:
    m_ui->m_videoTable->clear();
    m_ui->m_videoTable->setRowCount(0);

    QStringList headerlabels;
    headerlabels << "Position" << "Source";
    m_ui->m_videoTable->setHorizontalHeaderLabels( headerlabels );

    m_mapPositionsToFiles.clear();

    for (auto p = positions.begin(); p != positions.end(); ++p)
    {
        WbKeyValues::ValueIdPair const &key = *p;

        // For each camera position in the room
        const QStringList& positionStrings = key.value.ToQStringList();
        for (auto ps = positionStrings.begin(); ps != positionStrings.end(); ++ps)
        {
            QString const &positionString = *ps;

            // For every captured video
            for (auto cv = capturedVideos.begin(); cv != capturedVideos.end(); ++cv)
            {
                WbKeyValues::ValueIdPair const &v = *cv;

                // check if it was captured at the current camera position:
                if ( v.value.ToQString() == positionString )
                {
                    // If it was add it to the map of position -> videoFiles
                    QString videoFilename = videoConfig.GetKeyValue( VideoCaptureSchema::videoFileNameKey, v.id ).ToQString();
                    QString timestampFilename = videoConfig.GetKeyValue( VideoCaptureSchema::timestampFileNameKey, v.id ).ToQString();

                    m_mapPositionsToFiles[ positionString ].first << videoFilename;
                    m_mapPositionsToFiles[ positionString ].second << timestampFilename;
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

void TrackRobotWidget::CameraComboChanged()
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

    m_ui->m_useGlobalCheckBox->setChecked(useGlobalParams);

    m_ui->m_camTrackerThresholdSpinBox->setValue(biLevelThreshold);
    m_ui->m_camNccThresholdSpinBox->setValue(nccThreshold);
    m_ui->m_camResolutionSpinBox->setValue(resolution);

    m_ui->m_camTrackerThresholdSpinBox->setEnabled(!useGlobalParams);
    m_ui->m_camNccThresholdSpinBox->setEnabled(!useGlobalParams);
    m_ui->m_camResolutionSpinBox->setEnabled(!useGlobalParams);
}

void TrackRobotWidget::UseGlobalBtnClicked()
{
    if ( m_ui->m_useGlobalCheckBox->isChecked() )
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

void TrackRobotWidget::SaveBtnClicked()
{
    WbConfig config = GetCurrentConfig();

    const KeyId cameraId = KeyId(GetCameraId());

    using namespace TrackRobotSchema::PerCameraTrackingParams;

    Collection camPosCollection( CameraPositionsCollection() );
    Collection rooms ( RoomsCollection() );

    camPosCollection.SetConfig( config );
    rooms.SetConfig( config );

    std::vector< KeyId > idsToKeep;

    // Get the run configuration
    const WbConfig& runConfig = config.GetParent();

    // Get the room configuration (for this run)
    const KeyId roomId = runConfig.GetKeyValue( RunSchema::roomIdKey ).ToKeyId();
    const WbConfig roomConfig = rooms.ElementById( roomId );

    // Get the room configuration
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

    bool valid = true;

    valid = valid && !(m_ui->m_camNccThresholdSpinBox->value() == 0.0);
    valid = valid && !(m_ui->m_camResolutionSpinBox->value() == 0);
    valid = valid && !(m_ui->m_camTrackerThresholdSpinBox->value() == 0);

    if (valid)
    {
        config.KeepKeys( positionIdKey, idsToKeep );
        config.KeepKeys( useGlobalParams, idsToKeep );
        config.KeepKeys( biLevelThreshold, idsToKeep );
        config.KeepKeys( nccThreshold, idsToKeep );
        config.KeepKeys( resolution, idsToKeep );

        const KeyId cameraKeyId = config.AddKeyValue( positionIdKey,
                                                      KeyValue::from( cameraId ) );

        config.SetKeyValue( useGlobalParams,
                            KeyValue::from( m_ui->m_useGlobalCheckBox->isChecked() ),
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
}

void TrackRobotWidget::AddVideoFileConfigKey( const QString& videoFileName,
                                                  const KeyId& camPosId)
{
    WbConfig config(GetCurrentConfig());
    const KeyId keyId = config.AddKeyValue(TrackRobotSchema::videoFileNameKey, KeyValue::from(videoFileName));
    config.SetKeyValue(TrackRobotSchema::cameraPositionIdKey, KeyValue::from(camPosId), keyId);
}

const bool TrackRobotWidget::CreateVideoDirectory( const QString& videoDirectoryName )
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
                       tr( "Failed to create directory: %1!" )
                         .arg( outputDirectory.absolutePath() ),
                       Message::Severity_Critical );
        return false;
    }

    return true;
}

void TrackRobotWidget::ShowNoRoomError()
{
    Message::Show( this,
                   tr( "Track Robot" ),
                   tr( "There is no room selected!" ),
                   Message::Severity_Critical );
}

void TrackRobotWidget::ShowEmptyRoomError()
{
    Message::Show( this,
                   tr( "Track Robot" ),
                   tr( "The selected room is empty!" ),
                   Message::Severity_Critical );
}

const KeyId TrackRobotWidget::GetRoomIdToCapture() const
{
    const WbConfig trackRobotConfig( GetCurrentConfig() );
    const WbConfig runConfig( trackRobotConfig.GetParent() );
    const KeyId roomIdToCapture( runConfig.GetKeyValue( RunSchema::roomIdKey ).ToKeyId() );

    return roomIdToCapture;
}

const WbConfig TrackRobotWidget::GetRoomLayoutConfig( const KeyId& roomIdToCapture )
{
    Collection roomsCollection( RoomsCollection() );
    roomsCollection.SetConfig(GetCurrentConfig());
    const WbConfig roomConfig( roomsCollection.ElementById( roomIdToCapture ) );
    return roomConfig.GetSubConfig( RoomLayoutSchema::schemaName );
}

const WbKeyValues::ValueIdPairList TrackRobotWidget::GetCameraPositionPairList(const KeyId& roomIdToCapture)
{
    const WbConfig roomLayoutConfig = GetRoomLayoutConfig(roomIdToCapture);
    return roomLayoutConfig.GetKeyValues( RoomLayoutSchema::cameraPositionIdsKey );
}

const QStringList TrackRobotWidget::GetCameraPositionIds(const KeyId& roomIdToCapture)
{
    const WbConfig roomLayoutConfig = GetRoomLayoutConfig(roomIdToCapture);
    const QStringList cameraPositionIds(
        roomLayoutConfig.GetKeyValue(RoomLayoutSchema::cameraPositionIdsKey)
        .ToQStringList() );
    return cameraPositionIds;
}

void TrackRobotWidget::AddTableRow( const QString& roomPosition, const QStringList& videoFileNames )
{
    const int newAppendedRow = m_ui->m_videoTable->rowCount();

    m_ui->m_videoTable->insertRow( newAppendedRow );

    QTableWidgetItem* positionTableItem = new QTableWidgetItem( roomPosition );
    positionTableItem->setToolTip(tr("Position: ") + roomPosition);

    m_ui->m_videoTable->setItem(newAppendedRow, ROOM_POSITION_COLUMN, positionTableItem );

    QComboBox* videoSpinBox = new QComboBox();
    videoSpinBox->addItems( videoFileNames );

    m_ui->m_videoTable->setCellWidget( newAppendedRow, FILE_COLUMN, videoSpinBox );
}

bool TrackRobotWidget::IsDataValid() const
{
    if (GetCurrentConfig().IsNull()) return true;

    bool valid = true;

    if ( m_ui->m_robotCombo->currentText().isEmpty() )
    {
        valid = valid && false;
        Tool::HighlightLabel(m_ui->m_robotLabel, true);
    }
    else
    {
        Tool::HighlightLabel(m_ui->m_robotLabel, false);
    }

    return valid;
}

bool TrackRobotWidget::CanClose() const
{
    return IsDataValid();
}

const QString TrackRobotWidget::CannotCloseReason() const
{
    return tr("Please complete all highlighted boxes before leaving this tab.");
}

const WbSchema TrackRobotWidget::CreateSchema()
{
    using namespace TrackRobotSchema;

    WbSchema schema( CreateWorkbenchSubSchema( schemaName, tr( "Track Robot" ) ) );

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

void TrackRobotWidget::StepBackButtonClicked()
{
    TrackRun( m_optimumRate, false, true, false );

    // set play button icon
    SetButtonIcon(m_ui->m_playBtn,QString::fromUtf8(":/play.png"));

    // set step forward button icon
    SetButtonIcon(m_ui->m_stepBtn,QString::fromUtf8(":/step.png"));

    m_tracking = false;
}

void TrackRobotWidget::ScanBackButtonClicked()
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
    SetButtonIcon(m_ui->m_playBtn, QString::fromUtf8(":/pause.png"));

    m_playing = true;
}

void TrackRobotWidget::ScanForwardButtonClicked()
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
    SetButtonIcon(m_ui->m_playBtn, QString::fromUtf8(":/pause.png"));

    m_playing = true;
}


void TrackRobotWidget::PlayPauseButtonClicked()
{
    if (!m_playing) // play pressed
    {
        if ( m_tracking )
        {
            // play video AND track
            TrackRun( m_optimumRate, true, false, true );

            // switch to tracking/record icon
            SetButtonIcon(m_ui->m_playBtn, QString::fromUtf8(":/pauseTrack.png"));
        }
        else
        {
            // just play video
            TrackRun( m_optimumRate, false, false, true );

            // switch to pause icon
            SetButtonIcon(m_ui->m_playBtn, QString::fromUtf8(":/pause.png"));
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

void TrackRobotWidget::StepButtonClicked()
{
    TrackRun( m_optimumRate, true, true, true );
}

void TrackRobotWidget::StopButtonClicked()
{
    TrackStop();

    ResetUi();
}

void TrackRobotWidget::TrackLoadButtonClicked()
{
    const WbConfig& config = GetCurrentConfig();

    LOG_TRACE("Track Load");

    const KeyId roomIdToCapture(GetRoomIdToCapture());
    if (roomIdToCapture.isEmpty()) { ShowNoRoomError(); return; }

    const QStringList cameraPositionIds(GetCameraPositionIds(roomIdToCapture));
    if (cameraPositionIds.size() == 0) { ShowEmptyRoomError(); return; }

    const WbConfig runConfig( config.GetParent() );

    bool successful = CreateRunResultDirectory( runConfig );

    if (successful)
    {
        UnknownLengthProgressDlg* const progressDialog = new UnknownLengthProgressDlg( this );
        progressDialog->Start( tr( "Loading" ), tr( "" ) );

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

        SetupKeyboardShortcuts();

        m_loaded = true;
    }
}

void TrackRobotWidget::TrackSaveButtonClicked()
{
    const WbConfig& config = GetCurrentConfig();

    LOG_TRACE("Track Save Data");

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

    const QString trackResultsTemplate(
         runConfig.GetAbsoluteFileNameFor( "results/track_result_view%1.txt" ) );
    const QString pixelOffsetsTemplate(
         runConfig.GetAbsoluteFileNameFor( "results/pixel_offsets_view%1.txt" ) );

    UnknownLengthProgressDlg* const progressDialog = new UnknownLengthProgressDlg( this );
    progressDialog->Start( tr( "Saving" ), tr( "" ) );

    ExitStatus::Flags exitCode = TrackSaveData( floorPlanName.toAscii().data(),
                                                trackerResultsTxtName.toAscii().data(),
                                                trackerResultsCsvName.toAscii().data(),
                                                trackerResultsImgName.toAscii().data(),
                                                pixelOffsetsName.toAscii().data(),
                                                trackResultsTemplate,
                                                pixelOffsetsTemplate );

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

void TrackRobotWidget::TrackResetButtonClicked()
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

void TrackRobotWidget::Playing()
{
#if 0
    m_ui->m_pauseBtn->setEnabled(true);
    m_ui->m_stopBtn->setEnabled(true);
#endif
}

void TrackRobotWidget::Paused()
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

void TrackRobotWidget::Stopped()
{
    // tracking finished
    m_ui->m_stopBtn->setEnabled(false);
    m_ui->m_resetBtn->setEnabled(true);
    m_ui->m_saveBtn->setEnabled(true);

    // switch to (non-tracking) icons
    SetButtonIcon(m_ui->m_playBtn,QString::fromUtf8(":/play.png"));
    SetButtonIcon(m_ui->m_stepBtn,QString::fromUtf8(":/step.png"));
}

void TrackRobotWidget::SetButtonIcon(QToolButton* button, QString iconImage)
{
    QIcon icon;
    icon.addFile(iconImage, QSize(), QIcon::Normal, QIcon::Off);
    button->setIcon(icon);
}

void TrackRobotWidget::SelectTrack( int id, int x, int y )
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

void TrackRobotWidget::ClearTrack( int id )
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

void TrackRobotWidget::ThreadPaused( bool trackingLost )
{
    m_running = false;
    m_playing = false;

    m_tracking = m_tracking && !trackingLost;

    Paused();
}

void TrackRobotWidget::ThreadFinished()
{
    m_running = false;
    m_playing = false;
    m_tracking = false;

    Stopped();
}

void TrackRobotWidget::ImageUpdate( int id, const QImage& image, double fps )
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

void TrackRobotWidget::ImageSet( int id, const QImage& image, double fps )
{
    emit SetImage( id, image, fps );
}

void TrackRobotWidget::SetPosition( double position )
{
    QTime time = QTime(0,0);
    m_ui->m_timeLineEdit->setText( time.addMSecs(position).toString("hh:mm:ss:zzz") );
}

// ----------------------------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------------------

const ExitStatus::Flags TrackRobotWidget::TrackLoad( const WbConfig&               trackConfig,
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

    // Get the run configuration
    const WbConfig& runConfig = trackConfig.GetParent();

    // Get the room configuration (for this run)
    const KeyId roomId = runConfig.GetKeyValue( RunSchema::roomIdKey ).ToKeyId();
    const WbConfig roomConfig = rooms.ElementById( roomId );

    // Get the robot configuration (for this run)
    const KeyId robotId = trackConfig.GetKeyValue( TrackRobotSchema::robotIdKey ).ToKeyId();
    const WbConfig& robotConfig = robots.ElementById( robotId );

    // Get the robot metrics configuration
    const WbConfig& metricsConfig = robotConfig.GetSubConfig( RobotMetricsSchema::schemaName );

    // Get the target configuration (for this robot)
    const KeyId targetId = metricsConfig.GetKeyValue( RobotMetricsSchema::targetTypeKey ).ToKeyId();
    const WbConfig& targetConfig = targets.ElementById( targetId );

    // Get the target params configuration
    const WbConfig& paramsConfig = targetConfig.GetSubConfig( TargetSchema::schemaName );

    // Get the room configuration
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

        QComboBox* combo = (QComboBox*)m_ui->m_videoTable->cellWidget(i, FILE_COLUMN);

        if (!combo)
        {
            Message::Show( 0,
                           tr( "Robot Tracking Tool" ),
                           tr( "No videos found for this run" ),
                           Message::Severity_Critical );
            ResetUi();
            return ExitStatus::ERRORS_OCCURRED;;
        }

        const VideoCaptureEntry& captureEntry = m_mapPositionsToFiles.value( camPosId );
        const QString videoFileName =
           trackConfig.GetAbsoluteFileNameFor( captureEntry.first.at(combo->currentIndex()) );
        const QString timestampFileName =
           trackConfig.GetAbsoluteFileNameFor( captureEntry.second.at(combo->currentIndex()) );

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

const ExitStatus::Flags TrackRobotWidget::TrackRun( double rate, bool trackingActive,
                                                                     bool singleStep,
                                                                     bool runForward )
{
    ExitStatus::Flags exitStatus = ExitStatus::OK_TO_CONTINUE;

    Playing();
    m_running = true;
    m_scene.StartThread( rate, trackingActive, singleStep, runForward );

    return exitStatus;
}

const ExitStatus::Flags TrackRobotWidget::TrackPause()
{
    ExitStatus::Flags exitStatus = ExitStatus::OK_TO_CONTINUE;

    m_scene.PauseThread();

    return exitStatus;
}

const ExitStatus::Flags TrackRobotWidget::TrackStop()
{
    ExitStatus::Flags exitStatus = ExitStatus::OK_TO_CONTINUE;
    m_scene.StopThread();

    return exitStatus;
}

const ExitStatus::Flags TrackRobotWidget::TrackSaveData( char* floorPlanFile,
                                                             char* trackerResultsTxtFile,
                                                             char* trackerResultsCsvFile,
                                                             char* trackerResultsImgFile,
                                                             char* pixelOffsetsFile,
                                                             QString trackResultsTemplate,
                                                             QString pixelOffsetsTemplate )
{
    ExitStatus::Flags exitStatus = ExitStatus::OK_TO_CONTINUE;

    m_scene.SaveData( floorPlanFile,
                      trackerResultsTxtFile,
                      trackerResultsCsvFile,
                      trackerResultsImgFile,
                      pixelOffsetsFile,
                      trackResultsTemplate,
                      pixelOffsetsTemplate );

    return exitStatus;
}

const ExitStatus::Flags TrackRobotWidget::TrackReset( ImageGrid* imageGrid )
{
    ExitStatus::Flags exitStatus = ExitStatus::OK_TO_CONTINUE;
    m_scene.DestroyViewWindows( imageGrid );
    m_scene.Reset();

    return exitStatus;
}

// ----------------------------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------------------

bool TrackRobotWidget::CreateRunResultDirectory(const WbConfig& config)
{
    QDir m_resultDir( config.GetAbsoluteFileNameFor( "results" ) );

    const QString resultDirName = m_resultDir.dirName();

    QDir resultDirParent( m_resultDir );

    if ( !resultDirParent.cdUp() )
    {
        Message::Show( this,
                       tr( "Robot Tracking Tool" ),
                       tr( "Please save your workbench" ),
                       Message::Severity_Critical );

        return false;
    }

    if (resultDirParent.exists( resultDirName ))
    {
        const int result = QMessageBox::question( this,
                                                  "Confirm delete",
                                                  QObject::tr("Are you sure you want to overwrite already saved track data."),
                                                  QMessageBox::Yes|QMessageBox::No );
        if (result == QMessageBox::No)
        {
            return false;
        }

        FileUtilities::DeleteDirectory( resultDirParent.absoluteFilePath(resultDirName) );
    }

    if ( !resultDirParent.mkdir( resultDirName ) || !resultDirParent.cd( resultDirName ))
    {
        Message::Show( this,
                       tr( "Robot Tracking Tool" ),
                       tr( "Results directory cannot be found" ),
                       Message::Severity_Critical );

        return false;
    }

    return true;
}
