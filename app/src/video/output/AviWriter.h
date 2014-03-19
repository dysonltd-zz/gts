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

#ifndef AVIWRITER_H
#define AVIWRITER_H

#include <opencv/cv.h>
#include <opencv/highgui.h>

#if defined(__MINGW32__) || defined(_MSC_VER)
    #include <WinTime.h>
#endif

#include <QTextStream>
#include <QFile>

/** @brief Class to write AVI files.
 *
 * A thin wrapper around CvVideoWriter.
 */
class AviWriter
{
public:
    enum codecType
    {
        CODEC_XVID = 0,
        CODEC_FMP4
    };

public:
    AviWriter( const codecType   codec,
               const int         aviWidth,
               const int         aviHeight,
               const char* const videoFileName,
               const char* const timestampFileName,
               const double      frameRate );

    ~AviWriter();

    void addFrame(const char* const data,
                  const int frameWidth,
                  const int frameHeight,
                  const timespec& stamp);

private:
    IplImage* const CreateImage( const int width, const int height ) const;

    static const int XVID_COMPRESSION_CODEC;
    static const int FMP4_COMPRESSION_CODEC;
    static const int DEFAULT_NUM_CHANNELS = 3;
    static const int DEFAULT_IMAGE_DEPTH = IPL_DEPTH_8U;

    int m_aviWidth;       ///< The width of the AVI being generated.
    int m_aviHeight;      ///< The width of the AVI being generated.
    int m_numChannels;    ///< The number of colour channels in the AVI being generated.

    CvVideoWriter* m_avi; ///< OpenCV Video writer.
    IplImage*      m_img; ///< @brief The OpenCV image to use for appending to the AVI
                          ///         (to ensure we maintain the frame size / format).

    QFile          m_timestampFile;
    QTextStream    m_timestampStream;
};

#endif // AVIWRITER_H
