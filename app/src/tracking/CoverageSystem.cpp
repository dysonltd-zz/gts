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

#include "CoverageSystem.h"

#include "KltTracker.h"
#include "RobotMetrics.h"

#include "Logging.h"

#include <opencv/highgui.h>

/**
    A coverage system is initialised with
    the dimensions of the tracking image.
 **/
CoverageSystem::CoverageSystem(CvSize warpedImageSize) :
    m_cvgMask    (0),
    m_floorMask  (0),
    m_floorPixels(0)
{
    m_cvgMask = cvCreateImage(warpedImageSize, IPL_DEPTH_8U, 1);
    cvZero(m_cvgMask);

    m_inOutMask = cvCreateImage(warpedImageSize, IPL_DEPTH_8U, 1);
    m_colMap = cvCreateImage(warpedImageSize, IPL_DEPTH_8U, 3);
}

/**
    Clean up images in destructor.
 **/
CoverageSystem::~CoverageSystem()
{
    cvReleaseImage(&m_cvgMask);
    cvReleaseImage(&m_floorMask);

    cvReleaseImage(&m_inOutMask);
    cvReleaseImage(&m_colMap);
}

/**
    This function does an update for repeat coverage monitoring.
    The robot moves in and out of pixels inthe floor plane, and
    a counter is incremented for each pixel as it is uncovered
    by the robot.

    @param prev Robot's previous position.
    @param curr Robot's current position.
    @param radiusPx Radius of robot's base.
 **/
void CoverageSystem::Update(CvPoint2D32f prev, CvPoint2D32f curr, float radiusPx)
{
    cvZero(m_inOutMask);

    int radius = (int)(radiusPx + .5f);
    CvPoint pp = cvPoint((int)(prev.x + .5f), (int)(prev.y + .5f));
    CvPoint pc = cvPoint((int)(curr.x + .5f), (int)(curr.y + .5f));

    cvCircle(m_inOutMask, pp, radius, CV_RGB(255, 255, 255), CV_FILLED); // previous occupancy
    cvCircle(m_inOutMask, pc, radius, CV_RGB(0, 0, 0), CV_FILLED); // effectively 'subtracts' current occupancy from previous

    IncrementUncoveredPixels();
}

/**
    Updates the coverage system with the polygon swept out by the brush bar.

    @param pl Previous position of left edge of brush bar.
    @param pr Previous position of right edge of brush bar.
    @param cl Current position of left edge of brush bar.
    @param cr Current position of right edge of brush bar.
 **/
void CoverageSystem::BrushBarUpdate(CvPoint2D32f pl,
                                     CvPoint2D32f pr,
                                     CvPoint2D32f cl,
                                     CvPoint2D32f cr)
{
    cvZero(m_inOutMask);

    CvPoint poly[4] =
    {
        cvPoint(static_cast<int>(pl.x), static_cast<int>(pl.y)),
        cvPoint(static_cast<int>(pr.x), static_cast<int>(pr.y)),
        cvPoint(static_cast<int>(cr.x), static_cast<int>(cr.y)),
        cvPoint(static_cast<int>(cl.x), static_cast<int>(cl.y))
    };

    cvFillConvexPoly(m_inOutMask, poly, 4, cvScalar(255, 255, 255));

    // Erase last row of pixels
    cvFillConvexPoly(m_inOutMask, &(poly[2]), 2, cvScalar(0, 0, 0));

    // intersect with floor mask
    cvAnd(m_inOutMask, m_floorMask, m_inOutMask);

    IncrementUncoveredPixels();
}

/**
    Create a map indicating the number of times the robot
    has covered particular areas using different colours.
 **/
