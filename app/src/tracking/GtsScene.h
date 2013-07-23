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

#ifndef GTSSCENE_H
#define GTSSCENE_H

#include <QFile>

#include "GtsView.h"
#include "RobotTracker.h"
#include "RobotMetrics.h"
#include "CoverageSystem.h"
#include "TrackThread.h"
#include "WbConfig.h"

#include <string>

#define GTS_MAX_CAMERAS 8

class ImageGrid;

class TrackRobotToolWidget;

/**
    GtsScene: Ground-Truth-System Scene.

    Class for multi-camera ground truth system.
    Each camera is managed thru a TrackingView.
**/
class GtsScene
{
public:
    GtsScene();
    ~GtsScene();

    void Reset();

    bool LoadTarget( const WbConfig& targetCfg );

    bool LoadCameraConfig( const KeyId               camPosId,
                           const char* const         selectedVideoFileName,
                           const char* const         timestampFileName,
                           const WbConfig&           cameraConfig,
                           const WbConfig&           camPosConfig,
                           const WbConfig&           roomConfig,
                           const WbConfig&           robotConfig,
                           const WbConfig&           trackConfig,
                           RobotTracker::trackerType tracker );

    unsigned int GetNumMaxCameras() const { return GtsScene::kMaxCameras; }

    void SetupViewWindows( TrackRobotToolWidget* tool, ImageGrid* imageGrid );
    void DestroyViewWindows( ImageGrid* imageGrid );

    struct TrackStatus
    {
        double videoPosition;

        size_t numTrackersLost;
        size_t numTrackersActive;

        bool eof;
    };

    TrackStatus StepTrackers( bool forward );

    void SetupThread( TrackRobotToolWidget* tool );
    void StartThread( double rate, bool trackingActive = true,
                                   bool singleStep = false,
                                   bool runForward = true );
    void PauseThread();
    void StopThread();

    void SetRate( double rate );

    void PostProcess( char* floorPlanFile,
                      char* trackerResultsTxtFile,
                      char* trackerResultsCsvFile,
                      char* trackerResultsImgFile,
                      char* pixelOffsetsFile );

    void SetTrackPosition( int id, int x, int y );
    void ClrTrackPosition( int id );

private:
    int OrganiseLogs( TrackHistory::TrackLog* log );

    void PostProcessMultiCamera( TrackHistory::TrackLog& avg,
                                 CvPoint2D32f&           offset,
                                 IplImage**              compImgCol,
                                 float                   timeThresh,
                                 char*                   floorPlanName,
                                 unsigned int            baseIndex );

    static const unsigned int kMaxCameras = GTS_MAX_CAMERAS;

    TrackThread* m_thread;

    GtsView m_view[GtsScene::kMaxCameras];

    QString m_targetFile;

    // post processing variables
    double m_fps;
    unsigned int m_nPairs;
    int m_matchPairs[GtsScene::kMaxCameras][2];
    TrackHistory::TrackLog m_log[GtsScene::kMaxCameras];
    TrackHistory::TrackLog m_logPx[GtsScene::kMaxCameras];
    const IplImage* m_gpImg[GtsScene::kMaxCameras];
    CvPoint2D32f m_origin[GtsScene::kMaxCameras];

    // time offset for each log
    float m_dt[GtsScene::kMaxCameras];

    double m_msec;
    double m_rate;

    unsigned int m_ln;
};

#undef GTS_MAX_CAMERAS

#endif // GTSSCENE_H
