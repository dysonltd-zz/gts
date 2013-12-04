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

#include "GroundPlaneUtility.h"
#include "OpenCvUtility.h"
#include "MathsConstants.h"
#include "Logging.h"

#include <opencv/highgui.h>

#include <memory>

namespace GroundPlaneUtility
{
    CvMat* CreateCalibrationObject(int width, int height, float squareSize)
    {
        CvMat* objectPoints = cvCreateMat(1, width*height, CV_32FC3);

        // Fill in calibration-object point data
        CvPoint3D32f* pObj3D = ((CvPoint3D32f*)objectPoints->data.fl);
        for (int j=0; j<height; ++j)
        {
            for(int k=0; k<width; ++k)
            {
                *pObj3D++ = cvPoint3D32f((height-j-1)*squareSize, (k)*squareSize, 0.f);
            }
        }

        return objectPoints;
    }

    /*
    Uncomment once CvMat variant is removed, otherwise compiler complains

    cv::Mat* CreateCalibrationObject(int width, int height, float squareSize)
    {
        cv::Mat* objectPoints = cv::Mat(1, width*height, CV_32FC3);

        // Fill in calibration-object point data
        CvPoint3D32f* pObj3D = ((CvPoint3D32f*)objectPoints->data);

        for (int j=0; j<height; ++j)
        {
            for(int k=0; k<width; ++k)
            {
                *pObj3D++ = cvPoint3D32f((height-j-1)*squareSize, (k)*squareSize, 0.f);
            }
        }

        return objectPoints;
    }
    */

    void ComputeExtrinsicParameters(const CvMat* objectPoints,
                                     const CvMat* imagePoints,
                                     const CvMat* intrinsicMatrix,
                                     const CvMat* distortionCoeffs,
                                     CvMat* rotMat,
                                     CvMat* trans)
    {
        CvMat* rot = cvCreateMat(1,3,CV_32FC1);
        cvFindExtrinsicCameraParams2(objectPoints,
                                      imagePoints,
                                      intrinsicMatrix,
                                      distortionCoeffs,
                                      rot,
                                      trans);
#ifndef NDEBUG
        LOG_INFO("Object points:");
        OpenCvUtility::LogCvMat32F(objectPoints);

        LOG_INFO("Image points:");
        OpenCvUtility::LogCvMat32F(imagePoints);
#endif
        cvRodrigues2(rot,rotMat);
    }

    void ComputeExtrinsicParameters(const cv::Mat*  objectPoints,
                                    const cv::Mat*  imagePoints,
                                    const cv::Mat*  intrinsicMatrix,
                                    const cv::Mat*  distortionCoeffs,
                                    cv::Mat*        rotMat,
                                    cv::Mat*        trans)
    {
        std::unique_ptr<cv::Mat> rot(new cv::Mat(1,3,CV_32FC1));

        cv::solvePnP(*objectPoints,
                     *imagePoints,
                     *intrinsicMatrix,
                     *distortionCoeffs,
                     *rot,
                     *trans);

#ifndef NDEBUG
        LOG_INFO("Object points:");
        OpenCvUtility::LogCvMat32F(objectPoints);

        LOG_INFO("Image points:");
        OpenCvUtility::LogCvMat32F(imagePoints);
#endif

        cv::Rodrigues(*rot, *rotMat);
    }