void CoverageSystem::CreateColouredMap()
{
    cvZero(m_colMap);

    unsigned char colour[11][3] = { { 0, 0, 0 },
                                    { 0, 40, 0 },
                                    { 0, 80, 0 },
                                    { 0, 120, 0 },
                                    { 0, 160, 0 },
                                    { 0, 200, 0 },
                                    { 0, 240, 0 },
                                    { 0, 255, 0 },
                                    { 255, 255, 0 },
                                    { 255, 128, 0 },
                                    { 0, 0, 255 } };

    int step = m_cvgMask->widthStep;
    for (int r = 0; r < m_cvgMask->height; ++r)
    {
        char* pMask = m_cvgMask->imageData + (r * step);
        const char* const pEnd = pMask + (m_cvgMask->width * m_cvgMask->nChannels);

        char* pCol = m_colMap->imageData + (r * m_colMap->widthStep);

        while (pMask != pEnd)
        {
            int index = *pMask++;

            if (index > 8 && index <= 13)
            {
                index = 8;
            }
            else if (index > 13 && index <= 18)
            {
                index = 9;
            }
            else if (index > 18)
            {
                index = 10;
            }

            *pCol++ = colour[index][0];
            *pCol++ = colour[index][1];
            *pCol++ = colour[index][2];
        }
    }
}

/**
    Increment the coverage count for any pixels in m_cvgMask which
    have just been uncovered (i.e. which are on in the m_inOutMask).

    This is a bit slow at the moment but can be much improved:
    we only need to loop over pixels in  a square about the robot's
    previous position pp (since only those pixels can be uncovered each
    step). At the moment it checks every pixel in the image.
 **/
void CoverageSystem::IncrementUncoveredPixels()
{
    assert(m_cvgMask->widthStep == m_inOutMask->widthStep);

    int step = m_cvgMask->widthStep;
    for (int r = 0; r < m_cvgMask->height; ++r)
    {
        char* pMask = m_cvgMask->imageData + (r * step);
        const char* const pEnd = pMask + (m_cvgMask->width * m_cvgMask->nChannels);

        char* pTest = m_inOutMask->imageData + (r * step);

        while (pMask != pEnd)
        {
            if ((*pTest) != 0)
            {
                (*pMask) += 1;
            }

            pMask++;
            pTest++;
        }
    }
}

/**
    Pass in a tracker and update overage
    mask based on current robot position.

    Currently fills in a circle at the location
    of the base of the robot. Could be modified
    to only fill region swept out by brush-bar.

    @param tracker A tracker whose current position
                   will be used to update the mask.
 **/
//void CoverageSystem::Update(const RoboTrackKlt& tracker)
//{
//    CvPoint2D32f basePos = tracker.AdjustTrackForRobotHeight(tracker.GetPosition());
//    CvPoint pb = cvPoint((int)(basePos.x+.5f),(int)(basePos.y+.5f));
//
//    int radius = (int)tracker.GetMetrics()->GetBasePx();
//    cvCircle(m_cvgMask, pb, radius, cvScalar(255,255,255), CV_FILLED, CV_AA);
//}

/**
    Update coverage mask by directly
    passing in position and radius.
 **/
void CoverageSystem::DirectUpdate(CvPoint pb, float radiusPx)
{
    int radius = (int)(radiusPx + .5f);

    cvCircle(m_cvgMask, pb, radius, cvScalar(255, 255, 255), CV_FILLED, CV_AA);
}

/**
    Render the repeat-coverage map as a
    transparent overlay on another image.

    @param img The image to render to
 **/
void CoverageSystem::DrawMap(IplImage* img) const
{
    cvAddWeighted(img, 0.4, m_colMap, 0.6, 0, img);
}

/**
    Render coverage mask to another
    image as a transparent overlay.

    @param img The image to render to
 **/
