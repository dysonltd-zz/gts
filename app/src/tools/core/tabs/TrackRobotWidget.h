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

#ifndef TRACKROBOTWIDGET_H
#define TRACKROBOTWIDGET_H

#include <QtGui/QWidget>
#include <QtGui/QTableWidget>
#include <QtGui/QToolButton>
#include <QPair>
#include <QMap>
#include <QTime>

#include "Tool.h"
#include "ImageGrid.h"
#include "RobotTracker.h"
#include "AlgorithmInterface.h"
#include "GtsScene.h"

#include "WbConfigTools.h"

#include <vector>
#include <utility>

class GtsScene;

namespace Ui
{
    class TrackRobotWidget;
}

class TrackRobotWidget : public Tool
{
    Q_OBJECT

public:
    explicit TrackRobotWidget( QWidget* parent = 0 );
    ~TrackRobotWidget();

    virtual const QString Name() const { return tr( "Track Robot" ); }
    virtual bool CanClose() const;
    virtual const QString CannotCloseReason() const;

    void ReloadCurrentConfigToolSpecific();

    void Playing();
    void Paused();
    void Stopped();

    void ImageUpdate( int id, const QImage& image, double fps );
    void ImageSet( int id, const QImage& image, double fps );

signals:
     void UpdateImage( int id, const QImage& image, double fps );
     void SetImage( int id, const QImage& image, double fps );

public slots:
     void SetPosition( double position );

     void SelectTrack( int id, int x, int y );
     void ClearTrack( int id );

     void ThreadPaused( bool trackingLost );
     void ThreadFinished();

private:
    void SetupUi();
    void ResetUi();
    void ConnectSignals();

    void FillOutCameraCombo( QComboBox& comboBox );

    void SetButtonIcon(QToolButton* button, QString iconImage);

    const KeyId GetRoomIdToCapture() const;
    const WbConfig GetRoomLayoutConfig(const KeyId &roomIdToCapture);
    const WbKeyValues::ValueIdPairList GetCameraPositionPairList(const KeyId& roomIdToCapture);
    const QStringList GetCameraPositionIds(const KeyId& roomIdToCapture);

    typedef QPair< QStringList, QStringList > VideoCaptureEntry;
    void AddVideoFileConfigKey(const QString& videoFileName, const KeyId& camPosId);
    void AddTableRow( const QString& roomPosition, const QStringList& videoFileNames );

    void ShowNoRoomError();
    void ShowEmptyRoomError();

    virtual const QString GetSubSchemaDefaultFileName() const;

    void CreateMappers();

    const QString GetCameraId() const;
    void PopulateCameraParams();

    const WbSchema CreateSchema();

    bool IsDataValid() const;

    bool CreateRunResultDirectory(const WbConfig& config);

    const bool CreateVideoDirectory( const QString& videoDirectoryName );

    const ExitStatus::Flags TrackLoad( const WbConfig&           trackConfig,
                                       ImageGrid*                imageGrid,
                                       RobotTracker::trackerType tracker );

    const ExitStatus::Flags TrackRun( double rate,
                                      bool trackingActive,
                                      bool singleStep,
                                      bool runForward );

    const ExitStatus::Flags TrackPause();
    const ExitStatus::Flags TrackStop();
    const ExitStatus::Flags TrackSaveData( char* floorPlanFile,
                                           char* trackerResultsTxtFile,
                                           char* trackerResultsCsvFile,
                                           char* trackerResultsImgFile,
                                           char* pixelOffsetsFile,
                                           QString trackResultsTemplate,
                                           QString pixelOffsetsTemplate );
    const ExitStatus::Flags TrackReset( ImageGrid* imageGrid );

private slots:
    void CameraComboChanged();
    void UseGlobalBtnClicked();
    void SaveBtnClicked();

    void PlayPauseButtonClicked();
    void StepButtonClicked();
    void StepBackButtonClicked();
    void ScanBackButtonClicked();
    void ScanForwardButtonClicked();
    void StopButtonClicked();

    void TrackLoadButtonClicked();
    void TrackSaveButtonClicked();
    void TrackResetButtonClicked();

private:
    Ui::TrackRobotWidget* m_ui;

    GtsScene m_scene;

    QMap< QString, VideoCaptureEntry > m_mapPositionsToFiles;

    bool m_running;  // thread running
    bool m_playing;  // video playing / paused
    bool m_tracking; // tracking
    bool m_loaded;
    bool m_fpsSet;
    double m_fps;
    double m_optimumRate;

    void SetupKeyboardShortcuts();

    std::vector<std::pair<std::string, uint>> m_scanFwdIconRatePair;
    std::vector<std::pair<std::string, uint>> m_scanBackIconRatePair;

    uint m_scanFwdIndex;
    uint m_scanBackIndex;
};

#endif // TRACKROBOTWIDGET_H
