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

#ifndef GROUND_PLANE_UTILITY_H
#define GROUND_PLANE_UTILITY_H

#include "ScanMatch.h"

#include <opencv/cv.h>

#include <memory>

namespace GroundPlaneUtility
{
    struct Rect32f
    {
        CvPoint2D32f pos; // Position of rectangle (x and y coords).
        CvPoint2D32f dim; // Dimension of rectangle (width and height).
    };

    struct NewRect32f
    {
        cv::Point2f pos; // Position of rectangle (x and y coords).
        cv::Point2f dim; // Dimension of rectangle (width and height).
    };

    /**
      @brief Fill in an OpenCV Matrix with vertices for a planar calibration object of specified dimensions.
      The calibration object returned is given in pixels, this means the measurment units of the calibration are pixels
      (i.e. the CAMERA TRANSLATION resulting from calibration is in PIXELS)

      @note The order corner points in the x-axis the image coordinate system is flipped horizontally
      so that the perpsective-corrected ground-plane image is not mirrored/flipped. This can be changed by
      reversing the x-axis coords but other cahnges need to be made elsewhere to make    all the coordinate
      systems used consistent (and orientation output needs to be adjusted insuch a case also).

      @param width Width of the calibration board
      @param height Height of the calibration board
      @param squareSize size of calibration squares (in pixels) i.e. the size calibration squares will be mapped to.
    **/
    CvMat* CreateCalibrationObject(int width, int height, float squareSize);
    // cv::Mat* CreateCalibrationObject(int width, int height, float squareSize);

    /**
      @brief Simple utility function which calls cvFindExtrinsicParameters2 but internally
      converts from axis-angle to matrix representation for rotation.
    **/
    void ComputeExtrinsicParameters(const CvMat* objectPoints,
                                    const CvMat* imagePoints,
                                    const CvMat* intrinsicMatrix,
                                    const CvMat* distortionCoeffs,
                                    CvMat* rotMat,
                                    CvMat* trans);

    void ComputeExtrinsicParameters(const cv::Mat* objectPoints,
                                    const cv::Mat* imagePoints,
                                    const cv::Mat* intrinsicMatrix,
                                    const cv::Mat* distortionCoeffs,
                                    cv::Mat*       rotMat,
                                    cv::Mat*       trans);

    /**
      @brief For batch unwarping of ground plane (e.g. in video sequence)
      computes mapx and mapy suitable for use with remap function
      and allocates an image of the appropriate size. Also returns
      an offset used when forming the unwarped image - must be used
      when converting un-warped image coordinates back to the original image.
    **/
    IplImage* ComputeGroundPlaneWarpBatch(const IplImage* viewGrey,
                                          const CvMat* intrinsicMatrix,
                                          const CvMat* distortionCoeffs,
                                          const CvMat* inverseCoeffs,
                                          const CvMat* rotMat,
                                          const CvMat* trans,
                                          CvMat** mapx,
                                          CvMat** mapy,
                                          CvPoint2D32f* offset);

    std::shared_ptr<cv::Mat> ComputeGroundPlaneWarpBatch(const cv::Mat* viewGrey,
                                                         const cv::Mat* intrinsicMatrix,
                                                         const cv::Mat* distortionCoeffs,
                                                         const cv::Mat* inverseCoeffs,
                                                         const cv::Mat* rotMat,
                                                         const cv::Mat* trans,
                                                         cv::Mat* mapx,
                                                         cv::Mat* mapy,
                                                         cv::Point2f *offset);

    IplImage* UnwarpGroundPlane(const IplImage* viewGrey,
                                const CvMat* intrinsicMatrix,
                                const CvMat* distortionCoeffs,
                                const CvMat* inverseCoeffs,
                                const CvMat* rotMat,
                                const CvMat* trans,
                                CvPoint2D32f* offset);

    std::shared_ptr<cv::Mat> UnwarpGroundPlane(const cv::Mat* viewGrey,
                                               const cv::Mat* intrinsicMatrix,
                                               const cv::Mat* distortionCoeffs,
                                               const cv::Mat* inverseCoeffs,
                                               const cv::Mat* rotMat,
                                               const cv::Mat* trans,
                                               cv::Point2f*  offset);

    /**
      @brief Find chess board corners (sub-pixel accuracy) and return them in a useable format.
      @param view
      @param viewGrey
      @param boardSize
      @param draw
      @return pointer to new Matrix
    **/
    CvMat* FindChessBoard(IplImage* view,
                          const IplImage* viewGrey,
                          CvSize boardSize,
                          int draw=0);

