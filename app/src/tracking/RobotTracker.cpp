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

using namespace std;

const float TrackEntry::unknownWgm = -1.f;

/**
    Write a robot-track history log to file.
**/
bool WriteHistoryLog( const char* filename, const TrackHistory& history )
{
    FILE* to = fopen( filename, "w" );

    if (to)
    {
        time_t t;
        time( &t );
        struct tm *t2 = localtime( &t );
        char buf[1024];
        strftime( buf, sizeof(buf)-1, "%c", t2 );

        fprintf( to, "#Track log: %s\n# time(s)\tx(cm)\ty(cm)\theading(deg)\terror\t[wgm]\n", buf );

        for ( unsigned int i=0; i<history.size(); ++i )
        {
            const std::string* str = history[i].GetString();
            if ( str )
            {
                fprintf( to, "  %4.4f  %.3f %.3f %.3f %s\n",
                    history[i].GetTimeStamp(),
                    history[i].GetPosition().x,
                    -history[i].GetPosition().y,    // convert to right handed coords
                    history[i].GetOrientation() * R2D, // convert heading to degrees,
                    str->c_str()
                );
            }
            else
            {
                fprintf( to, "  %4.4f  %.3f %.3f %.3f %f %f\n",
                    history[i].GetTimeStamp(),
                    history[i].GetPosition().x,
                    -history[i].GetPosition().y,  // convert to right handed coords
                    history[i].GetOrientation() * R2D,
                    history[i].GetError(),
                    history[i].wgm()
                );
            }
        }

        fclose(to);

        return true;
    }
    else
    {
        LOG_ERROR(QObject::tr("Unable to write track log file: %1!")
                      .arg(filename));

        return false;
    }
}

/**
    Write a robot-track history log to CSV file.
**/
bool WriteHistoryCsv( const char* filename, const TrackHistory& history )
{
    FILE* to = fopen( filename, "w" );

    if (to)
    {
        time_t t;
        time( &t );
        struct tm *t2 = localtime( &t );
        char buf[1024];
        strftime( buf, sizeof(buf)-1, "%c", t2 );

        fprintf( to, "Time(s),X(cm),Y(cm),H(deg),Err,WGM\n");

        for ( unsigned int i=0; i<history.size(); ++i )
        {
            fprintf( to, "%4.4f,%.3f,%.3f,%.3f,%f,%f\n",
                history[i].GetTimeStamp(),
                history[i].GetPosition().x,
                -history[i].GetPosition().y,  // convert to right handed coords
                history[i].GetOrientation() * R2D,
                history[i].GetError(),
                history[i].wgm());
        }

        fclose(to);

        return true;
    }
    else
    {
        LOG_ERROR(QObject::tr("Unable to write track log file: %1!")
                     .arg(filename));

        return false;
    }
}

/**
    Read a robot-track history log from file.

    Note: when written log is converted to right handed coords,
    but when read it is not converted back!
**/
bool ReadHistoryLog( const char* filename, TrackHistory& log )
{
    FILE* fp = fopen( filename, "r" );
    char s[10000];
    int cnt;

    if (fp)
    {
        log.clear();
        while (!feof(fp))
        {
            cnt = fscanf(fp,"%s",s);
            if(cnt==1)
            {
                if(s[0]=='#') //comment?
                {
                    lineSkip(fp);
                }
                else //data
                {
                   float t,x,y,th,e;

                    cnt += sscanf(s,"%f",&t);
                    cnt += fscanf( fp, "%f %f %f", &x, &y, &th);
                    th  *= F_D2R;
                    cnt += fscanf(fp,"%s",s);
                    if ( cnt != 6 )
                    {
                        LOG_ERROR(QObject::tr("Unexpected data in log %1!").arg(filename));
                    }
                    int number = sscanf(s,"%f",&e);
                    if ( number ) //the last one is the tracker error
                    {
                        log.push_back( TrackEntry( cvPoint2D32f(x,y), th, e, t, 0.f ) );
                    }
                    else //the last one was a image
                    {
                        string str(s);
                        log.push_back( TrackEntry( cvPoint2D32f(x,y), th, 0, t, str ) );
                    }
                    lineSkip(fp);
                }//if comment
            }//if cnt
        }//while


    }
    else
    {
        LOG_ERROR(QObject::tr("Unable to read track log file: %1!")
                     .arg(filename));

        return false;
    }

    return true;
}

