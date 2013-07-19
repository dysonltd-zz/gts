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

#ifndef TRACKROBOTTOOLWIDGET_H
#define TRACKROBOTTOOLWIDGET_H

#include <QtGui/QWidget>
#include <QtGui/QTableWidget>
#include <QToolButton>
#include <QPair>
#include <QMap>

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
    class TrackRobotToolWidget;
}

class TrackRobotToolWidget : public Tool
{
    Q_OBJECT

public:
    explicit TrackRobotToolWidget( QWidget* parent = 0 );
    ~TrackRobotToolWidget();

    virtual const QString Name() const { return tr( "Track Robot" ); }

    void ReloadCurrentConfigToolSpecific();

    void Playing();
    void Paused();
    void Stopped();

    void UpdatePosition( long position );
    void ImageUpdate( int id, const QImage& image, double fps );
    void ImageSet( int id, const QImage& image, double fps );

signals:
     void UpdateImage( int id, const QImage& image, double fps );
     void SetImage( int id, const QImage& image, double fps );

public slots:
     void ViewClicked( int id, int x, int y );

     void ThreadPaused( bool trackingLost );
     void ThreadFinished();

     void VideoPosition( double position );

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

    typedef QPair< QStringList, QString > VideoCaptureEntry;
    void AddVideoFileConfigKey(const QString& videoFileName, const KeyId& camPosId);
    void AddTableRow( const QString& roomPosition, const QStringList& videoFileName );

    void ShowNoRoomError();
    void ShowEmptyRoomError();
    void ShowNullCameraPosError();
    void ShowMissingCameraError(const QString& cameraPosDisplayName);

    virtual const QString GetSubSchemaDefaultFileName() const;

    void CreateMappers();

    const QString GetCameraId() const;
    void PopulateCameraParams();

    const WbSchema CreateSchema();

    bool CreateRunResultDirectory(const WbConfig& config);

    const bool CreateVideoDirectory( const QString& videoDirectoryName );

    const ExitStatus::Flags TrackLoad( const WbConfig&           trackConfig,
                                       ImageGrid*                imageGrid,
                                       RobotTracker::trackerType tracker );

    const ExitStatus::Flags TrackRun( double rate, bool trackingActive,
                                                   bool singleStep,
                                                   bool runForward );
    const ExitStatus::Flags TrackPause();
    const ExitStatus::Flags TrackStop();
    const ExitStatus::Flags TrackPostProcess( char* floorPlanFile,
                                              char* trackerResultsTxtFile,
                                              char* trackerResultsCsvFile,
                                              char* trackerResultsImgFile,
                                              char* pixelOffsetsFile );
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
    Ui::TrackRobotToolWidget* m_ui;

    GtsScene m_scene;

    QMap< QString, VideoCaptureEntry > m_mapPositionsToFiles;

    bool m_running;  // thread running
    bool m_playing;  // video playing / paused
    bool m_tracking; // tracking
    bool m_loaded;
    bool m_fpsSet;
    double m_fps;

    double m_optimumRate;

    std::vector<std::pair<std::string, uint>> m_scanFwdIconRatePair;
    std::vector<std::pair<std::string, uint>> m_scanBackIconRatePair;

    uint m_scanFwdIndex;
    uint m_scanBackIndex;

};

#endif // TRACKROBOTTOOLWIDGET_H