    IplImage* ComputeGroundPlaneWarpBatch(const IplImage* viewGrey,
                                          const CvMat*    intrinsicMatrix,
                                          const CvMat*    distortionCoeffs,
                                          const CvMat*    inverseCoeffs,
                                          const CvMat*    rotMat,
                                          const CvMat*    trans,
                                          CvMat**         mapx,
                                          CvMat**         mapy,
                                          CvPoint2D32f*   offset)
    {
        // To make an image of correct size we need to know the inverse transformation
        float H[9];
        CvMat homMat = cvMat(3,3, CV_32F, H);

        LOG_TRACE("Extracting homography");

        OpenCvUtility::ExtractCvHomography(intrinsicMatrix,rotMat,trans,&homMat);

        //float iH[9];
        //CvMat invHomMat = cvMat(3,3, CV_32F, iH);
        //extractInverseCvHomography(intrinsicMatrix,rotMat,trans,&invHomMat);

        // Compute where image corners end up (we want to see whole of undistorted image plane)
        float imgCorners[12] = { 0.f,
                                 0.f,
                                 1.f,
                                 (float)viewGrey->width,
                                 0.f,
                                 1.f,
                                 (float)viewGrey->width,
                                 (float)viewGrey->height,
                                 1.f,
                                 0.f,
                                 (float)viewGrey->height,
                                 1.f };
        CvPoint2D32f pos,dim;

        CvPoint2D32f* cornerBuffer = (CvPoint2D32f*)cvAlloc(4*sizeof(CvPoint2D32f));

        // applyHomography(&invHomMat,4,(CvPoint3D32f*)imgCorners,cornerBuffer);

        LOG_TRACE("Inverting projection");
        OpenCvUtility::InvertGroundPlanePoints(inverseCoeffs,
                                               intrinsicMatrix,
                                               rotMat,
                                               trans,
                                               4,
                                               (CvPoint3D32f*)imgCorners,
                                               cornerBuffer);

        LOG_TRACE("Computing bounding box");

        float margin = 100.f;
        OpenCvUtility::BoundingBox(4, cornerBuffer, &pos, &dim, margin); // compute bounding box + a margin
        *offset = pos;
        cvFree(&cornerBuffer);

        // allocate an image of correct size to exactly hold the warp
        int sizex = (int)(dim.x+.5f);
        int sizey = (int)(dim.y+.5f);

        // handle padding
        sizex += 4 -(sizex % 4);

        LOG_INFO(QObject::tr("Computing warp map (size=%1x%2) (%3x%4).").arg(sizex)
                                                                        .arg(sizey)
                                                                        .arg(dim.x)
                                                                        .arg(dim.y));

        assert(sizex>0 && sizey>0);
        IplImage* viewWarp = cvCreateImage(cvSize(sizex,sizey), IPL_DEPTH_8U, 1);

        // 'Undo' homography in image.
        *mapx = cvCreateMat(viewWarp->height, viewWarp->width,CV_32F);
        *mapy = cvCreateMat(viewWarp->height, viewWarp->width,CV_32F);
        OpenCvUtility::ComputeGroundPlaneWarp(intrinsicMatrix, rotMat, trans, distortionCoeffs, &pos, *mapx, *mapy);

        return viewWarp;
    }

