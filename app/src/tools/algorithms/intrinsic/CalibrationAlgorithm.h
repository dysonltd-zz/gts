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

#ifndef CALIBRATIONALGORITHM_H_
#define CALIBRATIONALGORITHM_H_

#include <opencv/cv.h>

#include <QtCore/qstring.h>

#include "WbKeyValues.h"

#include <vector>
#include <memory>

class WbConfig;

/** @brief Class to calculate the intrinsic parameters of a camera.
 *
 *  Adapted from opencv/samples/c/calibration.cpp
 *
*/

class CalibrationAlgorithm
{
public:
    CalibrationAlgorithm();
    ~CalibrationAlgorithm();

    bool Run( WbConfig config );

private:
    typedef std::vector< cv::Point2f > PointsVec2D;
    typedef std::vector< cv::Point3f > PointsVec3D;

    void SetupParameters( const WbConfig& config );

    bool RunCalibration( const cv::Size& imgSize );
    double ComputeReprojectionError( const std::vector< cv::Mat >& rotationVectors,
                                     const std::vector< cv::Mat >& translationVectors );
    void InitialisePointMatrices();

    void SaveCalibrationResults( WbConfig config,
                                 const CvSize& imgSize,
                                 const bool wasSuccessful );
    void CalculateInverseDistortionParameters();

    int NumInputImages() const;
    bool CornersFoundInAllImages() const;
    const size_t NumImagesWithCorners() const;
    int NumGridPoints() const;

    void CalculateCameraSpaceGridCoords( const std::vector< cv::Mat >& rot_vects,
                                         const std::vector< cv::Mat >& trans_vects );

    bool TryToCapturePoints( const WbConfig& config,
                                   cv::Size& imgSize );

    IplImage* const TryToLoadImage( const WbConfig& config,
                                    const int       imgIndex ) const;

    void FlipImageIfNecessary( IplImage& image ) const;

    bool TryToFindImagePoints( IplImage& image, PointsVec2D& imagePoints,
                                     const int imageIndex ) const;

    void ImproveCornerAccuracy( IplImage& image, PointsVec2D& imagePoints );


    cv::Size                     m_gridSize;
    double                       m_squareSize;
    double                       m_aspectRatio;
    int                          m_calibrateFlags;
    bool                         m_flipVertical;
    WbKeyValues::ValueIdPairList m_fileNamesAndIds;
    std::vector< KeyId >         m_imageWithCornersIds;

    typedef CvPoint2D32f PointType;

    std::vector<double> m_reprojectionErrors;
    std::unique_ptr<cv::Mat> m_cameraMtx;
    std::unique_ptr<cv::Mat> m_distortionCoeffs;
    std::unique_ptr<cv::Mat> m_inverseDistortionCoeffs;
    std::vector< PointsVec2D > m_imageGridPoints;
    std::vector< PointsVec3D > m_worldGridPoints;
    std::vector< PointsVec3D > m_cameraGridPoints;
    std::vector< PointsVec2D > m_reprojectedImagePoints;

    double m_avgReprojectionError;
};

#endif
