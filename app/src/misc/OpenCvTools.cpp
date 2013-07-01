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

#include "OpenCvTools.h"

#include <opencv/cv.h>
#include <opencv/highgui.h>

bool IsValid( const IplImage* const iplImage )
{
    if ( !iplImage ) return false;
    if ( iplImage->width  <= 0 ) return false;
    if ( iplImage->height <= 0 ) return false;

    return true;
}

/**
 * Loads the given image file, converting it (if necessary) to a
 * single-channel (B&W) image.
 * @param fileName the name of the image-file to load.
 * @return A single-channel (B&W) image, or NULL if it cannot be
 * loaded or converted for some reason.
 */
IplImage* LoadSingleChannelImage(const std::string& fileName)
{
    IplImage* img = cvLoadImage( fileName.c_str(), CV_LOAD_IMAGE_GRAYSCALE );

    if (!img)
    {
        LOG_ERROR(QObject::tr("Could not load image %1!").arg(fileName.c_str()));

        return 0;
    }

    if (img->nChannels > 1)
    {
        LOG_INFO(QObject::tr("Converting %1 to single-channel.").arg(fileName.c_str()));

        IplImage* tmp = cvCreateImage( cvSize( img->width, img->height), IPL_DEPTH_8U, 1 );

        cvConvertImage( img, tmp );
        cvReleaseImage( &img );

        img = tmp;
    }

    return img;
}

/**
 * Counts the number of pixels in @a rawCoverageImg, where the pixel
 * value compares to @a nTimes using the @a cmp operation.
 *
 * @param rawCoverageImg An image containing raw-coverage data, with
 * the coverage count as the absolute pixel-value.
 * @param nTimes The number of coverage times (i.e. the pixel-value)
 * to count within the @a rawCoverageImg
 * @param cmp the comparison operator for locating pixels of interest
 */
int GetPixelCoverageCount(const IplImage* rawCoverageImg,
                          const int nTimes,
                          const int cmp)
{
    // first make a copy of the raw-coverage mask, and another one to
    // count with
    IplImage* mask = cvCloneImage( rawCoverageImg );

    // Find all pixels that were covered exactly 'nTimes' times...
    cvCmpS( rawCoverageImg, nTimes, mask, cmp );
    const int nPixels = cvCountNonZero( mask );

    cvReleaseImage( &mask );

    return nPixels;
}