    std::shared_ptr<cv::Mat> ComputeGroundPlaneWarpBatch(const cv::Mat* viewGrey,
                                                         const cv::Mat* intrinsicMatrix,
                                                         const cv::Mat* distortionCoeffs,
                                                         const cv::Mat* inverseCoeffs,
                                                         const cv::Mat* rotMat,
                                                         const cv::Mat* trans,
                                                         cv::Mat* mapx,
                                                         cv::Mat* mapy,
                                                         cv::Point2f *offset)
    {
        // To make an image of correct size we need to know the inverse transformation
        float H[9];
        std::shared_ptr<cv::Mat> homMat(new cv::Mat(3, 3, CV_32F, H));

        LOG_TRACE("Extracting homography");
        OpenCvUtility::ExtractCvHomography(intrinsicMatrix, rotMat, trans, homMat.get());

        // Compute where image corners end up (we want to see whole of undistorted image plane)
        float imgCorners[12] = { 0.f,
                                 0.f,
                                 1.f,
                                 (float)viewGrey->size().width,
                                 0.f,
                                 1.f,
                                 (float)viewGrey->size().width,
                                 (float)viewGrey->size().height,
                                 1.f,
                                 0.f,
                                 (float)viewGrey->size().height,
                                 1.f };
        cv::Point2f pos, dim, cornerBuffer;

        LOG_TRACE("Inverting projection");
        OpenCvUtility::InvertGroundPlanePoints(inverseCoeffs,
                                               intrinsicMatrix,
                                               rotMat,
                                               trans,
                                               4,
                                               (cv::Point3f*)&imgCorners,
                                               &cornerBuffer);

        LOG_TRACE("Computing bounding box");
        float margin = 100.f;
        OpenCvUtility::BoundingBox(4, &cornerBuffer, &pos, &dim, margin); // compute bounding box + a margin
        *offset = pos;

        // allocate an image of correct size to exactly hold the warp
        int sizex = (int)(dim.x+.5f);
        int sizey = (int)(dim.y+.5f);

        // handle padding
        sizex += 4 -(sizex % 4);

        LOG_INFO(QObject::tr("Computing warp map (size=%1x%2) (%3x%4).").arg(sizex)
                                                                        .arg(sizey)
                                                                        .arg(dim.x)
                                                                        .arg(dim.y));

        assert(sizex>0 && sizey>0);
        std::shared_ptr<cv::Mat> viewWarp(new cv::Mat(cv::Size(sizex,sizey), IPL_DEPTH_8U, 1));

        // 'Undo' homography in image.
        mapx = new cv::Mat(viewWarp.get()->size().height, viewWarp.get()->size().width, CV_32F);
        mapy = new cv::Mat(viewWarp.get()->size().height, viewWarp.get()->size().width, CV_32F);
        OpenCvUtility::ComputeGroundPlaneWarp(intrinsicMatrix, rotMat, trans, distortionCoeffs, &pos, mapx, mapy);

        return viewWarp;
    }

    IplImage* UnwarpGroundPlane(const IplImage*  viewGrey,
                                const CvMat*     intrinsicMatrix,
                                const CvMat*     distortionCoeffs,
                                const CvMat*     inverseCoeffs,
                                const CvMat*     rotMat,
                                const CvMat*     trans,
                                CvPoint2D32f*    offset)
    {
        CvMat* mapx;
        CvMat* mapy;
        IplImage* viewWarp = ComputeGroundPlaneWarpBatch(viewGrey,
                                                          intrinsicMatrix,
                                                          distortionCoeffs,
                                                          inverseCoeffs,
                                                          rotMat,
                                                          trans,
                                                          &mapx,
                                                          &mapy,
                                                          offset);
        cvSetZero(viewWarp);
        cvRemap(viewGrey, viewWarp, mapx, mapy, CV_INTER_LINEAR);

        cvReleaseMat(&mapx);
        cvReleaseMat(&mapy);

        return viewWarp;
    }

    std::shared_ptr<cv::Mat> UnwarpGroundPlane(const cv::Mat *viewGrey,
                                               const cv::Mat *intrinsicMatrix,
                                               const cv::Mat *distortionCoeffs,
                                               const cv::Mat *inverseCoeffs,
                                               const cv::Mat *rotMat,
                                               const cv::Mat *trans,
                                               cv::Point2f *offset)
    {
        cv::Mat mapx = cv::Mat();
        cv::Mat mapy = cv::Mat();

        std::shared_ptr<cv::Mat> viewWarp = ComputeGroundPlaneWarpBatch(viewGrey,
                                                                        intrinsicMatrix,
                                                                        distortionCoeffs,
                                                                        inverseCoeffs,
                                                                        rotMat,
                                                                        trans,
                                                                        &mapx,
                                                                        &mapy,
                                                                        offset);

        viewWarp->zeros(viewWarp->size().height, viewWarp->size().width, CV_32F);
        cv::remap(*viewGrey, *viewWarp, mapx, mapy, CV_INTER_LINEAR);
        return viewWarp;
    }