void CoverageSystem::DrawMask(IplImage* img, CvScalar colour) const
{
    assert(img->width == m_cvgMask->width);
    assert(img->height == m_cvgMask->height);

    int h = m_cvgMask->height;
    int w = m_cvgMask->width;
    for (int j = 0; j < h; ++j)
    {
        char* pImg = img->imageData + j * img->widthStep;
        const char* pMask = m_cvgMask->imageData + j * m_cvgMask->widthStep;

        for (int i = 0; i < w; ++i)
        {
            unsigned char mval = (unsigned char)*pMask++;
            float a = (mval) / 255.f;
            float b = 1.f - a;

            for (int c = 0; c < img->nChannels; ++c)
            {
                unsigned char pxl = (unsigned char)*pImg;
                float fill = (float)(colour.val[c]) * (pxl / 255.f);
                float col = a * fill + b * pxl;
                *pImg++ = (char)col;
            }
        }
    }
}

/**

 **/
void CoverageSystem::SaveMask(const char* file_name)
{
    cvSaveImage(file_name, m_cvgMask);
}

/**
    Compute how much of the floor area has been covered by robot
    path (that we have accumulated in the coverage mask so far).

    Percentage coverage estimated by counting pixels in both masks.

    @param floormask A mask (of correct dimensions) which is 255
                     for floor pixels and 0 for non-floor pixels.

    @return Percentage of floor covered by robot track.
 **/
float CoverageSystem::EstimateCoverage(const IplImage* floormask) const
{
    return GetCoveredPixelCount() * (100.f / cvCountNonZero(floormask));
}

/**
    Counts the number of pixels which have
    value 255 in the robot coverage mask.

    @return The number of white pixels in the coverage mask.
 **/
unsigned int CoverageSystem::GetCoveredPixelCount() const
{
    return cvCountNonZero(m_cvgMask);
}

/**
    Returns number of white pixels in single channel image.

    @param mask A single channel (grey-level) image.

    @return Number of white pixels in mask.
 **/
unsigned int CoverageSystem::CountWhitePixels(const IplImage* mask)
{
    if (!mask)
    {
        return 0;
    }

    assert(mask->nChannels == 1);

    unsigned int count = 0;
    for (int j = 0; j < mask->height; ++j)
    {
        for (int i = 0; i < mask->width; ++i)
        {
            if (cvGet2D(mask, j, i).val[0] == 255.0)
            {
                count++;
            }
        }
    }

    return count;
}

/**
    Returns the number of pixels >1.

    @param mask A single channel (grey-level) image.

    @return Number of pixels covered more than once.
 **/
unsigned int CoverageSystem::CountRepeatCoverage(const IplImage* mask)
{
    if (!mask)
    {
        return 0;
    }

    assert(mask->nChannels == 1);

    unsigned int count = 0;
    for (int j = 0; j < mask->height; ++j)
    {
        for (int i = 0; i < mask->width; ++i)
        {
            float val = cvGet2D(mask, j, i).val[0];
            if (val > 1)
            {
                count++;
            }
        }
    }

    return count;
}

/**
    Load floor mask image from disk and store internally.
    Also compute the number of white pixels in the mask.

    @param filename Name of floor-mask image file.

    @return True if mask was read sucessfully, false if not.
 **/
bool CoverageSystem::LoadFloorMask(const char* filename)
{
    IplImage* mask = cvLoadImage(filename, 0);

    if (!mask)
    {
        LOG_INFO(QObject::tr("Could not open floor mask %1!").arg(filename));

        return false;
    }

    SetFloorMask(mask);
    cvReleaseImage(&mask); // SetFloorMask makes its own copy so release

    // Check size of loaded mask matches floor-coverage mask
    if (m_floorMask->width == m_cvgMask->width &&
         m_floorMask->height == m_cvgMask->height &&
         m_floorMask->nChannels == 1)
    {
        return true;
    }
    else
    {
        LOG_ERROR("Floor mask has incorrect size or colour depth!");

        LOG_ERROR(QObject::tr("Expected a %1x%2 grey-scale image!").arg(m_cvgMask->width)
                                                                   .arg(m_cvgMask->height));
        assert(0);
        return false;
    }
}

/**
    @brief Set the floor mask for the coverage system.

    CoverageSystme makes its own internal copy of mask.
 **/
