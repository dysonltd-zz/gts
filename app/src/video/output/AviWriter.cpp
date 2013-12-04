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

#include "AviWriter.h"

#include <cstring>
#include <cassert>


const int AviWriter::XVID_COMPRESSION_CODEC = CV_FOURCC('X', 'V', 'I', 'D');  // use Xvid codec
const int AviWriter::FMP4_COMPRESSION_CODEC = CV_FOURCC('F', 'M', 'P', '4');  // use MPEG-4 codec

//-----------------------------------------------------------------------------------------------------------------

/** @brief Construct the writer.
 *
 *  @param fileName     The name of the output AVI file.
 *  @param framesPerSec The frame rate of the generated AVI.
 *  @param aviWidth     The width of the generated AVI.
 *  @param aviHeight    The height of the generated AVI.
 */
AviWriter::AviWriter(const codecType   codec,
                      const int         aviWidth,
                      const int         aviHeight,
                      const char* const videoFileName,
                      const char* const timestampFileName,
                      const double      frameRate) :
    m_aviWidth      (aviWidth),
    m_aviHeight     (aviHeight),
    m_numChannels   (DEFAULT_NUM_CHANNELS),
    m_timestampFile (timestampFileName)
{
    switch (codec)
    {
        case CODEC_XVID:
            m_avi = cvCreateVideoWriter(videoFileName,
                                         AviWriter::XVID_COMPRESSION_CODEC,
                                         frameRate,
                                         cvSize(aviWidth, aviHeight));
            break;

        case CODEC_FMP4:
            m_avi = cvCreateVideoWriter(videoFileName,
                                         AviWriter::FMP4_COMPRESSION_CODEC,
                                         frameRate,
                                         cvSize(aviWidth, aviHeight));
            break;
    }

    m_img = CreateImage(aviWidth, aviHeight);

    if (m_timestampFile.open(QFile::WriteOnly))
    {
        m_timestampStream.setDevice(&m_timestampFile);
    }
}

/** @brief Destructor.
 *
 */
AviWriter::~AviWriter()
{
    // delete the image
    cvReleaseImage(&m_img);

    // shut down the video writer
    cvReleaseVideoWriter(&m_avi);

    m_timestampFile.close();
}

/** @brief Create an @a IplImage of the appropriate size.
 *
 *  @param width  The image width.
 *  @param height The image height.
 *  @return A pointer to a newly-allocated IplImage with the default number of
 *  channels / image depth and the size specified.  The caller takes ownership.
 */
IplImage* const AviWriter::CreateImage(const int width, const int height) const
{
    return cvCreateImage(cvSize(width, height), DEFAULT_IMAGE_DEPTH, m_numChannels);
}

/** @brief Append a new frame to the AVI file.
 *
 *  @param data Pointer to the image data.
 *  @param frameWidth  The width of the image contained in data.
 *  @param frameHeight The height of the image contained in data.
 */
void AviWriter::addFrame(const char* const data,
                          const int frameWidth,
                          const int frameHeight,
                          const timespec& stamp)
{
    /// @bug [potential] Assumes num channels & channel width in frame
    /// are the same as we expect!
    const int expectedDataSize = m_aviHeight*m_aviWidth*m_numChannels;
    const int frameDataSize    = frameHeight*frameWidth*m_numChannels;

    if (frameDataSize == expectedDataSize)
    {
        // directly copy the image
        std::memcpy(m_img->imageData, data, frameDataSize);
    }
    else
    {
        // resize and copy the image data
        // Create a temporary image of the size provided
        IplImage* frameImg = CreateImage(frameWidth, frameHeight);

        // copy the data into the frame image
        std::memcpy(frameImg->imageData, data, frameDataSize);

        // scale the temporary image to the output image
        cvResize(frameImg, m_img);

        // Release the frame image
        cvReleaseImage(&frameImg);
    }

    // write a frame to the avi writer
    cvWriteFrame(m_avi, m_img);

    // Write the corresponding timestamp to the timestamp file:
    m_timestampStream << stamp.tv_sec << ' ' << stamp.tv_nsec << '\n';
}