    std::shared_ptr<cv::Mat> FindChessBoard(cv::Mat* view,
                                            const cv::Mat* viewGrey,
                                            cv::Size boardSize,
                                            int draw=0);
    /**
      @brief Find chess board corners (sub-pixel accuracy) and return them in a useable format.
      @param viewGrey
      @param boardSize
      @return pointer to new Matrix
    **/
    CvMat* FindChessBoard(IplImage* viewGrey, CvSize boardSize);
    std::shared_ptr<cv::Mat> FindChessBoard(cv::Mat* viewGrey, cv::Size boardSize);

    /**
      Given an alignment result from scan matching, apply
      the 2D transfortmation to the image so it will be
      aligned correctly for the composite ground-plane image.

      The translational units of pose (ScanPose::dx and ScanPose::dy)
      must be in pixels (e.g. convert them from cm to pixels using
      RobotMetrics::GetScaleFactor().)

      @param pose 2D-Pose transformation which aligns the ground plane to that of another camera.
      @param src The ground-plane image that is to be transformed/aligned.
      @param dst The composite image.
      @param imgOrigin Origin of the ground-plane coordinate system in the input image (in pixels)
      @param cmpOrigin Origin of the ground-plane coordinate system in the composite image (in pixels)
    **/
    void AlignGroundPlane(const CvMat* transform, const IplImage* src, IplImage* dst);
    void AlignGroundPlane(const cv::Mat* transform, const cv::Mat* src, cv::Mat* dst);

    /**
      @brief AlignGroundPlane
      @param transform
      @param src
      @param dst
      @param imgOrigin
      @param cmpOrigin
    **/
    void AlignGroundPlane(const CvMat* transform,
                          const IplImage* src,
                          IplImage* dst,
                          CvPoint2D32f imgOrigin,
                          CvPoint2D32f cmpOrigin);
    void AlignGroundPlane(const cv::Mat* transform,
                          const cv::Mat* src,
                          cv::Mat* dst,
                          cv::Point2f imgOrigin,
                          cv::Point2f cmpOrigin);

    void CompositeImageBoundingBox(const CvMat* transform, const IplImage* src, Rect32f* bbox);
    void CompositeImageBoundingBox(const cv::Mat* transform, const cv::Mat* src, NewRect32f *bbox);

    /**
      @brief Given the alignment result from a scan match, computes the image
      transformation that aligns src to dst, the new bounding box which
      contains both images and the position of the ground plane origin
      from dst in the composite image.

      The translational units of pose (ScanPose::dx and ScanPose::dy)
      must be in pixels (e.g. convert them from cm to pixels using
      RobotMetrics::GetScaleFactor().)

      @param[in] pose 2D-Pose transformation which aligns the ground-plane of src to the ground-plane of dst.
      @param[in] src The ground-plane image that is to be transformed/aligned.
      @param[in] srcOrigin Origin of the ground-plane coordinate system in the source image (in pixels).
      @param[in] dstOrigin Origin of the ground-plane coordinate system in the destination image (in pixels).
      @param[out] newOrigin Origin of the ground-plane in the composite image.
      @param[in,out] bbox Rectangular bounding box of composite image. Goes in as current bounding box; comes out as new one.
    **/
    void CompositeImageBoundingBox(const CvMat* transform,
                                   const IplImage* src,
                                   CvPoint2D32f srcOrigin,
                                   CvPoint2D32f dstOrigin,
                                   CvPoint2D32f* newOrigin,
                                   Rect32f* bbox);

    void CompositeImageBoundingBox(const cv::Mat* transform,
                                   const cv::Mat* src,
                                   cv::Point2f srcOrigin,
                                   cv::Point2f dstOrigin,
                                   cv::Point2f* newOrigin,
                                   NewRect32f* bbox);

    /**
      @brief Creates a composite ground-plane image.
      The two ground-plane images to be composited have already been
      transformed into src1 and src2 (which should both be the same size).

      @param src1 First ground plane image for composition
      @param src2 Second ground-plane image for composition
      @param dst  Resulting composite image. Must have already been allocated to same size as src1 and src2.
    **/
    void CreateCompositeImage(const IplImage* src1, const IplImage* src2, IplImage* dst);
    void CreateCompositeImage(const cv::Mat* src1, const cv::Mat* src2, cv::Mat* dst);
}

#endif // GROUND_PLANE_UTILITY_H
