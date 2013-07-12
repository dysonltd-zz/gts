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

#ifndef ROBOTTRACKER_H
#define ROBOTTRACKER_H

#include "TrackHistory.h"

#include <opencv/cv.h>

class RobotMetrics;
class CameraCalibration;

/**
    This abstract class specifies an interface for visually tracking the robot
    with an external camera.

    Primarily used for the robot ground-truth tracking system, but could be
    used as a (single object) tracker for other tasks.
**/
class RobotTracker
{
public:
    enum trackerType
    {
        KLT_TRACKER = 0
    };

    enum paramType
    {
        PARAM_NCC_THRESHOLD = 0
    };

    enum trackerStatus
    {
        TRACKER_INACTIVE = 0,
        TRACKER_ACTIVE,
        TRACKER_LOST,
        TRACKER_JUST_LOST // Tracker has just transitioned into lost state on previous frame
    };

    RobotTracker() : m_status(TRACKER_INACTIVE) {}
    virtual ~RobotTracker() {}

    virtual CvPoint2D32f GetPosition() const = 0;
    virtual float GetHeading()  const = 0;
    virtual CvPoint2D32f GetBrushBarLeft( CvPoint2D32f position, float heading ) const = 0;
    virtual CvPoint2D32f GetBrushBarRight( CvPoint2D32f position, float heading ) const = 0;

    virtual void SetPosition( CvPoint2D32f robotPosition ) = 0;

    virtual void SetCurrentImage(const IplImage* const pImg) = 0;
    virtual const IplImage* GetCurrentImage() const = 0;
    virtual bool Track(double timeStamp) = 0;
    virtual void Rewind( double timeStamp ) = 0;

    virtual bool LoadTargetImage( const char* fileName ) = 0;

    virtual float GetError() const = 0;
    virtual const TrackHistory::TrackLog& GetHistory() const = 0;

    virtual CvPoint2D32f GetGroundPlanePos() const = 0;

    virtual bool IsActive() const;
    virtual bool IsLost() const;
    virtual bool WasJustLost() const;
    virtual void Activate();
    virtual void Deactivate();
    virtual void DoInactiveProcessing(double timeStamp) = 0;
    virtual void LossRecovery() {}

    virtual const CameraCalibration* GetCalibration()    const = 0;
    virtual const RobotMetrics*      GetMetrics()        const = 0;
    virtual const CvPoint2D32f*      GetOffsetParams()    const;

    virtual void ConvertLogForProcessing( TrackHistory::TrackLog& newlog, bool relative ) const;
    virtual void WriteTrackData(const char* fileName, bool relative=false) const;

    virtual CvPoint2D32f AdjustTrackForRobotHeight(CvPoint2D32f pos, float heading) const;

    virtual void SetParam( paramType param, float value ) = 0;

    CvPoint2D32f ConvertTrackToCm( CvPoint2D32f pos ) const;
    CvPoint2D32f ConvertTrackToPx( CvPoint2D32f p ) const;

protected:
    trackerStatus m_status; // status is recorded and managed in base class

private:
    RobotTracker( RobotTracker& rt ); // Trackers are deliberately uncopyable!
};

#endif // ROBOTTRACKER_H
