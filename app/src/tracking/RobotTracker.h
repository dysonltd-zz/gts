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

#ifndef ROBOT_TRACKER_H
#define ROBOT_TRACKER_H

#include <opencv/cv.h>

#include <string>
#include <vector>

class TrackEntry {
    friend TrackEntry Interpolate( TrackEntry a, TrackEntry b, float w );

public:
    TrackEntry() :
      m_robotPosition        (cvPoint2D32f(0,0)),
      m_robotAngle    (0.f),
      m_nccError    (0.f),
      m_timeStamp    (0.0),
      m_additionalInfo     (NULL),
      m_warpGradientMagnitude        (0)
        {};

      TrackEntry( CvPoint2D32f pos, float angle, float error, double time, float wgm ) :
      m_robotPosition        (pos),
      m_robotAngle    (angle),
      m_nccError    (error),
      m_timeStamp    (time),
      m_additionalInfo     (NULL),
      m_warpGradientMagnitude        (wgm)
        {};

      TrackEntry( CvPoint2D32f pos, float angle, float error, double time, std::string &str ) :
      m_robotPosition        (pos),
      m_robotAngle    (angle),
      m_nccError    (error),
      m_timeStamp    (time),
      m_additionalInfo     (NULL),    //has to be here for the check to work
      m_warpGradientMagnitude        (0)
        {
            CopyStr(&str);
        };

    TrackEntry(const TrackEntry& te) :
      m_robotPosition(te.m_robotPosition),
      m_robotAngle(te.m_robotAngle),
      m_nccError(te.m_nccError),
      m_timeStamp(te.m_timeStamp),
      m_additionalInfo(NULL),
      m_warpGradientMagnitude(te.m_warpGradientMagnitude)
        {
            CopyStr(te.m_additionalInfo);
        };

    const TrackEntry& operator = (const TrackEntry& te)
    {
        m_robotPosition   = te.m_robotPosition;
        m_robotAngle = te.m_robotAngle;
        m_nccError = te.m_nccError;
        m_timeStamp  = te.m_timeStamp;
        CopyStr(te.m_additionalInfo);
        m_warpGradientMagnitude = te.m_warpGradientMagnitude;
        return *this;
    };

    ~TrackEntry()
    {
        delete m_additionalInfo;
    }

    static const float unknownWgm;

    float  x() const    { return m_robotPosition.x; };
    float  y() const    { return m_robotPosition.y; };
    double t() const    { return m_timeStamp; };
    float  th() const   { return m_robotAngle;};
    float  e() const    { return m_nccError; };
    std::string*  str() { return m_additionalInfo;};
    float wgm() const   { return m_warpGradientMagnitude;};

    bool operator == (const TrackEntry& te) const { return m_timeStamp == te.m_timeStamp; };
    bool operator != (const TrackEntry& te) const { return m_timeStamp != te.m_timeStamp; };
    bool operator <  (const TrackEntry& te) const  { return m_timeStamp < te.m_timeStamp; };
    bool operator >  (const TrackEntry& te) const  { return m_timeStamp > te.m_timeStamp; };
    bool operator <= (const TrackEntry& te) const  { return m_timeStamp <= te.m_timeStamp; };
    bool operator >= (const TrackEntry& te) const  { return m_timeStamp >= te.m_timeStamp; };

    CvPoint2D32f  GetPosition()     const { return m_robotPosition;   };
    const float   GetOrientation()  const { return m_robotAngle; };
    const float   GetError()        const { return m_nccError; };
    const double  GetTimeStamp()    const { return m_timeStamp;  };
    const std::string* GetString()  const { return m_additionalInfo;   };

    float GetWeighting() const { return m_warpGradientMagnitude*m_nccError; }; ///< Return the weight to be used in averaging.

    void SetPosition( CvPoint2D32f v ) { m_robotPosition = v; };
    void SetOrientation( float v )     { m_robotAngle = v; };
    void SetNccError( float v )        { m_nccError = v; };
    void SetTimeStamp( double v )      { m_timeStamp = v;  };
    void SetWarpGradMag( float v )     { m_warpGradientMagnitude = v; };

    //static TrackEntry Break() { return TrackEntry( cvPoint2D32f(0.f,0.f),-1000.f,0.f,-10000.0 ); };
    //bool IsBreak() const { return (m_angle < -900.f); };

private:
    CvPoint2D32f m_robotPosition;         ///< Robot position
    float        m_robotAngle;            ///< Angle of robot
    float        m_nccError;              ///< indicate tracking quality
    double       m_timeStamp;             ///< Time-stamp
    std::string* m_additionalInfo;        ///< additional information - e.g. img name
    float        m_warpGradientMagnitude; ///< warp gradient magnitude at tracked point

    inline void CopyStr(const std::string* str)
    {
        if ( str )
        {
            if(!m_additionalInfo)
                m_additionalInfo = new std::string(*str);
            else
                *m_additionalInfo = *str;
        }
        else
        {
            delete m_additionalInfo;
            m_additionalInfo = 0;
        }
    }; //copy_str

};

typedef std::vector<TrackEntry> TrackHistory;

bool WriteHistoryLog( const char* filename, const TrackHistory& hist );
bool WriteHistoryCsv( const char* filename, const TrackHistory& hist );

bool ReadHistoryLog( const char* filename, TrackHistory& hist );

// fwd decl
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
    virtual const TrackHistory& GetHistory() const = 0;

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

    virtual void ConvertLogForProcessing( TrackHistory& newlog, bool relative ) const;
    virtual void WriteTrackData(const char* fileName, bool relative=false) const;

    virtual CvPoint2D32f AdjustTrackForRobotHeight(CvPoint2D32f pos, float heading) const;

    virtual void SetParam( paramType param, float value ) = 0;

    CvPoint2D32f ConvertTrackToCm( CvPoint2D32f pos ) const;
    CvPoint2D32f ConvertTrackToPx( CvPoint2D32f p ) const;

protected:
    trackerStatus m_status; // status is recorded and managed in base class
private:

    RobotTracker(RobotTracker& rt); // Trackers are deliberately uncopyable!
};

#endif // ROBOT_TRACKER_H
