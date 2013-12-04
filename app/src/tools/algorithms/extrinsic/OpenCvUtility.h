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

#ifndef OPENCV_UTILITY_H
#define OPENCV_UTILITY_H

#include <opencv/cv.h>

#include <stdio.h>
#include <memory>

namespace OpenCvUtility
{
    /**
      @brief Compute the bounding box of some points:
      returned as two points representing the corner
      of the bounding box with minimum coords (min),
      and the width and height of the bounding box (dim)
    **/
    void BoundingBox(int nPts, const CvPoint2D32f* pts, CvPoint2D32f* min, CvPoint2D32f* dim, float margin = 0.f);
    void BoundingBox(int nPts, const cv::Point2f* pts, cv::Point2f* min, cv::Point2f* dim, float margin = 0.f);

    /**
      @brief Apply a 2D translation to a 3x3 homogenous transform matrix.
      It is ok for in and out to be aliases for the same matrix.
    **/
    void TranslateCvMat2D(CvMat* in, CvMat* out, float tx, float ty);
    void TranslateCvMat2D(cv::Mat* in, cv::Mat* out, float tx, float ty);

    /**
      @brief Line intersection whcih returns the parametric intersection variables (s,t)
      such that the intersection point is given by:

      p = a1 + s(a2-a1) = b1 + t(b2-b1)

      @return True if the intersection is valid (lines not parallel).
    **/
    bool IntersectLines(CvPoint2D32f a1, CvPoint2D32f a2, CvPoint2D32f b1, CvPoint2D32f b2, double* s, double* t);

    /**
      @brief Intersect two lines (or line segments) specified by pairs of points a1->a2 and b1->b2.
      the intersection point is returned in ip
      @return true if the lines are not parallel (within precision), false otherwise.
    **/
    bool IntersectLines(CvPoint2D32f a1, CvPoint2D32f a2, CvPoint2D32f b1, CvPoint2D32f b2, CvPoint2D32f* ip);

    /**
      @brief Intersect a line segment with a closed polygon. If the line segment intersects
      the polygon in multiple locations it will return the intersection closest to
      the point a1 (but must be at least min_dist away from a1).

      If the return value is false then *ip is guaranteed not to be modifed by this function (certain usages rely on this).
      @param ip A point where the intersection will be stored.
      @return true if there was an intersection, false otherwise.
    **/
    bool IntersectLineSegmentWithPolygon(int nPts, const CvPoint2D32f* poly, CvPoint2D32f a1, CvPoint2D32f a2, CvPoint2D32f* ip);

    /**
      @brief Determine whether a point lies in a polygon.
      @a poly The vertices of the polygon. The polygon is assumed to be closed.
      @return true if the point is inside the polygon, false otherwise
    **/
    bool PointInPolygon(int nPts, const CvPoint2D32f* poly, CvPoint2D32f pt);

    /**
      @brief Compute a map which can be used with the cvRemap function to undistort
      the entire ground plane (removing effects of perspective and lens
      distortion).
    **/
    void ComputeGroundPlaneWarp(const CvMat* intrinsicMatrix,
                                const CvMat* rot,
                                const CvMat* trans,
                                const CvMat* distortion,
                                const CvPoint2D32f* offset,
                                CvMat* mapx,
                                CvMat* mapy);
    void ComputeGroundPlaneWarp(const cv::Mat*      intrinsicMatrix,
                                const cv::Mat*      rot,
                                const cv::Mat*      trans,
                                const cv::Mat*      distortion,
                                const cv::Point2f*  offset,
                                cv::Mat*            mapx,
                                cv::Mat*            mapy);

    /**
      @brief Computes a perspective warp suitable for use by the cvRemap function.
      The warp is specified by the homography H
      (H supplied must map points from the destination to the source i.e.
      it is usually the inverse homography if the homography was computed
      directly from a calibration image).
    **/
    void ComputePerspectiveWarp(const CvMat* H, CvMat* mapx, CvMat* mapy);
    void ComputePerspectiveWarp(const cv::Mat* H, cv::Mat* mapx, cv::Mat* mapy);

    /**
      @brief Projects points without radial distortion
    **/
    void ProjectPointsSimple(CvMat* intrinsicMatrix,
                             CvMat* rot,
                             CvMat* trans,
                             int cornerCount,
                             CvPoint3D32f* pObj3D,
                             CvPoint2D32f* pPts2D);

    void ProjectPointsSimple(cv::Mat* intrinsicMatrix,
                             cv::Mat* rot,
                             cv::Mat* trans,
                             int cornerCount,
                             cv::Point2f* pObj3D,
                             cv::Point2f* pPts2D);

    void LogCvMat32F(const CvMat* mat);
    void LogCvMat32F(const cv::Mat* mat);

    void PostTranslateCvMat2D(CvMat* in, CvMat* out, float tx, float ty);
    void PostTranslateCvMat2D(cv::Mat* in, cv::Mat* out, float tx, float ty);

    /**
      @brief Use a homography (H) to project the model points (pObject) into the image (pImg).
    **/
    void ApplyHomography(CvMat* H, unsigned int numPoints, CvPoint3D32f* pObject, CvPoint2D32f* pImg);
    void ApplyHomography(cv::Mat* H, unsigned int numPoints, cv::Point2f* pObject, cv::Point2f* pImg);

    /**
      @brief This function takes the cameras internal calibration parameters (intrinsic parameters)
      and the external rotation and translation parameters and extracts the homographic mapping
      of the z=0 plane.
    **/
    void ExtractCvHomography(const CvMat* intrinsic, const CvMat* R, const CvMat* T, CvMat* H);
    void ExtractCvHomography(const cv::Mat* intrinsic, const cv::Mat* R, const cv::Mat* T, cv::Mat* H);

