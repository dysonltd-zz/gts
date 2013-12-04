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

#include "CameraCalibration.h"

#include "FloorPlanSchema.h"
#include "CalibrationSchema.h"

#include "RobotMetrics.h"

#include "OpenCvUtility.h"
#include "GroundPlaneUtility.h"

#include "Logging.h"

#include <opencv/highgui.h>
#include <opencv/cvaux.h>

#include <cassert>

struct CalibViewArgs
{
    CalibViewArgs(const CameraCalibration* cal, const RobotMetrics& met) :
        m_cal (cal),
        m_met (met)
    {
    };

    const CameraCalibration* m_cal;
    const RobotMetrics& m_met;
};

CameraCalibration::CameraCalibration() :
    m_mapx         (0),
    m_mapy         (0),
    m_mapDx        (0),
    m_mapDy        (0),
    m_mapGM        (0),
    m_imageWidth   (0),
    m_imageHeight  (0),
    m_offset       (cvPoint2D32f(0.f,0.f)),
    m_rot          (cvMat(3,3,CV_32F, m_rot_f)),
    m_trans        (cvMat(3,1,CV_32F, m_trans_f)),
    m_cameraCentre (cvMat(3,1,CV_32F, m_cC_f)),
    m_intrinsic    (cvMat(3,3,CV_32F, m_intrinsic_f)),
    m_distortion   (cvMat(1,5,CV_32F, m_distortion_f)),
    m_inverse      (cvMat(1,5,CV_32F, m_inverse_f)),
    m_transform    (cvMat(3,3,CV_32F, m_transform_f)),
    m_calWarpImg   (0)
{
}

CameraCalibration::~CameraCalibration()
{
    cvReleaseMat(&m_mapx);
    cvReleaseMat(&m_mapy);
    cvReleaseMat(&m_mapDx);
    cvReleaseMat(&m_mapDy);
    cvReleaseMat(&m_mapGM);

    cvReleaseImage(&m_calWarpImg);
}

/**
    Load the intrinsic camera parameters from config.
**/
bool CameraCalibration::LoadIntrinsicCalibration(const WbConfig& cameraCalCfg)
{
    bool successful = true;

    m_imageWidth = cameraCalCfg.GetKeyValue(
                       CalibrationSchema::imageWidthKey).ToInt();
    m_imageHeight = cameraCalCfg.GetKeyValue(
                       CalibrationSchema::imageHeightKey).ToInt();

    if (successful)
    {
        successful = cameraCalCfg.GetKeyValue(
                       CalibrationSchema::cameraMatrixKey).ToCvMat(m_intrinsic);
    }

    if (successful)
    {
        successful = cameraCalCfg.GetKeyValue(
                       CalibrationSchema::distortionCoefficientsKey).ToCvMat(m_distortion);
    }

    if (successful)
    {
        successful = cameraCalCfg.GetKeyValue(
                       CalibrationSchema::invDistortionCoefficientsKey).ToCvMat(m_inverse);
    }

    if (successful)
    {
        LOG_INFO(QObject::tr("Image dimensions (WxH): %1,%2").arg(m_imageWidth).
                                                              arg(m_imageHeight));

        LOG_INFO("Camera matrix:");
        OpenCvUtility::LogCvMat32F(&m_intrinsic);

        LOG_INFO("Distortion coefficients:");
        OpenCvUtility::LogCvMat32F(&m_distortion);

        LOG_INFO("Inverse coefficients:");
        OpenCvUtility::LogCvMat32F(&m_inverse);
    }

    return successful;
}

