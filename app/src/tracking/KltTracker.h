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

#ifndef KLTTRACKER_H
#define KLTTRACKER_H

#include "RobotTracker.h"

class CameraCalibration;
class RobotMetrics;

/**
 Implements the RobotTracker interface and
 tracks the robot target from a known starting
 position using rudimentary KLT (OpenCV's
 implementation).

 Orientation is determined by analysing the light/dark pixels
 in the tracked target template (known target must contain two
 squares diagonally offset).
 **/
class KltTracker: public RobotTracker
{
public:

    KltTracker( const CameraCalibration* cal,
                const RobotMetrics* metrics,
                const IplImage* currentImage,
                int thresh );
    ~KltTracker();

    CvPoint2D32f GetPosition() const
    {
        return m_pos;
    }

    CvPoint2D32f GetGroundPlanePos() const
    {
        return ConvertTrackToCm( AdjustTrackForRobotHeight( GetPosition(), GetHeading() ) );
    }

    float GetHeading() const
    {
        return m_angle;
    }

    CvPoint2D32f GetBrushBarLeft( CvPoint2D32f position, float heading ) const;
    CvPoint2D32f GetBrushBarRight( CvPoint2D32f position, float heading ) const;

    float GetError() const
    {
        return m_error;
    }

    const IplImage* GetCurrentImage() const
    {
        return m_currImg;
    }

    void SetParam( paramType param, float value );

    const TrackHistory::TrackLog& GetHistory() const
    {
        return m_history;
    }

    void SetPosition( CvPoint2D32f robotPosition )
    {
        m_pos = robotPosition;
    }

    void SetCurrentImage( const IplImage* const pImg )
    {
        m_prevImg = m_currImg;
        m_currImg = pImg;
    }

    void SetThresh( float thresh )
    {
        m_nccThresh = thresh;
    }

    void Activate();

    bool Track( double timeStamp )
    {
        return Track( timeStamp, true, false );
    }

    bool Track( double timestampInMillisecs, bool flipCorrect, bool init );
    void DoInactiveProcessing( double timeStamp );
    void MotionDetect();
    void LossRecovery();

    void Rewind( double timeStamp );

    bool LoadTargetImage( const char* fileName );

    const CameraCalibration* GetCalibration() const
    {
        return m_cal;
    }

    const RobotMetrics* GetMetrics() const
    {
        return m_metrics;
    }


private:
    bool HasFlipped( float angle, float oldAngle, float threshold );
    void SaveResult( const CvPoint2D32f& pos, const float angle, const float error );

    void SetLost()
    {
        m_status = TRACKER_LOST;
    }

    void SetJustLost()
    {
        m_status = TRACKER_JUST_LOST;
    }

    void TargetSearch( const IplImage* mask = 0 );

    CvPoint2D32f m_pos;
    float m_angle;

    int m_thresh1;
    //tTrackerStatus    m_status;
    float m_error;
    float m_nccThresh;
    const IplImage* m_currImg;
    const IplImage* m_prevImg;
    IplImage* m_currPyr;
    IplImage* m_prevPyr;
    CvMat* m_weightImg;
    IplImage* m_targetImg;
    IplImage* m_appearanceImg;

    static const int m_targetBackGroundGreyLevel = 128;

    IplImage* m_avgFloat; // Temporal-average (floating point)
    IplImage* m_avg; // Temporal average image
    IplImage* m_diff; // Difference image for motion detection
    IplImage* m_filtered; // filtered motion image

    // History stores the position, orientation, tracker error and time stamp.
    TrackHistory::TrackLog m_history;

    // Pointers to camera parameters (this tracker needs to know the camera calibration):
    const CameraCalibration* m_cal;
    const RobotMetrics* m_metrics;

    float ComputeHeading( CvPoint2D32f pos ) const;

    void CreateWeightImage();
    void ReleaseWeightImage();

    void AllocatePyramids();
    void ReleasePyramids();
    void SwapPyramids();

    void PredictTargetAppearance( float angleInRadians, float offsetAngleDegrees );
    void PredictTargetAppearance2( float angleInRadians, float offsetAngleDegrees, float x, float y );
    bool TrackStage2( CvPoint2D32f initialPosition, bool flipCorrect, bool init );

    void InitialiseRecoverySystem();
};

#endif // KLTTRACKER_H