    CvMat* FindChessBoard(IplImage* view, const IplImage* viewGrey, CvSize boardSize, int draw)
    {
        int ptsSize;
        int cornerCount;
        int found;
        ptsSize = boardSize.width*boardSize.height * sizeof(CvPoint2D32f);
        CvPoint2D32f* cornerBuffer = (CvPoint2D32f*)cvAlloc(ptsSize);
        found = cvFindChessboardCorners(viewGrey, boardSize, cornerBuffer, &cornerCount, CV_CALIB_CB_ADAPTIVE_THRESH);

        if (boardSize.width*boardSize.height != cornerCount)
        {
            LOG_ERROR(QObject::tr("Only detected %1/%2 corners!").arg(cornerCount)
                                                                 .arg(boardSize.width*boardSize.height));
            return 0;
        }

        // Improve accuracy of corner detections
        cvFindCornerSubPix(viewGrey,
                            cornerBuffer,
                            cornerCount,
                            cvSize(11,11),
                            cvSize(-1,-1),
                            cvTermCriteria(CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 30, 0.1));

        CvMat* imagePoints = cvCreateMat(1, cornerCount, CV_32FC2);

        // Copy detected corners into correct format for cvFindExtrinsicCameraParams2
        CvPoint2D32f* pPts2D = ((CvPoint2D32f*)imagePoints->data.fl);
        for (int p=0; p<cornerCount; ++p)
        {
            *pPts2D++ = cvPoint2D32f(cornerBuffer[p].x,cornerBuffer[p].y);
        }

        if (draw)
        {
            cvDrawChessboardCorners(view, boardSize, cornerBuffer, cornerCount, found);
        }

        delete cornerBuffer;

        return imagePoints;
    }

    std::shared_ptr<cv::Mat> FindChessBoard(cv::Mat* view, const cv::Mat* viewGrey, cv::Size boardSize, int draw)
    {
        int found;
        std::vector<cv::Point2f> cornerBuffer;

        found = cv::findChessboardCorners(*viewGrey, boardSize, cornerBuffer, CV_CALIB_CB_ADAPTIVE_THRESH);
        const unsigned int cornerCount = cornerBuffer.size();

        if ((unsigned)boardSize.width*boardSize.height != cornerCount)
        {
            LOG_ERROR(QObject::tr("Only detected %1/%2 corners!").arg(cornerBuffer.size())
                                                                 .arg(boardSize.width*boardSize.height));
            return 0;
        }

        // Improve accuracy of corner detections
        cv::cornerSubPix(*viewGrey,
                         cornerBuffer,
                         cv::Size(11,11),
                         cv::Size(-1,-1),
                         cvTermCriteria(CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 30, 0.1));

        std::shared_ptr<cv::Mat> imagePoints(new cv::Mat(1, cornerCount, CV_32FC2));

        // Copy detected corners into correct format for cvFindExtrinsicCameraParams2
        CvPoint2D32f* pPts2D = ((CvPoint2D32f*)imagePoints->ptr<float>(0));
        for (unsigned int p=0; p<cornerCount; ++p)
        {
            *pPts2D++ = cv::Point2f(cornerBuffer[p].x,cornerBuffer[p].y);
        }

        if (draw)
        {
            cv::drawChessboardCorners(*view, boardSize, cornerBuffer, found);
        }

        return imagePoints;
    }

    CvMat* FindChessBoard(IplImage* viewGrey, CvSize boardSize)
    {
        const int dontDraw = 0;
        return FindChessBoard(viewGrey, viewGrey, boardSize, dontDraw);
    }

    std::shared_ptr<cv::Mat> FindChessBoard(cv::Mat* viewGrey, cv::Size boardSize)
    {
        const int dontDraw = 0;
        return FindChessBoard(viewGrey, viewGrey, boardSize, dontDraw);
    }

    void AlignGroundPlane(const CvMat* transform, const IplImage* src, IplImage* dst)
    {
        assert(src);
        assert(dst);

#ifdef AFFINE_TRANSFORM
        float A[6];
        CvMat affine = cvMat(2, 3, CV_32F, A);

        A[0] = cvmGet(transform,0,0);
        A[1] = cvmGet(transform,0,1);
        A[2] = cvmGet(transform,0,2);
        A[3] = cvmGet(transform,1,0);
        A[4] = cvmGet(transform,1,1);
        A[5] = cvmGet(transform,1,2);

        cvWarpAffine(src, dst, &affine);
#else
        cvWarpPerspective(src, dst, transform);
#endif
    }

