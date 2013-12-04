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

#ifndef OPENCVTOOLS_H
#define OPENCVTOOLS_H

#include "Logging.h"

#include <opencv/cv.h>

namespace OpenCvTools
{
    /**
      @brief Check if IplImage is valid (contains data, width & height > 0)
      @param iplImage Image to validate
      @return True if valid
     **/
    bool IsValid(const IplImage* const iplImage);

    /**
      @brief Check if cv::Mat is valid (contains data, width & height > 0)
      @param mat Image to validate
      @return True if valid
    **/
    bool IsValid(const cv::Mat* const mat);

    /**
      Draws a coloured overlay of the @a mask into the @a img using the
      supplied @a colour.
      @param img The image to draw into.
      @param mask The mask to be overlaid into the @a img.
      @param colour The colour to use for the overlay.
      @param predicate Tests whether the pixel in the mask is a match.
    **/
    template <typename Pred>
    void DrawColouredOverlay(IplImage* img, const IplImage* mask, CvScalar colour, Pred predicate)
    {
        if (mask->nChannels != 1)
        {
            LOG_ERROR("Too many channels in mask!");

            return;
        }

        for (int j = 0; j < mask->height; ++j)
        {
            for (int i = 0; i < mask->width; ++i)
            {
                int val = (int)cvGet2D(mask, j, i).val[0];
                if (predicate(val))
                {
                    cvSet2D(img, j, i, colour);
                }
            }
        }
    }

   /**
      Draws a coloured overlay of the @a mask into the @a img using the
      supplied @a colour.
      @param img The image to draw into.
      @param mask The mask to be overlaid into the @a img.
      @param colour The colour to use for the overlay.
      @param predicate Tests whether the pixel in the mask is a match.
    **/
    template <typename Pred>
    void DrawColouredOverlay(cv::Mat* img, const cv::Mat* mask, cv::Scalar colour, Pred predicate)
    {
        if (mask->channels() != 1)
        {
            LOG_ERROR("Too many channels in mask");
            return;
        }

        for (int j = 0; j < mask->size().height; ++j)
        {
            for (int i = 0; i < mask->size().width; ++i)
            {

                int val = (int)mask->at<uchar>(i,j);
                if (predicate(val))
                {
                    mask->at<uchar>(i,j) = colour;
                }
            }
        }
    }

    /**
      @brief Loads file from disk and converts to grayscale
      @param fileName File path
      @return Pointer to IplImage
    **/
    IplImage* LoadSingleChannelImage(const std::string& fileName);

    /**
      @brief Loads file from disk and converts to grayscale
      @param fileName File path
      @return Pointer to IplImage
     **/
    //cv::Mat* LoadSingleChannelImage(const std::string& fileName);

    /**
      Counts the number of pixels in @a rawCoverageImg, where the pixel
      value compares to @a nTimes using the @a cmp operation.
      @param rawCoverageImg An image containing raw-coverage data, with
      the coverage count as the absolute pixel-value.
      @param nTimes The number of coverage times (i.e. the pixel-value)
      to count within the @a rawCoverageImg
      @param cmp the comparison operator for locating pixels of interest
    **/
    int GetPixelCoverageCount(const IplImage* rawCoverageImg,
                              const int nTimes,
                              const int cmp);
}

#endif // OPENCVTOOLS_H
