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

#include "FakeCameraApi.h"
#include "CameraDescription.h"

#include <QtCore/QMutex>
#include <QtGlobal>

#include <opencv/cv.h>

#include <cstdlib>

namespace
{
    class FakeVideoSequence : public VideoSequence
    {
    public:
        FakeVideoSequence() :
            m_image( cvCreateImage(cvSize(600, 400), IPL_DEPTH_8U, 1) )
        {
        }

        virtual ~FakeVideoSequence()
        {
            cvReleaseImage(&m_image);
        }

        /** @brief Sequence can step backwards through frames
         */
        virtual bool IsRewindable() const
        {
            return false;
        }

        /** @brief Sequence can step forward through frames
         */
        virtual bool IsForwardable() const
        {
            return false;
        }

        /** @brief Sequence can step forwards or backwards
         */
        virtual bool IsWindable() const
        {
            return false;
        }

        /** @brief Sequence is coming from a live camera (e.g. in real-time).
         */
        virtual bool IsLive() const
        {
            return true;
        }

        /** @brief prepare next frame internally
         */
        virtual bool ReadyNextFrame()
        {
            GenerateRandomImage();
            return true;
        }

        virtual bool ReadyNextFrame( double msec)
        {
            Q_UNUSED(msec);
            GenerateRandomImage();
            return true;
        }

        /** @brief get pointer to internally stored frame
         */
        virtual const IplImage* RetrieveNextFrame() const
        {
            QMutexLocker locker(&m_mutex);
            return m_image;
        }

        /** @brief return timestamp of ready frame
         */
        virtual double GetTimeStamp() const
        {
            return 0.;
        }

        /** @brief return index number of ready frame
         */
        virtual double GetFrameIndex() const
        {
            return 0;
        }

        /** @brief return total number of frames in sequence or -1 if sequence is live
         */
        virtual double GetNumFrames()  const
        {
            return -1;
        }

        /** @brief Get the width of a frame.
         */
        virtual int GetFrameWidth() const
        {
            return m_image->width;
        }

        /** @brief Get the height of a frame.
         */
        virtual int GetFrameHeight() const
        {
            return m_image->width;
        }

        /** @brief Was this Sequence successfully set-up?
         *
         *  @return @a true if the the sequence is set up and ready
         *  to use, otherwise, @a false.
         */
        virtual bool IsSetup() const
        {
            return (m_image != 0);
        }

        /** @brief Set the frame-rate of the source.
         *
         *  The implementation should try to get a close frame-rate
         *  if the specified one is not possible.
         *
         *  @param fps The new frame rate to set.
         */
        virtual void SetFrameRate( const double fps )
        {
            Q_UNUSED(fps);
        }

        virtual double GetFrameRate()
        {
            return 0.0;
        }

        virtual int Flip() const { return 0; };

        virtual void ReadyFrame() {};
        virtual bool TakeFrame() { return true; };

    private:
        void GenerateRandomImage()
        {
            QMutexLocker locker(&m_mutex);
            const uchar MAX_UCHAR = 255;
            for (int r = 0; r < m_image->height; ++r)
            {
                uchar* const data = reinterpret_cast<uchar*>(m_image->imageData + (r*m_image->widthStep));
                uchar colour = 0;
                for (int c = 0; c < m_image->width; ++c)
                {
                    const int   randomInt = std::rand();
                    const float randomFloat = (1.f*randomInt)/RAND_MAX;
                    const float swapProbability = 0.2f;
                    if (randomFloat < swapProbability)
                    {
                        //swap colour
                        colour = MAX_UCHAR - colour;
                    }
                    data[c] = colour;
                }
            }
        }

        mutable QMutex m_mutex;
        IplImage* m_image;
    };
}

FakeCameraApi::FakeCameraApi()
{}

FakeCameraApi::~FakeCameraApi()
{}

const CameraApi::CameraList FakeCameraApi::EnumerateCameras() const
{
    CameraList list;
    list.push_back( CameraDescription(*this).WithName(L"The Fake Camera")
                                            .WithDescription(L"For Testing Purposes")
                                            .WithUniqueId(L"fakeCamera:://00000"));
    return list;
}

VideoSequence* const FakeCameraApi::CreateVideoSequenceForCamera( const CameraDescription& camera ) const
{
    Q_UNUSED(camera);

    return new FakeVideoSequence;
}