/**
    Load the camera transform parameters from config.
**/
bool CameraCalibration::LoadCameraTransform(const KeyId camPosId, const WbConfig& floorPlanCfg)
{
    bool successful = false;

    const WbKeyValues::ValueIdPairList cameraTransformIds =
        floorPlanCfg.GetKeyValues(FloorPlanSchema::transformKey);
    for (WbKeyValues::ValueIdPairList::const_iterator itt = cameraTransformIds.begin(); itt != cameraTransformIds.end(); ++itt)
    {
        const KeyId cameraId(floorPlanCfg.GetKeyValue(FloorPlanSchema::cameraIdKey, itt->id).ToKeyId());

        if (cameraId == camPosId)
        {
            LOG_INFO(QObject::tr("Camera position id: %1").arg(camPosId));

            float transf_f[9];
            CvMat transf = cvMat(3, 3, CV_32F, transf_f);

            successful = floorPlanCfg.GetKeyValue(
                             FloorPlanSchema::transformKey, itt->id).ToCvMat(transf);

            LOG_INFO("Camera transform:");
            OpenCvUtility::LogCvMat32F(&transf);

            double offsetX = floorPlanCfg.GetKeyValue(
                                 FloorPlanSchema::offsetXKey, itt->id).ToDouble();
            double offsetY = floorPlanCfg.GetKeyValue(
                                 FloorPlanSchema::offsetYKey, itt->id).ToDouble();

            float transl_f[9];
            CvMat transl = cvMat(3, 3, CV_32F, transl_f);
            cvSetIdentity(&transl);
            transl_f[2] = offsetX;
            transl_f[5] = offsetY;

            cvMatMul(&transl, &transf, &m_transform);

            LOG_INFO("With offsets:");
            OpenCvUtility::LogCvMat32F(&m_transform);

            break;
        }
    }

    return successful;
}

/**
    Computes the external calibration parameters (rotation and translation) for a camera.
    @param boardSize The size of the chequer board calibration target
    @param metrics   Parameters describing dimensions of robot
    @param viewWarp  Pointer to the image to use for calibration
    @param interactive If it is true then the calibration will be interactive displaying images and waiting for key-presses before continuing.
**/
bool CameraCalibration::PerformExtrinsicCalibration(CvSize        boardSize,
                                                     RobotMetrics& metrics,
                                                     IplImage**    viewWarp,
                                                     const bool    scaled,
                                                     const char*   calImage )
{
    bool success = true;
    IplImage* view = cvLoadImage(calImage, 1);

    if (view)
    {
        LOG_INFO(QObject::tr("Opened calibration image: %1.")
                    .arg(calImage));

        // Make a grey-level version of the image and show it
        IplImage* viewGrey = cvCreateImage(cvGetSize(view), IPL_DEPTH_8U, 1);
        cvCvtColor(view, viewGrey, CV_BGR2GRAY);

        CvMat* imagePoints = GroundPlaneUtility::FindChessBoard(view, viewGrey, boardSize, 1);

        if (imagePoints)
        {
            LOG_INFO(QObject::tr("Found %1/%2 calibration corners.")
                        .arg(imagePoints->cols)
                        .arg(boardSize.width*boardSize.height));

#ifdef SCALED_PIXELS
            float squarePx = ComputeSquareSize(imagePoints);
            metrics->ComputePixelMetrics(squarePx);
#endif

            // Compute camera rotation and translation (extrinsic calibration)
            CvMat* objectPoints;
            if (scaled)
            {
                objectPoints = GroundPlaneUtility::CreateCalibrationObject(boardSize.width,
                                                                            boardSize.height,
#ifdef SCALED_PIXELS
                                                                            squarePx * metrics->GetResolution());
#else
                                                                            metrics.GetSquareSizePx());
#endif
            }
            else
            {
                objectPoints = GroundPlaneUtility::CreateCalibrationObject(boardSize.width,
                                                                            boardSize.height,
                                                                            metrics.GetSquareSizeCm());
            }

            LOG_TRACE("Computing extrinsic params");
            ComputeExtrinsicParams(objectPoints, imagePoints);

            LOG_INFO("Camera rotation:");
            OpenCvUtility::LogCvMat32F(&m_rot);

            LOG_INFO("Camera translation:");
            OpenCvUtility::LogCvMat32F(&m_trans);

            LOG_TRACE("Computing warp");

            // Compute undistortion maps for the ground plane
            // (these will be used to undistort entire sequence)
            *viewWarp = GroundPlaneUtility::ComputeGroundPlaneWarpBatch(viewGrey,
                                                                         GetIntrinsicParams(),
                                                                         GetDistortionParams(),
                                                                         GetUndistortionParams(),
                                                                         GetRotationParams(),
                                                                         GetTranslationParams(),
                                                                         &m_mapx,
                                                                         &m_mapy,
                                                                         &m_offset);

            LOG_TRACE("Warping ground plane image");

            cvSetZero(*viewWarp);
            cvRemap(viewGrey, *viewWarp, m_mapx, m_mapy, CV_INTER_LINEAR);

            CalibViewArgs args(this, metrics);
            PlotCameraCentre(*viewWarp, metrics);
            m_calWarpImg = cvCloneImage(*viewWarp);

            // Compute the warp gradients
            ComputeWarpGradientMagnitude();

            cvReleaseMat(&objectPoints);
        }
        else
        {
            success = false;
        }

        cvReleaseMat(&imagePoints);
        cvReleaseImage(&viewGrey);
    }
    else
    {
        LOG_ERROR(QObject::tr("Could not open calibration image '%s'!").arg(calImage));

        success = false;
    }

    cvReleaseImage(&view);

    return success;
}