void CoverageSystem::SetFloorMask(const IplImage* mask)
{
    if (m_floorMask)
    {
        cvReleaseImage(&m_floorMask);
        m_floorPixels = 0;
    }

    if (mask)
    {
        m_floorMask = cvCloneImage(mask);
        m_floorPixels = cvCountNonZero(m_floorMask);
    }
}

/**
    @brief Estimates coverage using the floor mask and track masks which are stored internally.

    Calling this function without loading a floor mask is an error.
    Calling this function with an emtpy floor mask (no white pixels) is also considered an error.

    @return The percentage coverage, or -1.f if there is an error.
 **/
float CoverageSystem::EstimateCoverage() const
{
    if (!m_floorMask || m_floorPixels == 0)
    {
        return -1.f;
    }

    return GetCoveredPixelCount() * (100.f / m_floorPixels);
}

/**
    Returns percentage of floor space
    that was covered more than once.
 **/
float CoverageSystem::EstimateRepeatCoverage() const
{
    if (!m_cvgMask || m_floorPixels == 0)
    {
        return -1.f;
    }

    return CountRepeatCoverage(m_cvgMask) * (100.f / m_floorPixels);
}

/**
    Computes the area that was covered a particular number of
    times and writes the result to a specified log file.
 **/
void CoverageSystem::CoverageHistogram(const char* file) const
{
    FILE* fp = fopen(file, "w");

    if (!fp || !m_floorMask || m_floorPixels == 0)
    {
        return;
    }

    // First make a copy of the coverage mask.
    IplImage* mask = cvCloneImage(m_cvgMask);
    IplImage* count = cvCloneImage(mask);

    double t = 1.0;
    for (int i = 1; i < 256; ++i)
    {
        cvCmpS(m_cvgMask, t, mask, CV_CMP_EQ); // Find all pixels that were covered 't' times
        cvAnd(mask, m_floorMask, count); // then intersect those pixels with floor mask
        int nPixels = cvCountNonZero(count); // then count the result

        t += 1.0;

        // Work out percentage of floor area covered.
        float pc = nPixels * (100.f / m_floorPixels);
        fprintf(fp, "%d %f\n", i, pc);
    }

    fclose(fp);

    cvReleaseImage(&mask);
    cvReleaseImage(&count);
}

/**
    @brief Writes incremental coverage data for the current state of floor coverage:

    For each level of coverage writes the percentage of floor covered by that amount.

    @param fp A valid open file pointer to which the data will be written.
    @param count The maximum coverage count in which we are interested.
 **/
void CoverageSystem::WriteIncrementalCoverage(FILE* fp, unsigned int count)
{
    if (fp)
    {
        IplImage* dst = cvCreateImage(cvSize(m_cvgMask->width,
                                               m_cvgMask->height), IPL_DEPTH_8U, 1);

        IplImage* countImage = cvCloneImage(dst);

        for (unsigned int i = 1; i <= count; ++i)
        {
            cvCmpS(m_cvgMask, i, dst, CV_CMP_GE);

            // Intersect pixels with floor mask.
            cvAnd(dst, m_floorMask, countImage);
            float cov = cvCountNonZero(countImage) * (100.f / m_floorPixels);
            fprintf(fp, " %f", cov);
        }

        fprintf(fp, "\n");

        cvReleaseImage(&dst);
    }
}

/**
    Create a mask which shows the areas of
    the floor that were missed completely.
 **/
int CoverageSystem::MissedMask(const char* fileName)
{
    int count = -1;

    if (m_floorMask)
    {
        IplImage* dst = cvCreateImage(cvSize(m_cvgMask->width,
                                               m_cvgMask->height), IPL_DEPTH_8U, 1);

        cvCmpS(m_cvgMask, 1, dst, CV_CMP_GE);

        cvSub(m_floorMask, dst, dst);

        cvSaveImage(fileName, dst);

        count = cvCountNonZero(dst);

        cvReleaseImage(&dst);
    }

    return count;
}
