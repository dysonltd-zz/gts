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

#ifndef VIDEO_SEQUENCE_H
#define VIDEO_SEQUENCE_H

#include <opencv/cv.h>

/** @brief Class for video playback.
 *
 *  Intention is that underlying video could come from a live source,
 *  from a video file, or from a sequence of images, but these would
 *  all be handled transparently.
**/
class VideoSequence
{
public:
    VideoSequence() {}
    virtual ~VideoSequence() {}

    /** @brief Sequence can step backwards through frames
     */
    virtual bool IsRewindable() const = 0;

    /** @brief Sequence can step forward through frames
     */
    virtual bool IsForwardable() const = 0;

    /** @brief Sequence can step forwards or backwards
     */
    virtual bool IsWindable() const = 0;

    /** @brief Sequence is coming from a live camera (e.g. in real-time).
     */
    virtual bool IsLive() const = 0;

    /** @brief prepare next frame internally
     */
    virtual bool ReadyNextFrame() = 0;
    virtual bool ReadyNextFrame( double msec ) = 0;

    /** @brief get pointer to internally stored frame
     */
    virtual const IplImage* RetrieveNextFrame() const = 0;

    /** @brief return timestamp of ready frame
     */
    virtual double GetTimeStamp() const = 0;

    /** @brief return index number of ready frame
     */
    virtual double GetFrameIndex() const = 0;

    /** @brief return total number of frames in sequence or -1 if sequence is live
     */
    virtual double GetNumFrames()  const = 0;

    /** @brief Get the width of a frame.
     */
    virtual int GetFrameWidth() const = 0;

    /** @brief Get the height of a frame.
     */
    virtual int GetFrameHeight() const = 0;

    /** @brief Was this Sequence successfully set-up?
     *
     *  @return @a true if the the sequence is set up and ready
     *  to use, otherwise, @a false.
     */
    virtual bool IsSetup() const = 0;

    /** @brief Set the frame-rate of the source.
     *
     *  The implementation should try to get a close frame-rate
     *  if the specified one is not possible.
     *
     *  @param fps The new frame rate to set.
     */
    virtual void SetFrameRate( const double fps ) = 0;

    virtual double GetFrameRate() = 0;

    virtual int Flip() const = 0;

    virtual void ReadyFrame() = 0;
    virtual bool TakeFrame()  = 0;

private:
};

#endif // VIDEO_SEQUENCE_H