    void AlignGroundPlane(const cv::Mat* transform, const cv::Mat* src, cv::Mat* dst)
    {
        if (!transform->data || !src->data || !dst->data)
        {
            LOG_TRACE("Ground plane unabl to be warped");
            return;
        }

        unsigned int rows = 2;
        unsigned int columns = 3;

#ifdef AFFINE_TRANSFORM
        float A[6];
        cv::Mat affine = cv::Mat(rows, columns, CV_32F, A);

        A[0] = transform->at<float>(0,0);
        A[1] = transform->at<float>(0,1);
        A[2] = transform->at<float>(0,2);
        A[3] = transform->at<float>(1,0);
        A[4] = transform->at<float>(1,1);
        A[5] = transform->at<float>(1,2);

        cv::warpAffine(src, dst, &affine);
#else
        cv::warpPerspective(*src, *dst, *transform, cv::Size(rows, columns));
#endif
    }

    void AlignGroundPlane(const CvMat*    transform,
                          const IplImage* src,
                          IplImage*       dst,
                          CvPoint2D32f    imgOrigin,
                          CvPoint2D32f    cmpOrigin)
    {
        assert(src);
        assert(dst);

        float A[9];
        CvMat transf = cvMat(3, 3, CV_32F, A);
        A[0] = cvmGet(transform,0,0);
        A[1] = cvmGet(transform,0,1);
        A[2] = cvmGet(transform,0,2);
        A[3] = cvmGet(transform,1,0);
        A[4] = cvmGet(transform,1,1);
        A[5] = cvmGet(transform,1,2);
        A[6] = cvmGet(transform,2,0);
        A[7] = cvmGet(transform,2,1);
        A[8] = cvmGet(transform,2,2);

        float C[9];
        CvMat transl = cvMat(3, 3, CV_32F, C);
        C[0] = 1.0;
        C[1] = 0.0;
        C[2] = cmpOrigin.x - imgOrigin.x;
        C[3] = 0.0;
        C[4] = 1.0;
        C[5] = cmpOrigin.y - imgOrigin.y;
        C[6] = 0.0;
        C[7] = 0.0;
        C[8] = 1.0;

        float X[9];
        CvMat x = cvMat(3, 3, CV_32F, X);

        cvMatMul(&transl, &transf, &x);

        cvWarpPerspective(src, dst, &x);
    }

    void AlignGroundPlane(const cv::Mat* transform,
                          const cv::Mat* src,
                          cv::Mat* dst,
                          cv::Point2f imgOrigin,
                          cv::Point2f cmpOrigin)
    {
        if (!transform->data || !src->data || !dst->data)
        {
            LOG_ERROR("Ground plane could not be aligned");
            return;
        }

        unsigned int rows = 3;
        unsigned int columns = 3;

        float A[9];
        cv::Mat transf = cv::Mat(rows, columns, CV_32F, A);
        A[0] = transform->at<float>(0,0);
        A[1] = transform->at<float>(0,1);
        A[2] = transform->at<float>(0,2);
        A[3] = transform->at<float>(1,0);
        A[4] = transform->at<float>(1,1);
        A[5] = transform->at<float>(1,2);
        A[6] = transform->at<float>(2,0);
        A[7] = transform->at<float>(2,1);
        A[8] = transform->at<float>(2,2);

        float C[9];
        cv::Mat transl = cv::Mat(rows, columns, CV_32F, C);
        C[0] = 1.0;
        C[1] = 0.0;
        C[2] = cmpOrigin.x - imgOrigin.x;
        C[3] = 0.0;
        C[4] = 1.0;
        C[5] = cmpOrigin.y - imgOrigin.y;
        C[6] = 0.0;
        C[7] = 0.0;
        C[8] = 1.0;

        float X[9];
        cv::Mat x = cv::Mat(rows, columns, CV_32F, X);

        x = transl * transf;
        cv::warpPerspective(*src, *dst, x, cv::Size(rows, columns));
    }

