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

#include "ScanUtility.h"

#include "Angles.h"
#include "MathsConstants.h"

#include "CameraCalibration.h"
#include "OpenCvUtility.h"

#include <opencv/highgui.h>

namespace ScanUtility
{
    CvPoint2D32f Convert( CvPoint2D32f pSrc, const CvMat* H )
    {
        CvPoint2D32f pDst;
        CvScalar p1;
        CvMat tsrc, tdst;
        CvMat* src = cvCreateMat( 2, 1, CV_32F );
        CvMat* dst = cvCreateMat( 2, 1, CV_32F );

        cvSet2D(src,0,0,cvScalar(pSrc.x));
        cvSet2D(src,1,0,cvScalar(pSrc.y));
        cvReshape( src, &tsrc, 2, 0 );
        cvReshape( dst, &tdst, 2, 0 );

#ifdef AFFINE_TRANSFORM
        float A[6];
        CvMat T = cvMat( 2, 3, CV_32F, A );

        A[0] = cvmGet(H,0,0);
        A[1] = cvmGet(H,0,1);
        A[2] = cvmGet(H,0,2);
        A[3] = cvmGet(H,1,0);
        A[4] = cvmGet(H,1,1);
        A[5] = cvmGet(H,1,2);

        cvTransform(&tsrc, &tdst, &T);
#else
        cvPerspectiveTransform( &tsrc, &tdst, H );
#endif

        p1=cvGet2D(&tdst,0,0);
        pDst.x=p1.val[0];
        pDst.y=p1.val[1];

        cvReleaseMat( &src );
        cvReleaseMat( &dst );

        return pDst;
    }

    /**
	    Plots robot track into image.
	    Image must be allocated to the correct size.
    **/
    void PlotLog( const TrackHistory::TrackLog& log,
                  IplImage*                     img,
                  CvScalar                      col,
                  CvRect                        rect,
                  int                           margin,
                  int                           scale,
                  double                        timeThresh )
    {
	    if ( log.size() == 0 ) return;

	    int xoffset = margin-scale*rect.x;
	    int yoffset = margin-scale*rect.y;

	    CvPoint old = cvPoint( scale*log[0].x()+xoffset,
                               scale*log[0].y()+yoffset );

	    for ( unsigned int i=1; i<log.size(); ++i )
	    {
		    double tdiff = fabs( log[i].t() - log[i-1].t() );
            if ( tdiff > timeThresh )
		    {
			    old = cvPoint( scale*log[i].x()+xoffset,
                               scale*log[i].y()+yoffset );
		    }
		    else
		    {
			    CvPoint cur = cvPoint( scale*log[i].x()+xoffset,
                                       scale*log[i].y()+yoffset );

			    cvLine( img, old, cur, col, 1, CV_AA );
			    old = cur;
		    }
	    }
    }

    /**
	    Plots robot track into image.
	    Image must be allocated to the correct size.
    **/
    void PlotPoint( const TrackEntry& entry,
                    IplImage*   img,
                    CvScalar    col,
                    CvRect      rect,
                    int         margin,
                    int         scale )
    {
	    int xoffset = margin-scale*rect.x;
	    int yoffset = margin-scale*rect.y;

	    CvPoint pos = cvPoint( scale*entry.x()+xoffset,
                               scale*entry.y()+yoffset );

        const int halfWidth = 3;
        cvRectangle(img, cvPoint(pos.x - halfWidth, pos.y - halfWidth), cvPoint(pos.x + halfWidth, pos.y + halfWidth), col, 1, CV_AA);
    }

    /**
	    Transform a robot position log using result of scan matching.
    **/
    void TransformLog( const TrackHistory::TrackLog& log, TrackHistory::TrackLog& newlog, const CvMat* H )
    {
	    newlog.clear();
	    newlog.resize( log.size() );

	    for ( unsigned int i=0; i<log.size(); ++i )
	    {
		    {
		        CvPoint2D32f pDst = Convert( log[i].GetPosition(), H );

                // cos(angle)  sin(angle) 0
                // -sin(angle) cos(angle) 0
                //     0          0       1

                float angle = Angles::NormAngle(log[i].GetOrientation() + asin(cvmGet(H,0,1)));

			    newlog[i] = TrackEntry( cvPoint2D32f(pDst.x, pDst.y),
                                        angle,
                                        log[i].GetError(),
                                        log[i].GetTimeStamp(),
                                        log[i].wgm() );
		    }
	    }
    }