float CameraCalibration::ComputeSquareSize(CvMat* imagePoints)
{
    CvPoint2D32f* pPts2D = ((CvPoint2D32f*)imagePoints->data.fl);

    CvPoint2D32f p1 = *pPts2D++;
    CvPoint2D32f p2 = *pPts2D++;

    double x = p1.x - p2.x;
    double y = p1.y - p2.y;

    return sqrt(x*x + y*y);
}

void CameraCalibration::ComputeWarpGradientMagnitude()
{
    m_mapDx = OpenCvUtility::GradientMagCv32fc1(m_mapx);
    m_mapDy = OpenCvUtility::GradientMagCv32fc1(m_mapy);
    m_mapGM = cvCloneMat(m_mapDy);

    cvAdd(m_mapDx, m_mapDy, m_mapGM);
}

void CameraCalibration::ComputeExtrinsicParams(const CvMat* objectPoints,
                                                const CvMat* imagePoints)
{
    // Compute parameters and convert rotation
    // from axis-angle to matrix representation.
    float rot_f[3];
    CvMat rot = cvMat(1, 3, CV_32F, rot_f);
    cvFindExtrinsicCameraParams2(objectPoints,
                                  imagePoints,
                                  &m_intrinsic,
                                  &m_distortion,
                                  &rot,
                                  &m_trans);

    LOG_INFO("Object points:");
    OpenCvUtility::LogCvMat32F(objectPoints);

    LOG_INFO("Image points:");
    OpenCvUtility::LogCvMat32F(imagePoints);

    cvRodrigues2(&rot,&m_rot);

    // Compute camera centre
    // (NOTE: this is in pixels as calibration
    //  uses pixels as the unit of measurement)
    float tx = -m_trans_f[0];
    float ty = -m_trans_f[1];
    float tz = -m_trans_f[2];
    m_cameraCentre.data.fl[0] = /*m_cC_f[0] =*/ m_rot_f[0]*tx +
                                                m_rot_f[3]*ty +
                                                m_rot_f[6]*tz;
    m_cameraCentre.data.fl[1] = /*m_cC_f[1] =*/ m_rot_f[1]*tx +
                                                m_rot_f[4]*ty +
                                                m_rot_f[7]*tz;
    m_cameraCentre.data.fl[2] = /*m_cC_f[2] =*/ m_rot_f[2]*tx +
                                                m_rot_f[5]*ty +
                                                m_rot_f[8]*tz;
}

