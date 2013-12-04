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

#include "RobotTracker.h"

#include "OpenCvUtility.h"

#include "Angles.h"
#include "MathsConstants.h"
#include "ScanMatch.h"

#include "CameraCalibration.h"
#include "RobotMetrics.h"

#include "Logging.h"

#include <stdio.h>
#include <time.h>
#include <iostream>

/**
    Converts coordinates so that they represent the x,y coordinates
    of the robots centre at its base in the ground plane (we track
    the centre of the target on top of the robot but this does not
    lie in the ground plane so we must compensate for this).

    For height compensation calculation see MP's Notebook, book 1, 23/08/2007.

    NOTE 1:
    When I corrected the mirroring of the ground plane image I did it by reversing the coordinates
    of the calibration target. This changes the handedness of the calibration coordinate system hence
    cz (camera height) now becomes negative. This is why cz is negative in the scale calculation
    (where as in my notebook it is positive).
**/
CvPoint2D32f RobotTracker::AdjustTrackForRobotHeight(CvPoint2D32f pos, float heading) const
{
    // Offset takes us to calibration-plane coords (in pixels)
    pos.x += GetOffsetParams()->x;
    pos.y += GetOffsetParams()->y;

    // get the camera centre (IN PIXELS)
    const CvMat* C = GetCalibration()->GetCameraCentrePx();
    float cx = C->data.fl[0];
    float cy = C->data.fl[1];
    float cz = C->data.fl[2];

    // now make the adjustment
    float scale = (GetMetrics()->GetHeightPx()/-cz)-1.f; // See NOTE 1
    pos.x = cx + scale*(cx - pos.x);
    pos.y = cy + scale*(cy - pos.y);

    // put it back into image coords
    pos.x -= GetOffsetParams()->x;
    pos.y -= GetOffsetParams()->y;

    // NEW: need to adjust for off centre robot target aswell!
    float ox = GetMetrics()->GetXTargetOffsetPx();
    float oy = GetMetrics()->GetYTargetOffsetPx();

    // rotate offset using robots orientation and SUBTRACT from position
    float cosa = cos(-heading);
    float sina = sin(-heading);
    pos.x -= ox*cosa - oy*sina;
    pos.y -= ox*sina + oy*cosa;

    return pos;
}

/**
    Returns the 2D offset from tracker coords to calibration plane coords
**/
const CvPoint2D32f* RobotTracker::GetOffsetParams() const
{
    return GetCalibration()->GetUnwarpOffset();
}

/**
    Shift and scale track position (which is in pixels)
    so it is in centi-metres relative to the origin of
    the calibration plane.
**/
CvPoint2D32f RobotTracker::ConvertTrackToCm(CvPoint2D32f p) const
{
    p.x += GetOffsetParams()->x;   // shift to calibration-plane coords (in pixels)
    p.y += GetOffsetParams()->y;

    p.x /= GetMetrics()->GetScaleFactor(); // then divide by scale factor (pixels per cm)
    p.y /= GetMetrics()->GetScaleFactor();

    return p;
}

/**
    Scale and shift a track from centimetres to image coords in pixels.
**/
CvPoint2D32f RobotTracker::ConvertTrackToPx(CvPoint2D32f p) const
{
    p.x *= GetMetrics()->GetScaleFactor(); // multiply by scale factor (pixels per cm)
    p.y *= GetMetrics()->GetScaleFactor();

    p.x -= GetOffsetParams()->x;   // shift to image coords (in pixels)
    p.y -= GetOffsetParams()->y;

    return p;
}

/**
    Convert the stored robot log to cm in the ground plane coordinate system
    (adjusting for robot height). Also converts the timestamps from milliseconds
    to seconds.

    Optionally converts the log so it is a relative log (if relative==true).
**/
void RobotTracker::ConvertLogForProcessing(TrackHistory::TrackLog& newlog, bool relative) const
{
    const TrackHistory::TrackLog& history = GetHistory();

    if (history.size() > 0)
    {
        newlog.resize(history.size());
        CvPoint2D32f pStart = {0,0};
        float oStart        = 0;
        float cosa = 1.f;
        float sina = 0.f;

        if (relative)
        {
            oStart = history[0].GetOrientation();
            pStart = ConvertTrackToCm(AdjustTrackForRobotHeight(history[0].GetPosition(), oStart));

            // compute rotation to undo initial orientation
            cosa = cosf((MathsConstants::F_PI*.5f)-oStart);
            sina = sinf((MathsConstants::F_PI*.5f)-oStart);
        }

        for (unsigned int i=0; i<history.size(); ++i)
        {
            {
                CvPoint2D32f p = AdjustTrackForRobotHeight(history[i].GetPosition(),
                                                            history[i].GetOrientation());
                p = ConvertTrackToCm(p);

                // subtract origin (will be zero so have no effect if relative is false)
                float x =  p.x - pStart.x;
                float y =  p.y - pStart.y;

                // rotate data (will be identity so have no effect if relative is false)
                p.x = (cosa*x) - (sina*y);
                p.y = (sina*x) + (cosa*y);

                float o = history[i].GetOrientation() - oStart;
                double t = history[i].GetTimeStamp()/1000.0; // convert timestamp from millisecs to secs

                // save conversion into the new log
                newlog[i] = TrackEntry(p, o, history[i].GetError(), t, history[i].wgm());
            }
        }
    }
}

/**
    Writes the tracker's history record
    (timestamp, position, heading, tracker error, for each frame)
    to a log file.

    Can write an absolute or relative log. A relative log will be comparable
    with the robot's own odometry log; that is it starts at position 0,0, with
    initial orientation considered to be directly down the +ve y-axis.
**/
void RobotTracker::WriteTrackData(const char* trackerOutput, bool relative) const
{
    TrackHistory::TrackLog history;

    ConvertLogForProcessing(history, relative);

    if (history.size() == 0)
    {
        LOG_WARN("Track history is empty - no log file written.");

        return;
    }

    TrackHistory::WriteHistoryLog(trackerOutput, history);
}

bool RobotTracker::IsActive() const
{
    return m_status == TRACKER_ACTIVE;
}

bool RobotTracker::IsLost() const
{
    return (m_status == TRACKER_LOST || m_status == TRACKER_JUST_LOST);
}

bool RobotTracker::WasJustLost() const
{
    return m_status == TRACKER_JUST_LOST;
}

void RobotTracker::Activate()
{
    m_status = TRACKER_ACTIVE;
}

void RobotTracker::Deactivate()
{
    m_status = TRACKER_INACTIVE;
}
