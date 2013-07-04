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

#ifndef GTS_VIEW_H
#define GTS_VIEW_H

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

class TrackRobotToolWidget;

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

    void SetId( int id );
    bool IsSetup() const { return m_id>=0; }

    //bool LoadMetrics( const char* filename );

    bool SetupCalibration( const KeyId     camPosId,
                           const WbConfig& cameraCfg,
                           const WbConfig& camPosCfg,
                           const WbConfig& floorPlanCfg,
                           CvSize          boardSize,
                           RobotMetrics&   met );

    bool SetupTracker( RobotTracker::trackerType type, const RobotMetrics& met, const char* targetFile, int thresh );
    bool SetupVideo( const char* const videoFile, const char* const timestampFile, float shutter, float gain );
    void SetTrackerParam( RobotTracker::paramType param, float nccThresh );
    void SetupView( TrackRobotToolWidget* tool, ImageGrid* imageGrid );

    CvSize GetWarpImageSize() const { assert(m_imgWarp[0]); return cvSize(m_imgWarp[0]->width,m_imgWarp[0]->height); }

    void LoadTimestampFile( const char* const fileName );

    const IplImage* GetNextFrame();
    const IplImage* GetNextFrame( double msec );

    const IplImage* GetCurrentImage() const { return m_imgFrame; }
    const IplImage* GetGroundPlaneImage() const { return m_imgWarp[m_imgIndex]; }

    RobotTracker* GetTracker() const { return m_tracker; }
    //RobotMetrics* GetMetrics() const { return m_metrics; }

    void StepTracker( bool forward, CoverageSystem* coverage=0 );

    const std::string& GetName() const { return m_name; }
    const std::string& GetTrackViewName() const { return m_trackView; }
    const std::string& GetAviViewName() const { return m_aviView; }

    void SaveThumbnail();

    void ShowRobotTrack();

    const CameraCalibration* GetScaledCalibration() const
    {
        return m_calScaled;
    }

    const CameraCalibration* GetNormalCalibration() const
    {
        return m_calNormal;
    }

private:
    int m_id;

    double m_fps;

    CameraCalibration*   m_calScaled;
    CameraCalibration*   m_calNormal;
    RobotTracker*        m_tracker;

    std::vector<timespec> m_timestamps;
    VideoSequence*       m_sequencer;
    IplImage*            m_imgFrame;
    IplImage*            m_imgGrey;
    IplImage*            m_thumbnail;

    ImageView*           m_viewer;

    unsigned int         m_imgIndex;
    IplImage*            m_imgWarp[2];
    IplImage*            m_imgWarp_[2];

    std::string m_name;

	std::string m_trackView;
    std::string m_aviView;

    TrackRobotToolWidget* m_tool;

    // make uncopyable
    GtsView(const GtsView&);
    const GtsView& operator = (const GtsView&);
};

#endif // GTS_VIEW_H