/**
    Converts a point from image coordinates to ground-plane coordinates.
**/
CvPoint2D32f CameraCalibration::ImageToPlane(CvPoint2D32f p) const
{
    // Extract homography components and invert
    float    h[9];
    CvMat    hMat  = cvMat(3,3,CV_32F,h);

    h[0] = m_rot_f[0];
    h[1] = m_rot_f[1];
    h[2] = m_trans_f[0];
    h[3] = m_rot_f[3];
    h[4] = m_rot_f[4];
    h[5] = m_trans_f[1];
    h[6] = m_rot_f[6];
    h[7] = m_rot_f[7];
    h[8] = m_trans_f[2];
    cvInvert(&hMat,&hMat);

    float* hData = hMat.data.fl;

    float x = p.x;
    float y = p.y;

    // Apply inverse radial distortion - found out we can't use this to undistort image corners
    // because they are beyond the limit of the distortion model for our camera
    float xd = (x-cx())/fx();
    float yd = (y-cy())/fy();
    float r2 = xd*xd + yd*yd;
    float r4 = r2*r2;
    float r6 = r4*r2;
    float a1 = 2.f*xd*yd;
    float a2 = r2 + 2.f*xd*xd;
    float a3 = r2 + 2.f*yd*yd;
    float cdist = 1.f + ik1()*r2 + ik2()*r4 + ik5()*r6;
    x = xd*cdist + ik3()*a1 + ik4()*a2;
    y = yd*cdist + ik3()*a3 + ik4()*a1;

    // Apply inverse homography
    float xImg = hData[0]*x + hData[1]*y + hData[2];
    float yImg = hData[3]*x + hData[4]*y + hData[5];
    float wImg = hData[6]*x + hData[7]*y + hData[8];

    float proj = 1.f/wImg;

    return cvPoint2D32f(xImg*proj, yImg*proj);
}

/**
    Convert from ground-plane coordinates to image coordinates
**/
CvPoint2D32f CameraCalibration::PlaneToImage(CvPoint2D32f p) const
{
    float x,y,z;

    // Parameters for radial distortion model
    float r2, r4, r6, a1, a2, a3, cdist, xd, yd;

    // homography
    x = m_rot_f[0]*p.x + m_rot_f[1]*p.y + m_trans_f[0];
    y = m_rot_f[3]*p.x + m_rot_f[4]*p.y + m_trans_f[1];
    z = m_rot_f[6]*p.x + m_rot_f[7]*p.y + m_trans_f[2];
    z = 1.f/z;
    x *= z;
    y *= z;

    // lens distortion
    r2 = x*x + y*y;
    r4 = r2*r2;
    r6 = r2*r4;
    a1 = 2*x*y;
    a2 = r2 + 2*x*x;
    a3 = r2 + 2*y*y;
    cdist = 1.f + k1()*r2 + k2()*r4 + k5()*r6;
    xd = x*cdist + k3()*a1 + k4()*a2;
    yd = y*cdist + k3()*a3 + k4()*a1;
    x  = xd*fx() + cx();
    y  = yd*fy() + cy();

    return cvPoint2D32f(x,y);
}

/**
    Plots a line in the image which goes from the coordinate origin to the
    camera centre's x,y-position in the ground plane (if we dropped a plum
    line from the camera it would roughly land there.)
**/
void CameraCalibration::PlotCameraCentre(IplImage* img, const RobotMetrics& metrics)
{
    float c[3];
    const CvMat* C = GetCameraCentrePx();
    c[0] = C->data.fl[0];
    c[1] = C->data.fl[1];
    c[2] = C->data.fl[2];

    // print the camera centre in centimetres
    float isf = 1.f/metrics.GetScaleFactor();

    LOG_INFO(QObject::tr("Camera Position: %1 %2 %3.").arg(c[0]*isf)
                                                      .arg(c[1]*isf)
                                                      .arg(c[2]*isf));

    // double line for clarity in grey-scale image
    int ox = (int)(-GetUnwarpOffset()->x+.5f);
    int oy = (int)(-GetUnwarpOffset()->y+.5f);
    int cx = (int)(c[0]-GetUnwarpOffset()->x+.5f);
    int cy = (int)(c[1]-GetUnwarpOffset()->y+.5f);

    cvLine(img,
            cvPoint(ox, oy),
            cvPoint(cx, cy),
            cvScalar(255,255,255),
            3, CV_AA);

    cvLine(img,
            cvPoint(ox, oy),
            cvPoint(cx, cy),
            cvScalar(0,0,0),
            1, CV_AA);
}
