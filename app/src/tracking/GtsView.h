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

#ifndef GTSVIEW_H
#define GTSVIEW_H

#include <string>

#include "RobotTracker.h"
#include "RobotMetrics.h"

#include "WbConfig.h"

#include <opencv/cv.h>

#include <QtGui/QImage>

#if defined(__MINGW32__) || defined(_MSC_VER)
    #include <WinTime.h>
#endif

#include <assert.h>

class CoverageSystem;
class RobotMetrics;
class CameraCalibration;
class VideoSequence;
class ImageView;
class ImageGrid;

class TrackRobotWidget;

/**
    GtsView - Ground-Truth-System View.

    This class represents a single camera ground truth system.
    It consists of camera calibration information and a video
    sequence from the camera.

    It also contains a RobotTracker which it uses to locate the
    robot in the video sequence.
**/
class GtsView
{
public:
    GtsView();
    ~GtsView();

    void Reset();

    void SetId(int id);
    bool IsSetup() const { return m_id>=0; }

    bool LoadMetrics(const WbConfig& metricsConfig,
                      const WbConfig& camPosCalConfig,
                      float trackingResolution);

    bool SetupCalibration(const KeyId     camPosId,
                           const WbConfig& cameraConfig,
                           const WbConfig& camPosConfig,
                           const WbConfig& floorPlanConfig,
                           const WbConfig& camPosCalConfig,
                           RobotMetrics&   metrics);

    bool SetupTracker(RobotTracker::trackerType type,
                       const RobotMetrics& met,
                       const char* targetFile,
                       int biLevelThreshold);

    bool SetupVideo(const char* const videoFile,
                     const char* const timestampFile,
                     float shutter,
                     float gain);

    void SetupView(TrackRobotWidget* tool, ImageGrid* imageGrid);

    void SetTrackerParam(RobotTracker::paramType param, float value);

    CvSize GetWarpImageSize() const { assert(m_imgWarp[0]); return cvSize(m_imgWarp[0]->width,
                                                                          m_imgWarp[0]->height); }

    void LoadTimestampFile(const char* const fileName);

    double GetSeekPositionInMilliseconds() const;
    bool ReadySeekFrame(double msec);
    bool ReadyNextFrame();
    const IplImage* GetNextFrame();

    const IplImage* GetCurrentImage() const { return m_imgFrame; }
    const IplImage* GetGroundPlaneImage() const { return m_imgWarp[m_imgIndex]; }

    RobotTracker& GetTracker() const { return *m_tracker; }
    RobotMetrics& GetMetrics() const { return *m_metrics; }

    void StepTracker(bool forward, CoverageSystem* coverage=0);

    const std::string& GetName() const { return m_name; }
    const std::string& GetTrackViewName() const { return m_trackView; }
    const std::string& GetAviViewName() const { return m_aviView; }

    void SaveThumbnail();
    void ShowRobotTrack();
    void HideRobotTrack();

    const CameraCalibration* GetScaledCalibration() const
    {
        return m_calScaled;
    }

    const CameraCalibration* GetNormalCalibration() const
    {
        return m_calNormal;
    }

private:
    int                   m_id;

    double                m_fps;

    CameraCalibration*    m_calScaled;
    CameraCalibration*    m_calNormal;
    RobotTracker*         m_tracker;

    std::vector<timespec> m_timestamps;
    VideoSequence*        m_sequencer;
    IplImage*             m_imgFrame;
    IplImage*             m_imgGrey;
    IplImage*             m_thumbnail;

    RobotMetrics*         m_metrics;

    ImageView*            m_viewer;

    unsigned int          m_imgIndex;
    IplImage*             m_imgWarp[2];
    IplImage*             m_imgWarp_[2];

    std::string           m_name;

	std::string           m_trackView;
    std::string           m_aviView;

    TrackRobotWidget* m_tool;

    // make uncopyable
    GtsView(const GtsView&);
    const GtsView& operator = (const GtsView&);
};

#endif // GTSVIEW_H