    void CompositeImageBoundingBox(const CvMat* transform,
                                    const IplImage* src,
                                    Rect32f* bbox)
    {
        assert(src);

        // transform corners of source image:
        CvPoint2D32f inVerts[4] = { cvPoint2D32f(0,          0),
                                    cvPoint2D32f(0,          src->height),
                                    cvPoint2D32f(src->width, src->height),
                                    cvPoint2D32f(src->width, 0) };

        CvPoint2D32f outVerts[8];

#ifdef AFFINE_TRANSFORM
        CvMat tsrc = cvMat(1, 4, CV_32FC3, inVerts);
        CvMat tdst = cvMat(1, 8, CV_32FC2, outVerts);

        float A[6];
        CvMat affine = cvMat(2, 3, CV_32F, A);
        A[0] = cvmGet(transform,0,0);
        A[1] = cvmGet(transform,0,1);
        A[2] = cvmGet(transform,0,2);
        A[3] = cvmGet(transform,1,0);
        A[4] = cvmGet(transform,1,1);
        A[5] = cvmGet(transform,1,2);

        cvTransform(&tsrc, &tdst, &affine);
#else
        for (unsigned int i=0; i<4; ++i)
        {
            CvScalar p1;
            CvMat tsrc, tdst;
            CvMat* src = cvCreateMat(2, 1, CV_32F);
            CvMat* dst = cvCreateMat(2, 1, CV_32F);

            cvSet2D(src,0,0,cvScalar(inVerts[i].x));
            cvSet2D(src,1,0,cvScalar(inVerts[i].y));
            cvReshape(src, &tsrc, 2, 0);
            cvReshape(dst, &tdst, 2, 0);

            cvPerspectiveTransform(&tsrc, &tdst, transform);

            p1=cvGet2D(&tdst,0,0);
            outVerts[i].x=p1.val[0];
            outVerts[i].y=p1.val[1];

            cvReleaseMat(&src);
            cvReleaseMat(&dst);
        }
#endif

        outVerts[4] = cvPoint2D32f(bbox->pos.x,             bbox->pos.y);
        outVerts[5] = cvPoint2D32f(bbox->pos.x,             bbox->pos.y+bbox->dim.y);
        outVerts[6] = cvPoint2D32f(bbox->pos.x+bbox->dim.x, bbox->pos.y+bbox->dim.y);
        outVerts[7] = cvPoint2D32f(bbox->pos.x+bbox->dim.x, bbox->pos.y);

        // compute bounding box
        OpenCvUtility::BoundingBox(8, outVerts, &bbox->pos, &bbox->dim);
    }

    void CompositeImageBoundingBox(const cv::Mat* transform,
                                   const cv::Mat* src,
                                   NewRect32f* bbox)
    {
        if (!src->data || !transform->data)
        {
            LOG_ERROR("CompositeImageBoundingBox error");
            return;
        }

        // transform corners of source image:
        CvPoint2D32f inVerts[4] = { cv::Point2f(0,          0),
                                    cv::Point2f(0,          src->size().height),
                                    cv::Point2f(src->size().width, src->size().height),
                                    cv::Point2f(src->size().width, 0) };

        cv::Point2f outVerts[8];

#ifdef AFFINE_TRANSFORM
        cv::Mat tsrc = cv::Mat(1, 4, CV_32FC3, inVerts);
        cv::Mat tdst = cv::Mat(1, 8, CV_32FC2, outVerts);

        float A[6];
        cv::Mat affine = cv::Mat(2, 3, CV_32F, A);
        A[0] = transform->at<float>(0,0);
        A[1] = transform->at<float>(0,1);
        A[2] = transform->at<float>(0,2);
        A[3] = transform->at<float>(,1,0);
        A[4] = transform->at<float>(1,1);
        A[5] = transform->at<float>(1,2);

        cv::transform(&tsrc, &tdst, &affine);
#else
        for (unsigned int i=0; i<4; ++i)
        {
            cv::Scalar p1;
            cv::Mat tSource, tDestination;
            cv::Mat source = cv::Mat(2, 1, CV_32F);
            cv::Mat destination = cv::Mat(2, 1, CV_32F);

            source.at<double>(0,0) = inVerts[i].x;
            source.at<double>(1,0) = inVerts[i].y;

            tSource = source.reshape(2,0);
            tDestination = destination.reshape(2,0);

            cv::perspectiveTransform(tSource, tDestination, *transform);

            p1 = tDestination.at<cv::Scalar>(0,0);
            outVerts[i].x = p1.val[0];
            outVerts[i].y = p1.val[1];
        }
#endif

        outVerts[4] = cv::Point2f(bbox->pos.x,             bbox->pos.y);
        outVerts[5] = cv::Point2f(bbox->pos.x,             bbox->pos.y+bbox->dim.y);
        outVerts[6] = cv::Point2f(bbox->pos.x+bbox->dim.x, bbox->pos.y+bbox->dim.y);
        outVerts[7] = cv::Point2f(bbox->pos.x+bbox->dim.x, bbox->pos.y);

        // compute bounding box
        OpenCvUtility::BoundingBox(8, outVerts, &bbox->pos, &bbox->dim);
    }