    /**
      @brief Invert an intrinsic matix - the calibration must have no skew,
      i.e.:
      [fx 0 cx]-1     [1/fx 0 -cx/fx]
      [0 fy cy]    =  [0 1/fy -cy/fy]
      [0  0  1]       [0    0    1  ]
    **/
    void InvertCvIntrinsicMatrix(const CvMat* intrinsicMatrix, CvMat* inv);
    void InvertCvIntrinsicMatrix(const cv::Mat* intrinsicMatrix, cv::Mat* inv);

    /**
      @brief This returns the same result as if we called ExtractCvHomography and
      then computed the inverse transform of the result, but here we invert
      the calibration matrix seperately;
      It assumes that there are no skew terms in the calibration matrix.
    **/
    void ExtractInverseCvHomography(const CvMat* intrinsic, const CvMat* R, const CvMat* T, CvMat* H);
    void ExtractInverseCvHomography(const cv::Mat* intrinsic, const cv::Mat* R, const cv::Mat* T, cv::Mat* H);

    /**
      @brief Undo the full camera transform (assuming all image points lie in the ground plane).
      Takes the camera intrinsic parameters, inverse distortion parameters,
      plus the extrinsic camera parameters (rotation and translation of the camera).
      From all these it computes the inverse mapping and applies to the supplied points.
    **/
    void InvertGroundPlanePoints(const CvMat* inverseCoeffs,
                                 const CvMat* intrinsic,
                                 const CvMat* R,
                                 const CvMat* T,
                                 unsigned int numPoints,
                                 CvPoint3D32f* pObject,
                                 CvPoint2D32f* pImg);

    void InvertGroundPlanePoints(const cv::Mat* inverseCoeffs,
                                 const cv::Mat* intrinsic,
                                 const cv::Mat* R,
                                 const cv::Mat* T,
                                 unsigned int numPoints,
                                 cv::Point3f* pObject,
                                 cv::Point2f* pImg);

    CvPoint2D32f ImgToNorm(CvPoint2D32f p, const CvMat* intrinsic, const CvMat* distortion);
    cv::Point2f ImgToNorm(cv::Point2f p, const cv::Mat* intrinsic, const cv::Mat* distortion);

    CvPoint2D32f NormToImg(CvPoint2D32f p, const CvMat* intrinsic, const CvMat* distortion);
    cv::Point2f NormToImg(cv::Point2f p, const cv::Mat* intrinsic, const cv::Mat* distortion);

    CvPoint2D32f PlaneToImage(CvPoint2D32f p, const CvMat* intrinsic, const CvMat* distortion, const CvMat* Rot, const CvMat* Trans);
    cv::Point2f PlaneToImage(cv::Point2f p, const cv::Mat* intrinsic, const cv::Mat* distortion, const cv::Mat* Rot, const cv::Mat* Trans);

    CvPoint2D32f ImageToPlane(CvPoint2D32f p, const CvMat* intrinsic, const CvMat* distortion, const CvMat* Rot, const CvMat* Trans);
    cv::Point2f ImageToPlane(cv::Point2f p, const cv::Mat* intrinsic, const cv::Mat* distortion, const cv::Mat* Rot, const cv::Mat* Trans);

    /**
      @brief Visualise a single channel float matrix.
      Computes max and min value of float array, then
      linearly scales all values into range (0,255)
      for visualisation as grey level image.
    **/
    IplImage* VisualiseCvMatrix32fc1(const CvMat* mat);
    //std::shared_ptr<cv::Mat> VisualiseCvMatrix32fc1(const cv::Mat* mat);

    /**
      @brief Takes the log of all pixel values and then rescales result into range 0..255.
    **/
    IplImage* LogNormaliseCvImage(const IplImage* img);
    std::shared_ptr<cv::Mat> LogNormaliseCvImage(const  cv::Mat* img);

    /**
      @brief Simple two tap gradient from a single-channel floating point image
    **/
    CvMat* GradientMagCv32fc1(const CvMat* mat);
    std::shared_ptr<cv::Mat> GradientMagCv32fc1(const cv::Mat* mat);

    /**
      @brief Fill in an Image with raw bytes stored in pImgData.
      img must be allocated to the desired width, height,
      and number of channels. The function then assumes
      that there is enough data in pImgData to fill img.

      @param img A pointer to an allocatged OpenCV image to be filled with bytes.
      @param pImgData A pointer to bytes (unsigned chars) to be copied into img.
    **/
    void FillCvImageWithRawBytes(IplImage* img, const unsigned char* pImgData);
    void FillCvImageWithRawBytes(cv::Mat* img, const unsigned char* pImgData);


    /**
      @brief Extract image data into a simple array of unsigned chars.
      The destination array is assumed to be allocated to (at least) the correct size.

      @param img A pointer to the source OpenCV image.
      @param pImgData A pointer to bytes (unsigned chars) where data will be copied.
    **/
    void GetRawBytesFromCvImage(const IplImage* img, unsigned char* pImgData);
    void GetRawBytesFromCvImage(const cv::Mat* img, unsigned char* pImgData);

    void MouseCvMat32fc1Query(int event, int x, int y, void* params);

    /**
      @brief The motion filter counts the number of 'moving' pixels in a
      window around the current pixel.

      @param src Input motion-difference image.
      @param dst Result must be an IPL_DEPTH_32S image.
    **/
    void MotionFilter(const IplImage* src, IplImage* dst, int windowWidth, int windowHeight);
    void MotionFilter(const cv::Mat* src, cv::Mat* dst, int windowWidth, int windowHeight);
}

#endif // OPENCV_UTILITY_H
