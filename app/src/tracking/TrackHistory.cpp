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

#include "TrackHistory.h"

#include "Angles.h"
#include "FileUtilities.h"
#include "MathsConstants.h"
#include "Logging.h"

#include <QObject>

#include <string>

const float TrackEntry::unknownWgm = 0.f;

namespace TrackHistory
{
    /**
        Write a robot-track history log to file.
    **/
    bool WriteHistoryLog( const char* filename, const TrackLog& history )
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
                        history[i].GetOrientation() * MathsConstants::R2D, // convert heading to degrees,
                        str->c_str() );
                }
                else
                {
                    fprintf( to, "  %4.4f  %.3f %.3f %.3f %f %f\n",
                        history[i].GetTimeStamp(),
                        history[i].GetPosition().x,
                        -history[i].GetPosition().y,  // convert to right handed coords
                        history[i].GetOrientation() * MathsConstants::R2D,
                        history[i].GetError(),
                        history[i].wgm() );
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
    bool WriteHistoryCsv( const char* filename, const TrackLog& history )
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
                    history[i].GetOrientation() * MathsConstants::R2D,
                    history[i].GetError(),
                    history[i].wgm() );
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
    bool ReadHistoryLog( const char* filename, TrackLog& log )
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
                        FileUtilities::LineSkip(fp);
                    }
                    else //data
                    {
                       float t,x,y,th,e;

                        cnt += sscanf(s,"%f",&t);
                        cnt += fscanf( fp, "%f %f %f", &x, &y, &th);
                        th  *= MathsConstants::F_D2R;
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
                            std::string str(s);
                            log.push_back( TrackEntry( cvPoint2D32f(x,y), th, 0, t, str ) );
                        }

                        FileUtilities::LineSkip(fp);
                    }
                }
            }
        }
        else
        {
            LOG_ERROR(QObject::tr("Unable to read track log file: %1!")
                         .arg(filename));

            return false;
        }

        return true;
    }

    bool ReadHistoryCsv( const char* filename, TrackLog& log )
    {
        FILE* fp = fopen( filename, "r" );
        int cnt;

        if (fp)
        {
            log.clear();

            // Skip headers
            FileUtilities::LineSkip(fp);

            while (!feof(fp))
            {
                float t,x,y,th,e,w;

                cnt = fscanf( fp, "%f,%f,%f,%f,%f,%f\n", &t, &x, &y, &th, &e, &w );
                if ( cnt != 6 )
                {
                    LOG_ERROR(QObject::tr("Unexpected data (%1) in log %2!").arg(cnt).arg(filename));
                }

                log.push_back( TrackEntry( cvPoint2D32f(x,y), th*MathsConstants::F_D2R, e, t, w ) );
            }

            fclose(fp);

            return true;
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
    TrackEntry InterpolateEntries( TrackEntry a, TrackEntry b, float w )
    {
        CvPoint2D32f pa = a.GetPosition(); // earlier position
        CvPoint2D32f pb = b.GetPosition(); // later position
        CvPoint2D32f pi; // interpolated position

        pi.x = pa.x + (pb.x-pa.x)*w;
        pi.y = pa.y + (pb.y-pa.y)*w;

        // Check that we dont interpolate over too large an angle
        float angle = a.GetOrientation();

        float diff = Angles::DiffAngle( angle, b.GetOrientation() );

        if (diff > 3.f)
        {
            angle -= MathsConstants::F_PI;
        }

        if (diff < -3.f)
        {
            angle += MathsConstants::F_PI;
        }

        angle = Angles::NormAngle( angle );

        // interpolate angle, error, and warp-gradient-magnitude (wgm)
        float ai = (float)Angles::Interpolate( 0.0, angle, 1.0, b.GetOrientation(), w, true);
        float ei = a.GetError() + (b.GetError()-a.GetError())*w;
        float wgmi = a.wgm()   + (b.wgm()-a.wgm())*w;

        double ti = a.GetTimeStamp() + ( b.GetTimeStamp() - a.GetTimeStamp() )*w;

        return TrackEntry( pi, ai, ei, ti, wgmi );
    }
}