    void CompositeImageBoundingBox(const CvMat*    transform,
                                   const IplImage* src,
                                   CvPoint2D32f    srcOrigin,
                                   CvPoint2D32f    dstOrigin,
                                   CvPoint2D32f*   newOrigin,
                                   Rect32f*        bbox)
    {
        Q_UNUSED(srcOrigin);

        // transform corners of source image:
        CvPoint2D32f inVerts[4] = { cvPoint2D32f(0,          0),
                                    cvPoint2D32f(0,          src->height),
                                    cvPoint2D32f(src->width, src->height),
                                    cvPoint2D32f(src->width, 0) };

        CvPoint2D32f outVerts[8];

printf("INVERTS: %f,%f %f,%f %f,%f %f,%f\n", inVerts[0].x,inVerts[0].y,
                                             inVerts[1].x,inVerts[1].y,
                                             inVerts[2].x,inVerts[2].y,
                                             inVerts[3].x,inVerts[3].y);

        for (unsigned int i=0; i<4; ++i)
        {
            CvScalar p1;
            CvMat tsrc, tdst;
            CvMat* src = cvCreateMat(2, 1, CV_32F);
            CvMat* dst = cvCreateMat(2, 1, CV_32F);

            cvSet2D(src,0,0,cvScalar(inVerts[i].x));
            cvSet2D(src,1,0,cvScalar(inVerts[i].y));
            cvReshape(src, &tsrc, 2, 0);
            cvReshape(dst, &tdst, 2, 0);

            cvPerspectiveTransform(&tsrc, &tdst, transform);

            p1=cvGet2D(&tdst,0,0);
            outVerts[i].x=p1.val[0];
            outVerts[i].y=p1.val[1];

            cvReleaseMat(&src);
            cvReleaseMat(&dst);
        }

        outVerts[4] = cvPoint2D32f(bbox->pos.x,             bbox->pos.y);
        outVerts[5] = cvPoint2D32f(bbox->pos.x,             bbox->pos.y+bbox->dim.y);
        outVerts[6] = cvPoint2D32f(bbox->pos.x+bbox->dim.x, bbox->pos.y+bbox->dim.y);
        outVerts[7] = cvPoint2D32f(bbox->pos.x+bbox->dim.x, bbox->pos.y);
        printf("OUTVERTS: %f,%f %f,%f %f,%f %f,%f\n", outVerts[0].x,outVerts[0].y,
                                                      outVerts[1].x,outVerts[1].y,
                                                      outVerts[2].x,outVerts[2].y,
                                                      outVerts[3].x,outVerts[3].y);
        printf("OUTVERTS: %f,%f %f,%f %f,%f %f,%f\n", outVerts[4].x,outVerts[4].y,
                                                      outVerts[5].x,outVerts[5].y,
                                                      outVerts[6].x,outVerts[6].y,
                                                      outVerts[7].x,outVerts[7].y);
        printf("\n\n");
        // compute bounding box
        OpenCvUtility::BoundingBox(8, outVerts, &bbox->pos, &bbox->dim);

        // compute new origin
        newOrigin->x = dstOrigin.x - bbox->pos.x;
        newOrigin->y = dstOrigin.y - bbox->pos.y;
    }