/**
    Interpolates bewteen two track entries.
    Every entry is linearly interpolated using
    the weighting value w which should be between
    0 and 1.

    If w==0 then (a copy of) a is returned (effectively).
    If w==1 then (a copy of) b is returned (effectively).

    The string data member is ignored (and hence not present in the interpolated result).

    :NOTE: Modification on 30/05/2008
    Averaging for angles is modified so that we don't average
    tracks which are 180 degrees opposed (doing so would mess up
    coverage computations when using the IR tracker - because
    it can't distiguish one blob from another and therefore
    (potentially) has a 180 degree ambiguity between any two tracks).
**/
TrackEntry Interpolate( TrackEntry a, TrackEntry b, float w )
{
    CvPoint2D32f pa = a.GetPosition(); // earlier position
    CvPoint2D32f pb = b.GetPosition(); // later position
    CvPoint2D32f pi; // interpolated position
    pi.x = pa.x + (pb.x-pa.x)*w;
    pi.y = pa.y + (pb.y-pa.y)*w;

    // Check that we dont interpolate over too large an angle
    float angle = a.GetOrientation();
    float diff = diff_angle( angle, b.GetOrientation() );
    if (diff > 3.f)
    {
        angle -= F_PI;
    }
    if (diff < -3.f)
    {
        angle += F_PI;
    }
    angle = norm_angle( angle );

    // interpolate angle, error, and warp-gradient-magnitude (wgm)
    float ai = (float)interpolate( 0.0, angle, 1.0, b.GetOrientation(), w, true);
    float ei = a.GetError() + (b.GetError()-a.GetError())*w;
    float wgmi = a.wgm()   + (b.wgm()-a.wgm())*w;

    double ti = a.m_timeStamp + ( b.m_timeStamp - a.m_timeStamp )*w;

    return TrackEntry( pi, ai, ei, ti, wgmi );
}

//------------------------------------------------------------------------------------------
////    Members of RobotTracker
//------------------------------------------------------------------------------------------

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
CvPoint2D32f RobotTracker::AdjustTrackForRobotHeight( CvPoint2D32f pos, float heading ) const
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
    float cosa = cos( -heading );
    float sina = sin( -heading );
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
CvPoint2D32f RobotTracker::ConvertTrackToCm( CvPoint2D32f p ) const
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
CvPoint2D32f RobotTracker::ConvertTrackToPx( CvPoint2D32f p ) const
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
void RobotTracker::ConvertLogForProcessing( TrackHistory& newlog, bool relative ) const
{
    const TrackHistory& history = GetHistory();

    if ( history.size() > 0 )
    {
        newlog.resize( history.size() );
        CvPoint2D32f pStart = {0,0};
        float oStart        = 0;
        float cosa = 1.f;
        float sina = 0.f;

        if ( relative )
        {
            oStart = history[0].GetOrientation();
            pStart = ConvertTrackToCm( AdjustTrackForRobotHeight( history[0].GetPosition(), oStart ) );

            // compute rotation to undo initial orientation
            cosa = cosf( (F_PI*.5f)-oStart );
            sina = sinf( (F_PI*.5f)-oStart );
        }

        for    ( unsigned int i=0; i<history.size(); ++i )
        {
            {
                CvPoint2D32f p = AdjustTrackForRobotHeight( history[i].GetPosition(), history[i].GetOrientation() );
                p = ConvertTrackToCm( p );

                // subtract origin (will be zero so have no effect if relative is false)
                float x =  p.x - pStart.x;
                float y =  p.y - pStart.y;

                // rotate data (will be identity so have no effect if relative is false)
                p.x = (cosa*x) - (sina*y);
                p.y = (sina*x) + (cosa*y);

                float o = history[i].GetOrientation() - oStart;
                float t = history[i].GetTimeStamp()/1000.0;

                // save conversion into the new log
                newlog[i] = TrackEntry( p, o, history[i].GetError(), t, history[i].wgm() );
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
void RobotTracker::WriteTrackData( const char* trackerOutput, bool relative ) const
{
    TrackHistory history;

    ConvertLogForProcessing( history, relative );

    if ( history.size() == 0 )
    {
        LOG_WARN("Track history is empty - no log file written.");

        return;
    }

    WriteHistoryLog( trackerOutput, history );
}

bool RobotTracker::IsActive() const
{
    return m_status == TRACKER_ACTIVE;
}

bool RobotTracker::IsLost() const
{
    return ( m_status == TRACKER_LOST || m_status == TRACKER_JUST_LOST );
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
