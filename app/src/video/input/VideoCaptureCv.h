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

#ifndef VIDEO_CAPTURE_CV_H
#define VIDEO_CAPTURE_CV_H

#include "VideoSequence.h"

#include <opencv/highgui.h>

#include <iostream>

/** @brief Wrapper for OpenCv VideoCapture.
**/
class VideoCaptureCv : public VideoSequence
{
public:

    /** @brief Create a capture-from-camera object.
     *
     *  @param i The index of the camera according to the underlying API.
     */
    VideoCaptureCv(int i) : VideoSequence(), m_capture(0), m_avi(false)
    {
        m_capture = cvCreateCameraCapture(i);
        cvSetCaptureProperty( m_capture, CV_CAP_PROP_FPS, 7.5 );
    }

    /** @brief Create a capture-from-video-file object.
     *
     *  @param vidname The name of the video file.
     */
    VideoCaptureCv( const char* vidname ) : VideoSequence(), m_capture(0), m_avi(true) { m_capture = cvCaptureFromAVI( vidname ); }

    ~VideoCaptureCv() { cvReleaseCapture( &m_capture ); }

    /** @brief @copybrief VideoSequence::IsRewindable
     *  @copydetails VideoSequence::IsRewindable
     */
    virtual bool IsRewindable()  const  { return m_avi;  }

    /** @brief @copybrief VideoSequence::IsForwardable
     *  @copydetails VideoSequence::IsForwardable
     */
    virtual bool IsForwardable() const  { return m_avi;  }

    /** @brief @copybrief VideoSequence::IsWindable
     *  @copydetails VideoSequence::IsWindable
     */
    virtual bool IsWindable()    const  { return m_avi;  }

    /** @brief @copybrief VideoSequence::IsLive
     *  @copydetails VideoSequence::IsLive
     */
    virtual bool IsLive()        const  { return !m_avi; }

    /** @brief @copybrief VideoSequence::ReadyNextFrame
     *  @copydetails VideoSequence::ReadyNextFrame
     */
    virtual bool ReadyNextFrame() { return 0 != cvGrabFrame( m_capture ); }

    virtual bool ReadyNextFrame( double msec)
    {
        cvSetCaptureProperty( m_capture, CV_CAP_PROP_POS_MSEC, msec );

        return 0 != cvGrabFrame( m_capture );
    }

    /** @brief @copybrief VideoSequence::RetrieveNextFrame
     *  @copydetails VideoSequence::RetrieveNextFrame
     */
    virtual const IplImage* RetrieveNextFrame() const { return cvRetrieveFrame( m_capture ); }

    /** @brief @copybrief VideoSequence::GetTimeStamp
     *  @copydetails VideoSequence::GetTimeStamp
     */
    virtual double GetTimeStamp()  const { return cvGetCaptureProperty( m_capture, CV_CAP_PROP_POS_MSEC ); }

    /** @brief @copybrief VideoSequence::GetFrameIndex
     *  @copydetails VideoSequence::GetFrameIndex
     */
    virtual double GetFrameIndex() const { return cvGetCaptureProperty( m_capture, CV_CAP_PROP_POS_FRAMES ); }

    /** @brief @copybrief VideoSequence::GetNumFrames
     *  @copydetails VideoSequence::GetNumFrames
     */
    virtual double GetNumFrames()  const { return cvGetCaptureProperty( m_capture, CV_CAP_PROP_FRAME_COUNT ); }

    /** @brief @copybrief VideoSequence::GetFrameWidth
     *  @copydetails VideoSequence::GetFrameWidth
     */
    virtual int GetFrameWidth() const  { return (int)cvGetCaptureProperty( m_capture, CV_CAP_PROP_FRAME_WIDTH ); }

    /** @brief @copybrief VideoSequence::GetFrameHeight
     *  @copydetails VideoSequence::GetFrameHeight
     */
    virtual int GetFrameHeight() const { return (int)cvGetCaptureProperty( m_capture, CV_CAP_PROP_FRAME_HEIGHT ); }

    /** @brief @copybrief VideoSequence::SetFrameRate
     *  @copydetails VideoSequence::SetFrameRate
     */
    virtual void SetFrameRate( const double fps ) { cvSetCaptureProperty( m_capture, CV_CAP_PROP_FPS, fps ); }

    double GetFrameRate() { return cvGetCaptureProperty( m_capture, CV_CAP_PROP_FPS ); }

    /** @brief @copybrief VideoSequence::IsSetup
     *  @copydetails VideoSequence::IsSetup
     */
    virtual bool IsSetup() const { return (m_capture != 0); }

    virtual int Flip() const { return 0; };

    virtual void ReadyFrame() {};
    virtual bool TakeFrame() { return true; };

private:
    CvCapture* m_capture;

    /** @brief If file is an offline video (avi file) then m_avi is true.
     */
    bool m_avi;

};

#endif // VIDEO_CAPTURE_CV_H