    void CompositeImageBoundingBox(const cv::Mat* transform,
                                   const cv::Mat* src,
                                   cv::Point2f srcOrigin,
                                   cv::Point2f dstOrigin,
                                   cv::Point2f* newOrigin,
                                   NewRect32f* bbox)
    {
        Q_UNUSED(srcOrigin);

        // transform corners of source image:
        cv::Point2f inVerts[4] =  { cv::Point2f(0, 0),
                                    cv::Point2f(0, src->size().height),
                                    cv::Point2f(src->size().width, src->size().height),
                                    cv::Point2f(src->size().width, 0) };

        cv::Point2f outVerts[8];

        for (unsigned int i=0; i<4; ++i)
        {
            cv::Scalar p1;
            cv::Mat tSource, tDestination;
            cv::Mat source = cv::Mat(2, 1, CV_32F);
            cv::Mat destination = cv::Mat(2, 1, CV_32F);

            source.at<double>(0,0) = inVerts[i].x;
            source.at<double>(1,0) = inVerts[i].y;

            tSource = source.reshape(2,0);
            tDestination = destination.reshape(2,0);

            cv::perspectiveTransform(tSource, tDestination, *transform);

            p1 = tDestination.at<cv::Scalar>(0,0);
            outVerts[i].x = p1.val[0];
            outVerts[i].y = p1.val[1];
        }

        outVerts[4] = cv::Point2f(bbox->pos.x,             bbox->pos.y);
        outVerts[5] = cv::Point2f(bbox->pos.x,             bbox->pos.y+bbox->dim.y);
        outVerts[6] = cv::Point2f(bbox->pos.x+bbox->dim.x, bbox->pos.y+bbox->dim.y);
        outVerts[7] = cv::Point2f(bbox->pos.x+bbox->dim.x, bbox->pos.y);

        // compute bounding box
        OpenCvUtility::BoundingBox(8, outVerts, &bbox->pos, &bbox->dim);

        // compute new origin
        newOrigin->x = dstOrigin.x - bbox->pos.x;
        newOrigin->y = dstOrigin.y - bbox->pos.y;
    }

    void CreateCompositeImage(const IplImage* src1, const IplImage* src2, IplImage* dst)
    {
        assert(src1->width == src2->width);
        assert(src2->width == dst->width);
        assert(src1->height == src2->height);
        assert(src2->height == dst->height);

        //int c = src1->nChannels;
        int w = src1->width;
        int h = src1->height;

        for (int i=0; i<h; ++i)
        {
            for (int j=0; j<w; ++j)
            {
                double v1 = cvGet2D(src1, i,j).val[0];
                double v2 = cvGet2D(src2, i,j).val[0];
                double val;

                if (v2 > v1)
                {
                    val = v2;
                }
                else
                {
                    val = v1;
                }

                cvSet2D(dst, i, j, cvScalar(val));
            }
        }
    }

    void CreateCompositeImage(const cv::Mat* src1, const cv::Mat* src2, cv::Mat* dst)
    {
        assert(src1->size().width == src2->size().width);
        assert(src2->size().width == dst->size().width);
        assert(src1->size().height == src2->size().height);
        assert(src2->size().height == dst->size().height);

        //int c = src1->nChannels;
        int w = src1->size().width;
        int h = src1->size().height;

        for (int i=0; i<h; ++i)
        {
            for (int j=0; j<w; ++j)
            {
                double v1 = src1->at<double>(i,j);
                double v2 = src2->at<double>(i,j);
                double val;

                if (v2 > v1)
                {
                    val = v2;
                }
                else
                {
                    val = v1;
                }

                dst->at<double>(i,j) = val;
            }
        }
    }
}
