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

#ifndef CAMERACALIBRATION_H
#define CAMERACALIBRATION_H

#include "WbConfig.h"

#include <opencv/cv.h>

class RobotMetrics;

/**
    Class for storing and manipulating a complete camera calibration
    (that is, intrinsic and extrinsic parameters, distortion coefficients
    and inverse distortion coefficients).

    To construct we must pass it the name of a calibration image (view of
    chessboard calibration pattern).
**/
class CameraCalibration
{

public:
    CameraCalibration();
    ~CameraCalibration();

    bool LoadIntrinsicCalibration( const WbConfig& cameraCalCfg );

    bool LoadCameraTransform( const KeyId camPosId, const WbConfig& floorPlanCfg );

    bool PerformExtrinsicCalibration(CvSize        boardSize,
                                      RobotMetrics& metrics,
                                      IplImage**    viewWarp,
                                      const bool    scaled,
                                      const char*   calImage );

    void ComputeWarpGradientMagnitude();

    float ComputeSquareSize(CvMat* imagePoints);

    void ComputeExtrinsicParams( const CvMat* objectPoints, const CvMat* imagePoints );

    void UnwarpGroundPlane( const IplImage* src, IplImage* dst ) { cvRemap( src, dst, m_mapx, m_mapy, CV_INTER_LINEAR ); };

    void PlotCameraCentre( IplImage* img, const RobotMetrics& metrics );

    CvPoint2D32f ImageToPlane(CvPoint2D32f p) const;
    CvPoint2D32f PlaneToImage(CvPoint2D32f p) const;

    // get calibration info
    const CvMat* GetIntrinsicParams()       const { return &m_intrinsic; };
    const CvMat* GetDistortionParams()      const { return &m_distortion; };
    const CvMat* GetUndistortionParams()    const { return &m_inverse; };
    const CvMat* GetRotationParams()        const { return &m_rot; };
    const CvMat* GetTranslationParams()     const { return &m_trans; };
    const CvMat* GetCameraCentrePx()        const { return &m_cameraCentre; };

    const CvMat* GetCameraTransform()       const { return &m_transform; };

    // get individual parameters
    const float& fx() const { return m_intrinsic_f[0]; };
    const float& fy() const { return m_intrinsic_f[4]; };
    const float& cx() const { return m_intrinsic_f[2]; };
    const float& cy() const { return m_intrinsic_f[5]; };

    const float& k1() const { return m_distortion_f[0]; };
    const float& k2() const { return m_distortion_f[1]; };
    const float& k3() const { return m_distortion_f[2]; };
    const float& k4() const { return m_distortion_f[3]; };
    const float& k5() const { return m_distortion_f[4]; };

    const float& ik1() const { return m_inverse_f[0]; };
    const float& ik2() const { return m_inverse_f[1]; };
    const float& ik3() const { return m_inverse_f[2]; };
    const float& ik4() const { return m_inverse_f[3]; };
    const float& ik5() const { return m_inverse_f[4]; };

    const CvPoint2D32f* GetUnwarpOffset() const { return &m_offset; };
    CvSize GetImageSize() const { return cvSize( m_imageWidth, m_imageHeight); };

    const CvMat* GetWarpGradientImage() const { return m_mapGM; };

    const IplImage* GetWarpedCalibrationImage() const { return m_calWarpImg; };

private:
    CvMat* m_mapx;         // Stores precomputed undistortion map x-coords
    CvMat* m_mapy;         // Stores precomputed undistortion map y-coords
    CvMat* m_mapDx;        // x gradient-magnitude of undistortion
    CvMat* m_mapDy;        // y gradient-magnitude of undistortion
    CvMat* m_mapGM;        // combined grad-mag of x and y map derivatives

    int m_imageWidth;      // width of image
    int m_imageHeight;     // height of image

    CvPoint2D32f m_offset; // unwarp offset

    CvMat m_rot;
    float m_rot_f[9];

    CvMat m_trans;
    float m_trans_f[3];

    CvMat m_cameraCentre;
    float m_cC_f[3];

    CvMat m_intrinsic;
    float m_intrinsic_f[9];

    CvMat m_distortion;
    float m_distortion_f[5];

    CvMat m_inverse;
    float m_inverse_f[5];

    CvMat m_transform;
    float m_transform_f[9];

    const char* m_calImageName;
    IplImage*   m_calWarpImg;
};

#endif // CAMERACALIBRATION_H