    /**
	    Compute the average frame rate from the time-stamps
	    (time-stamps are assumed to be in milli-seconds.

	    We use average frame rate to determine time thresholds.
	    If variance of frame rate is high then thresholds
	    won't work and there will be gaps in output.
	    In that case comething is wrong with the video capture
	    anyway (e.g. excessive frame dropping).
    **/
    double AverageFpsSec( const TrackHistory::TrackLog& in )
    {
	    if (in.size()<1) return 0.f;

	    double sum = 0.0;
	    for ( unsigned int i=0; i<in.size()-1; ++i )
	    {
		    sum += fabs(in[i+1].t()-in[i].t());
	    }

	    sum /= in.size();

	    return 1.0/sum;
    }

    /**
    	Convert a log from cm to pixels
    **/
    void LogCmToPx( const TrackHistory::TrackLog& in,
                    TrackHistory::TrackLog& out,
                    float scale,
                    CvPoint2D32f offset )
    {
	    out.clear();
	    out.reserve( in.size() );

	    for ( unsigned int i=0; i<in.size(); ++i )
	    {
		    out.push_back( in[i] );
		    CvPoint2D32f pos = cvPoint2D32f( in[i].x()*scale - offset.x, in[i].y()*scale - offset.y );
		    out[i].SetPosition( pos );
	    }
    }

    /**
	    Convert a log from pixels to cm
    **/
    void LogPxToCm( const TrackHistory::TrackLog& in,
                    TrackHistory::TrackLog& out,
                    float scale,
                    CvPoint2D32f offset )
    {
	    out.clear();
	    out.reserve( in.size() );

	    for ( unsigned int i=0; i<in.size(); ++i )
	    {
		    out.push_back( in[i] );
		    CvPoint2D32f pos = cvPoint2D32f( (in[i].x() + offset.x)/scale, (in[i].y() + offset.y)/scale );
		    out[i].SetPosition( pos );
	    }
    }

    void LogPxToImage( const TrackHistory::TrackLog& in,
                       TrackHistory::TrackLog& out,
                       const CameraCalibration* cal,
                       const CvPoint2D32f* offset )
    {
	    out.clear();
	    out.reserve( in.size() );

	    for ( unsigned int i=0; i<in.size(); ++i )
	    {
		    out.push_back( in[i] );

            CvPoint2D32f pos = in[i].GetPosition();
            pos.x += offset->x;
            pos.y += offset->y;
            CvPoint2D32f oldPosf = cal->PlaneToImage( pos );

		    out[i].SetPosition( oldPosf );
	    }
    }

    void LogImageToPx( const TrackHistory::TrackLog& in,
                       TrackHistory::TrackLog& out,
                       const CameraCalibration* cal,
                       const CvPoint2D32f* offset )
    {
	    out.clear();
	    out.reserve( in.size() );

	    for ( unsigned int i=0; i<in.size(); ++i )
	    {
		    out.push_back( in[i] );

            CvPoint2D32f pos = in[i].GetPosition();
            CvPoint2D32f oldPosf = cal->ImageToPlane( pos );
            oldPosf.x -= offset->x;
            oldPosf.y -= offset->y;

		    out[i].SetPosition( oldPosf );
	    }
    }

    /**
        Swaps log between right and left handed coords.
        (Just flips y-coord).
    **/
    void LogSwapHandedness( TrackHistory::TrackLog& log )
    {
	    for ( unsigned int i=0; i<log.size(); ++i )
        {
            CvPoint2D32f point = log[i].GetPosition();
            point.y = -point.y;
            log[i].SetPosition( point );
        }
    }

    /**
	    Rotates and translates a track-history log so that the first entry
	    is at (0,0) and the initial heading is directly down the y-axis.

	    Timestamp and error fields are left unchanged.
    **/
    void ConvertToRelativeLog( const TrackHistory::TrackLog& in,
                               TrackHistory::TrackLog& out )
    {
	    out.clear();

	    if ( in.size() > 0 )
	    {
		    out.resize( in.size() );

		    CvPoint2D32f pStart = in[0].GetPosition();
		    float oStart = in[0].GetOrientation();

		    // rotation to undo initial orientation
		    float cosa = cosf( (MathsConstants::D_PI*.5)-oStart );
		    float sina = sinf( (MathsConstants::D_PI*.5)-oStart );

		    for	( unsigned int i=0; i<in.size(); ++i )
		    {
			    CvPoint2D32f point = in[i].GetPosition();

			    // subtract origin (will be zero so have no effect if relative is false)
			    float x =  point.x - pStart.x;
			    float y =  point.y - pStart.y;

			    // rotate data (will be identity so have no effect if relative is false)
			    point.x = (cosa*x) - (sina*y);
			    point.y = (sina*x) + (cosa*y);

			    float angle = Angles::NormAngle(in[i].GetOrientation() - oStart);

			    // Save into the new log
			    out[i] = TrackEntry( point,
                                     angle,
                                     in[i].GetError(),
                                     in[i].GetTimeStamp(),
                                     in[i].wgm() );
		    }
	    }
    }
}
