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

#include "CalibrationAlgorithm.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <iostream>
#include <algorithm>

#include "WbConfig.h"
#include "CalibrationSchema.h"
#include "LevenbergMarquardt.h"
#include "Message.h"

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include <QtCore/QFileInfo>
#include <QtCore/QObject>
#include <QtCore/QTime>
#include <QtGui/QApplication>

#include "Debugging.h"

namespace
{
    const int FOCAL_LENGTH_X_ROW = 0;
    const int FOCAL_LENGTH_X_COL = 0;
    const int FOCAL_LENGTH_Y_ROW = 1;
    const int FOCAL_LENGTH_Y_COL = 1;
    const int CAM_CENTRE_X_ROW = 0;
    const int CAM_CENTRE_X_COL = 2;
    const int CAM_CENTRE_Y_ROW = 1;
    const int CAM_CENTRE_Y_COL = 2;

    void SetFocalLength(cv::Mat& cameraMtx, const double focalX, const double focalY)
    {
        cameraMtx.at<double>(FOCAL_LENGTH_X_ROW, FOCAL_LENGTH_X_COL) = focalX;
        cameraMtx.at<double>(FOCAL_LENGTH_Y_ROW, FOCAL_LENGTH_Y_COL) = focalY;
    }

    const cv::Point2d GetFocalLength(cv::Mat& cameraMtx)
    {
        return cv::Point2d(cameraMtx.at<double>(FOCAL_LENGTH_X_ROW, FOCAL_LENGTH_X_COL),
                            cameraMtx.at<double>(FOCAL_LENGTH_Y_ROW, FOCAL_LENGTH_Y_COL)
                           );
    }

    const cv::Point2d GetCameraCentre(cv::Mat& cameraMtx)
    {
        return cv::Point2d(cameraMtx.at<double>(CAM_CENTRE_X_ROW, CAM_CENTRE_X_COL),
                            cameraMtx.at<double>(CAM_CENTRE_Y_ROW, CAM_CENTRE_Y_COL)
                           );
    }
}

namespace
{
    cv::Point2f GetDistortedImagePt(const cv::Point2f& imagePt,
                                     const cv::Point2d& focalLength,
                                     const cv::Point2d& cameraCentre)
    {
        cv::Point2f distortedImagePt;
        distortedImagePt.x = (imagePt.x - cameraCentre.x) / focalLength.x;
        distortedImagePt.y = (imagePt.y - cameraCentre.y) / focalLength.y;
        return distortedImagePt;
    }

    cv::Point2f GetNormalisedCameraSpacePt(const cv::Point3f& cameraSpacePt)
    {
        cv::Point2f normalisedCameraSpacePt;
        normalisedCameraSpacePt.x = cameraSpacePt.x / cameraSpacePt.z;
        normalisedCameraSpacePt.y = cameraSpacePt.y / cameraSpacePt.z;
        return normalisedCameraSpacePt;
    }

    inline const MatrixElementType sq(const MatrixElementType x)
    {
        return x*x;
    }

    const cv::Point2f Undistort(const cv::Point2f& distortedNormalisedImagePt,
                                 const cv::Mat& p)
    {
        const cv::Point2f& xd = distortedNormalisedImagePt;

        MatrixElementType k1 = p.at<MatrixElementType>(0,0);
        MatrixElementType k2 = p.at<MatrixElementType>(1,0);
        MatrixElementType d1 = p.at<MatrixElementType>(2,0);
        MatrixElementType d2 = p.at<MatrixElementType>(3,0);
        MatrixElementType k3 = p.at<MatrixElementType>(4,0);
        const MatrixElementType r2 = sq(xd.x) + sq(xd.y);
        const MatrixElementType r4 = sq(r2);
        const MatrixElementType r6 = r4 * r2;

        const MatrixElementType a1 = 2*xd.x*xd.y;
        const MatrixElementType a2 = r2+2*sq(xd.x);
        const MatrixElementType a3 = r2+2*sq(xd.y);

        const MatrixElementType cdist = 1 + k1*r2 + k2*r4 + k3*r6;

        //undistorted
        const MatrixElementType xu = xd.x*cdist + d1*a1 + d2*a2;
        const MatrixElementType yu = xd.y*cdist + d1*a3 + d2*a1;

        return cv::Point2f(xu, yu);
    }

    class ImageDistortionCostFunction
    {
    public:
        static const int numPtDimensions = 2;
        static const int xOffset = 0;
        static const int yOffset = 1;

        ImageDistortionCostFunction(const std::vector< cv::Point2f >& normalisedCameraSpaceGridPts,
                                     const std::vector< cv::Point2f >& distortedImageGridPts)
        :
            m_normalisedCameraSpaceGridPts(normalisedCameraSpaceGridPts),
            m_distortedImageGridPts(distortedImageGridPts),
            m_jacobian(rangeDim(), domainDim(), OpenCvMatrixElementType)
        {
            assert(m_distortedImageGridPts.size() ==
                m_normalisedCameraSpaceGridPts.size());
            PreComputeConstantsAndJacobian();
        }

        void operator () (cv::Mat& r, const cv::Mat& p) const
        {
            // calculate the sum of square errors between normalized
            // image coordinates x=X/Z, y=Y/Z of the checkerboard and
            // distorted (almost normalized) image coordinates
            // xd = (x_image-cx)/fx;yd = (y_image-cy)/fy
            // p = (k1,k2,d1,d2) - radial and tangential distortion parameters

            MatrixElementType k1 = p.at<MatrixElementType>(0,0);
            MatrixElementType k2 = p.at<MatrixElementType>(1,0);
            MatrixElementType d1 = p.at<MatrixElementType>(2,0);
            MatrixElementType d2 = p.at<MatrixElementType>(3,0);
            MatrixElementType k3 = p.at<MatrixElementType>(4,0);

            for (size_t i = 0; i < m_distortedImageGridPts.size(); ++i)
            {
                QApplication::processEvents();
                const int thisPtOffset = i*numPtDimensions;
                const cv::Point2f& xd = m_distortedImageGridPts[ i ];
                const cv::Point2f& xn = m_normalisedCameraSpaceGridPts[ i ];

                // r=distance from camera centre, r2 = r^2, r4=r^4, r6=r^6
                const MatrixElementType& r2 = m_r2[i];
                const MatrixElementType& r4 = m_r4[i];
                const MatrixElementType& r6 = m_r6[i];

                const MatrixElementType& a1 = m_a1[i];
                const MatrixElementType& a2 = m_a2[i];
                const MatrixElementType& a3 = m_a3[i];

                const MatrixElementType cdist = 1 + k1*r2 + k2*r4 + k3*r6;

                //undistorted
                const MatrixElementType xu = xd.x*cdist + d1*a1 + d2*a2;
                const MatrixElementType yu = xd.y*cdist + d1*a3 + d2*a1;

                r.at<MatrixElementType>(thisPtOffset+xOffset, 0) = xu-xn.x;
                r.at<MatrixElementType>(thisPtOffset+yOffset, 0) = yu-xn.y;
            }
        }

        void jacobian(cv::Mat& J, const cv::Mat& p) const
        {
            Q_UNUSED(p);

            J = m_jacobian;
        }

        int domainDim() const { return 5; }

        int rangeDim() const { return m_distortedImageGridPts.size()*numPtDimensions; }

    private:
        void PreComputeConstantsAndJacobian()
        {
            m_r2.reserve(m_distortedImageGridPts.size());
            m_r4.reserve(m_distortedImageGridPts.size());
            m_r6.reserve(m_distortedImageGridPts.size());
            m_a1.reserve(m_distortedImageGridPts.size());
            m_a2.reserve(m_distortedImageGridPts.size());
            m_a3.reserve(m_distortedImageGridPts.size());

            cv::Mat& J(m_jacobian);

            for (size_t i = 0; i < m_distortedImageGridPts.size(); ++i)
            {
                 const int thisPtOffset = i*numPtDimensions;
                 const cv::Point2f& xd = m_distortedImageGridPts[ i ];
                 //const cv::Point2f& xn = m_normalisedCameraSpaceGridPts[ i ];

                 // r=distance from camera centre, r2 = r^2, r4=r^4, r6=r^6
                 const MatrixElementType r2 = sq(xd.x) + sq(xd.y);
                 const MatrixElementType r4 = sq(r2);
                 const MatrixElementType r6 = r4 * r2;

                 const MatrixElementType a1 = 2*xd.x*xd.y;
                 const MatrixElementType a2 = r2+2*sq(xd.x);
                 const MatrixElementType a3 = r2+2*sq(xd.y);

                 J.at<MatrixElementType>(thisPtOffset+xOffset, 0) = r2*xd.x;
                 J.at<MatrixElementType>(thisPtOffset+xOffset, 1) = r4*xd.x;
                 J.at<MatrixElementType>(thisPtOffset+xOffset, 2) = a1;
                 J.at<MatrixElementType>(thisPtOffset+xOffset, 3) = a2;
                 J.at<MatrixElementType>(thisPtOffset+xOffset, 4) = r6*xd.x;

                 J.at<MatrixElementType>(thisPtOffset+yOffset, 0) = r2*xd.y;
                 J.at<MatrixElementType>(thisPtOffset+yOffset, 1) = r4*xd.y;
                 J.at<MatrixElementType>(thisPtOffset+yOffset, 2) = a3;
                 J.at<MatrixElementType>(thisPtOffset+yOffset, 3) = a1;
                 J.at<MatrixElementType>(thisPtOffset+yOffset, 4) = r6*xd.y;

                 m_r2.push_back(r2);
                 m_r4.push_back(r4);
                 m_r6.push_back(r6);
                 m_a1.push_back(a1);
                 m_a2.push_back(a2);
                 m_a3.push_back(a3);
             }
        }

        const std::vector< cv::Point2f >& m_normalisedCameraSpaceGridPts; //normalised to lie on unit depth plane
        const std::vector< cv::Point2f >& m_distortedImageGridPts; //normalised except still distorted
        std::vector< MatrixElementType > m_r2;
        std::vector< MatrixElementType > m_r4;
        std::vector< MatrixElementType > m_r6;
        std::vector< MatrixElementType > m_a1;
        std::vector< MatrixElementType > m_a2;
        std::vector< MatrixElementType > m_a3;
        cv::Mat m_jacobian;
    };
}

namespace
{
    const bool DONT_RAISE_RUNTIME_ERRORS = true;

    bool AllInRange(const std::vector< cv::Mat > matrices)
    {
        for (size_t i = 0; i < matrices.size(); ++i)
        {
            if (!cv::checkRange(matrices.at(i), DONT_RAISE_RUNTIME_ERRORS))
            {
                return false;
            }
        }

        return true;
    }
}

CalibrationAlgorithm::CalibrationAlgorithm() :
    m_gridSize               (cvSize(0, 0)),
    m_squareSize             (1.),
    m_aspectRatio            (1.),
    m_calibrateFlags         (0),
    m_flipVertical           (false),
    m_fileNamesAndIds        (),
    m_imageWithCornersIds    (),
    m_reprojectionErrors     (0),
    m_cameraMtx              (new cv::Mat(3, 3, CV_64F)),
    m_distortionCoeffs       (new cv::Mat(5, 1, CV_64F)),
    m_inverseDistortionCoeffs(new cv::Mat(5, 1, CV_64F)),
    m_imageGridPoints        (0),
    m_worldGridPoints        (0),
    m_avgReprojectionError   (0.0)
{
}

CalibrationAlgorithm::~CalibrationAlgorithm()
{
}

void CalibrationAlgorithm::CalculateInverseDistortionParameters()
{
    cv::Point2d focalLength (GetFocalLength(*m_cameraMtx));
    cv::Point2d cameraCentre(GetCameraCentre(*m_cameraMtx));

//    a = GetPointsInCameraCoordsAndMeasuredInImage();

    std::vector< cv::Point2f > normalisedCameraSpaceGridPts; //normalised to lie on unit depth plane
    std::vector< cv::Point2f > distortedImageGridPts; //normalised except still distorted
    for (size_t imageIndex = 0; imageIndex < m_imageGridPoints.size(); ++imageIndex)
    {
        QApplication::processEvents();
        std::vector< cv::Point2f >& thisImgGridPoints(m_imageGridPoints[ imageIndex ]);
        std::vector< cv::Point3f >& thisImgCamGridPoints(m_cameraGridPoints[ imageIndex ]);
        for (size_t ptIndex = 0; ptIndex < thisImgGridPoints.size(); ++ptIndex)
        {
            cv::Point2f& imgPt(thisImgGridPoints[ ptIndex ]);
            cv::Point3f& camPt(thisImgCamGridPoints[ ptIndex ]);

            normalisedCameraSpaceGridPts.push_back(GetNormalisedCameraSpacePt(camPt));
            distortedImageGridPts.push_back(GetDistortedImagePt(imgPt,
                                                                  focalLength,
                                                                  cameraCentre));
        }
    }

    cv::Mat inverseDistortionParamsInitEstimate(5, 1, OpenCvMatrixElementType);
    inverseDistortionParamsInitEstimate.setTo(0.0);

    ImageDistortionCostFunction f(normalisedCameraSpaceGridPts,
                                   distortedImageGridPts);
    LevenbergMarquardt(f,
                        inverseDistortionParamsInitEstimate,
                        *m_inverseDistortionCoeffs);
}

void CalibrationAlgorithm::CalculateCameraSpaceGridCoords(const std::vector< cv::Mat >& rot_vects,
                                                           const std::vector< cv::Mat >& trans_vects)
{
    m_cameraGridPoints.clear();
    cv::Mat Rot(3, 3, CV_32FC1);

    for (size_t i = 0; i < rot_vects.size(); i++)
    {
        cv::Mat rot_v;
        rot_vects[ i ].convertTo(rot_v, CV_32FC1);
        cv::Mat t;
        trans_vects[ i ].convertTo(t, CV_32FC1);

        cv::Rodrigues(rot_v, Rot);

        m_cameraGridPoints.push_back(std::vector< cv::Point3f >());
        m_cameraGridPoints.reserve(m_imageGridPoints[ i ].size());
        for (size_t j = 0; j < m_imageGridPoints[ i ].size(); j++)
        {
            cv::Mat X(m_worldGridPoints[ i ][ j ]);
            cv::Mat x(3, 1, CV_32FC1);
            x = (Rot * X) + t;
            m_cameraGridPoints.back().push_back(cv::Point3f(x));
        }
    }
}

double CalibrationAlgorithm::ComputeReprojectionError(const std::vector< cv::Mat >& rotationVectors,
                                                       const std::vector< cv::Mat >& translationVectors)
{
    const size_t numImages = NumImagesWithCorners();
    int totalPointsSoFar = 0;
    double totalReprojectionError = 0;

    m_reprojectionErrors.clear();
    m_reprojectedImagePoints.clear();
    for (size_t imageIndex = 0; imageIndex < numImages; imageIndex++)
    {
        QApplication::processEvents();
        std::vector< cv::Point2f > imageGridPointsInThisImage(m_imageGridPoints[ imageIndex ]);
        std::vector< cv::Point3f > worldGridPointsInThisImage(m_worldGridPoints[ imageIndex ]);
        std::vector< cv::Point2f > reprojectedPointsInThisImage;
        int numPointsInThisImage = imageGridPointsInThisImage.size();
        cv::Mat rotationVector(rotationVectors[ imageIndex ]);
        cv::Mat translationVector(translationVectors[ imageIndex ]);

        totalPointsSoFar += numPointsInThisImage;

        cv::projectPoints(cv::Mat(worldGridPointsInThisImage),
                           rotationVector,
                           translationVector,
                           *m_cameraMtx,
                           *m_distortionCoeffs,
                           reprojectedPointsInThisImage);

        ///@todo  L1 Norm --- really?
        const double totalReprojectionErrorInThisImage =
            cv::norm(cv::Mat(imageGridPointsInThisImage),
                      cv::Mat(reprojectedPointsInThisImage), CV_L1);
        const double avgReprojectionErrorInThisImage =
            totalReprojectionErrorInThisImage / numPointsInThisImage;
        m_reprojectionErrors.push_back(avgReprojectionErrorInThisImage);
        totalReprojectionError += totalReprojectionErrorInThisImage;
        m_reprojectedImagePoints.push_back(reprojectedPointsInThisImage);
    }

    return totalReprojectionError / totalPointsSoFar;
}

void CalibrationAlgorithm::InitialisePointMatrices()
{
    m_worldGridPoints.clear();

    for (size_t i = 0; i < NumImagesWithCorners(); i++)
    {
        m_worldGridPoints.push_back(std::vector<cv::Point3f>());
        m_worldGridPoints.back().reserve(NumGridPoints());
        for (int j = 0; j < m_gridSize.height; j++)
        {
            for (int k = 0; k < m_gridSize.width; k++)
            {
                m_worldGridPoints.back().push_back(cv::Point3f(j * m_squareSize,
                                                                 k * m_squareSize, 0));
            }
        }
    }
}

int CalibrationAlgorithm::NumGridPoints() const
{
    return m_gridSize.width * m_gridSize.height;
}

bool CalibrationAlgorithm::RunCalibration(const cv::Size& imgSize)
{
    InitialisePointMatrices();

    std::vector< cv::Mat > rotationVectors;
    std::vector< cv::Mat > translationVectors;

    m_cameraMtx->setTo(0);
    m_distortionCoeffs->setTo(0);

    if (m_calibrateFlags & CV_CALIB_FIX_ASPECT_RATIO)
    {
        SetFocalLength(*m_cameraMtx, m_aspectRatio, 1.0);
    }

    QApplication::processEvents();
    cv::calibrateCamera(m_worldGridPoints,
                         m_imageGridPoints,
                         imgSize,
                         *m_cameraMtx,
                         *m_distortionCoeffs,
                         rotationVectors,
                         translationVectors,
                         m_calibrateFlags);

    // Check for Infs and Nans
    bool successful = cv::checkRange(*m_cameraMtx, DONT_RAISE_RUNTIME_ERRORS) &&
                      cv::checkRange(*m_distortionCoeffs, DONT_RAISE_RUNTIME_ERRORS) &&
                      AllInRange(rotationVectors) && AllInRange(translationVectors);

    m_reprojectionErrors.clear();
    m_avgReprojectionError = ComputeReprojectionError(rotationVectors,
                                                       translationVectors);

    CalculateCameraSpaceGridCoords(rotationVectors, translationVectors);
    CalculateInverseDistortionParameters();

    return successful;
}

void CalibrationAlgorithm::SaveCalibrationResults(WbConfig config,
                                                   const CvSize& imgSize,
                                                   const bool wasSuccessful)
{
    using namespace CalibrationSchema;

    const QString currentDateString(
        QDate::currentDate().toString(QObject::tr("d MMMM yyyy", "Calibration Date Format")));
    const QString currentTimeString(
        QTime::currentTime().toString(QObject::tr("h.mmap", "Calibration Time Format")));

    config.SetKeyValue(calibrationSuccessfulKey, KeyValue::from(wasSuccessful));
    config.SetKeyValue(calibrationDateKey, KeyValue::from(currentDateString));
    config.SetKeyValue(calibrationTimeKey, KeyValue::from(currentTimeString));

    config.SetKeyValue(rowsUsedForCalibrationKey,    KeyValue::from(m_gridSize.height));
    config.SetKeyValue(columnsUsedForCalibrationKey, KeyValue::from(m_gridSize.width));

    config.SetKeyValue(imageWidthKey,  KeyValue::from(imgSize.width));
    config.SetKeyValue(imageHeightKey, KeyValue::from(imgSize.height));

    config.SetKeyValue(cameraMatrixKey, KeyValue::from(*m_cameraMtx));
    config.SetKeyValue(distortionCoefficientsKey, KeyValue::from(*m_distortionCoeffs));
    config.SetKeyValue(invDistortionCoefficientsKey,
                        KeyValue::from(*m_inverseDistortionCoeffs));

    config.SetKeyValue(avgReprojectionErrorKey, KeyValue::from(m_avgReprojectionError));

    for (size_t i = 0; i < m_imageWithCornersIds.size(); ++i)
    {
        config.SetKeyValue(imageErrorKey,
                            KeyValue::from(m_reprojectionErrors.at(i)),
                            m_imageWithCornersIds.at(i));
        config.SetKeyValue(imageReprojectedPointsKey,
                            KeyValue::from(m_reprojectedImagePoints.at(i)),
                            m_imageWithCornersIds.at(i));
    }
}

int CalibrationAlgorithm::NumInputImages() const
{
    return (int) m_fileNamesAndIds.size();
}

const size_t CalibrationAlgorithm::NumImagesWithCorners() const
{
    return m_imageGridPoints.size();
}

bool CalibrationAlgorithm::CornersFoundInAllImages() const
{
    return (NumImagesWithCorners() ==  (unsigned) NumInputImages());
}

void CalibrationAlgorithm::SetupParameters(const WbConfig& config)
{
    using namespace CalibrationSchema;

    m_gridSize.width = config.GetKeyValue(gridColumnsKey).ToInt();
    assert((m_gridSize.width >= 1) && "Invalid num grid columns");
    m_gridSize.height = config.GetKeyValue(gridRowsKey).ToInt();
    assert((m_gridSize.height >= 1) && "Invalid num grid rows");

    m_squareSize = config.GetKeyValue(gridSquareSizeInCmKey).ToDouble();
    assert((m_squareSize >= 0) && "Invalid square width");

    m_aspectRatio = config.GetKeyValue(fixedAspectRatioKey).ToDouble();
    if (config.GetKeyValue(shouldFixAspectRatioKey).ToBool())
    {
        assert((m_aspectRatio >= 0) && "Invalid fixed aspect ratio");
        m_calibrateFlags |= CV_CALIB_FIX_ASPECT_RATIO;
    }

    if (config.GetKeyValue(noTangentialDistortionKey).ToBool())
    {
        m_calibrateFlags |= CV_CALIB_ZERO_TANGENT_DIST;
    }

    if (config.GetKeyValue(fixPrincipalPointKey).ToBool())
    {
        m_calibrateFlags |= CV_CALIB_FIX_PRINCIPAL_POINT;
    }

    if (config.GetKeyValue(flipImagesKey).ToBool())
    {
        m_flipVertical = 1;
    }

    m_fileNamesAndIds = config.GetKeyValues(imageFileKey);
}

IplImage* const CalibrationAlgorithm::TryToLoadImage(const WbConfig& config, const int imgIndex) const
{
    QApplication::processEvents();
    IplImage* image = 0;
    QString imagename(config.GetAbsoluteFileNameFor(m_fileNamesAndIds.at(imgIndex).value.ToQString()));
    if (QFileInfo(imagename).exists())
    {
        image = cvLoadImage(imagename.toAscii(), CV_LOAD_IMAGE_GRAYSCALE);
    }

    if (!image)
    {
        Message::Show(0,
                       QObject::tr("Calibration Algorithm"),
                       QObject::tr("Warning - Unable to load image: %1!")
                         .arg(imagename),
                       Message::Severity_Warning);
    }
    return image;
}

/*
std::shared_ptr<cv::Mat> const CalibrationAlgorithm::TryToLoadImage(const WbConfig& config, const int imgIndex) const
{
    QApplication::processEvents();
    std::shared_ptr<cv::Mat> image(new cv::Mat());
    QString imagename(config.GetAbsoluteFileNameFor(m_fileNamesAndIds.at(imgIndex).value.ToQString()));
    if (QFileInfo(imagename).exists())
    {
        image = cv::imread(imagename.toAscii(), CV_LOAD_IMAGE_GRAYSCALE);
    }

    if (!image)
    {
        Message::Show(0,
                       QObject::tr("Calibration Algorithm"),
                       QObject::tr("Warning - Unable to load image: %1!")
                         .arg(imagename),
                       Message::Severity_Warning);
    }
    return image;
}
*/

void CalibrationAlgorithm::FlipImageIfNecessary(IplImage& image) const
{
    const int FLIP_AROUND_HORIZONTAL = 0;
    if (m_flipVertical)
    {
        QApplication::processEvents();
        cvFlip(&image, &image, FLIP_AROUND_HORIZONTAL);
    }
}

void CalibrationAlgorithm::FlipImageIfNecessary(cv::Mat& image) const
{
    const int FLIP_AROUND_HORIZONTAL = 0;
    if (m_flipVertical)
    {
        QApplication::processEvents();
        cv::flip(image, image, FLIP_AROUND_HORIZONTAL);
    }
}

bool CalibrationAlgorithm::TryToFindImagePoints(IplImage&    image,
                                                       PointsVec2D& imagePoints,
                                                       const int    imageIndex) const
{
    QApplication::processEvents();

#ifdef Deprecated_OpenCV
    const bool foundCorners = cv::findChessboardCorners(&image,
#else
    const bool foundCorners = cv::findChessboardCorners(cv::Mat(&image),
#endif
                                                         m_gridSize,
                                                         imagePoints,
                                                         CV_CALIB_CB_ADAPTIVE_THRESH);
    const bool found = foundCorners && (imagePoints.size() == (unsigned) m_gridSize.area());
    if (!found)
    {
        QString extraDebugInfo;
#ifndef NDEBUG
        if (!foundCorners)
        {
            extraDebugInfo = QObject::tr("findChessboardCorners method failed.");
        }
        else
        {
            extraDebugInfo = QObject::tr("Found %1 corners, expected %2.")
                                    .arg(imagePoints.size())
                                    .arg(m_gridSize.area());
        }
#endif
        Message::Show(0,
                       QObject::tr("Calibration Algorithm"),
                       QObject::tr("Warning - %1x%2 grid not found in Image %3!")
                                        .arg(m_gridSize.height)
                                        .arg(m_gridSize.width)
                                        .arg(imageIndex+1),
                                    Message::Severity_Warning,
                                    extraDebugInfo);
    }
    return found;
}

bool CalibrationAlgorithm::TryToFindImagePoints(cv::Mat& image, PointsVec2D& imagePoints, const int imageIndex) const
{
    QApplication::processEvents();

#ifdef Deprecated_OpenCV
    const bool foundCorners = cv::findChessboardCorners(&image,
#else
    const bool foundCorners = cv::findChessboardCorners(image,
#endif
                                                        m_gridSize,
                                                        imagePoints,
                                                        CV_CALIB_CB_ADAPTIVE_THRESH);
    const bool found = foundCorners && (imagePoints.size() == (unsigned) m_gridSize.area());
    if (!found)
    {
        QString extraDebugInfo;
#ifndef NDEBUG
        if (!foundCorners)
        {
            extraDebugInfo = QObject::tr("findChessboardCorners method failed.");
        }
        else
        {
            extraDebugInfo = QObject::tr("Found %1 corners, expected %2.")
                                    .arg(imagePoints.size())
                                    .arg(m_gridSize.area());
        }
#endif
        Message::Show(0,
                      QObject::tr("Calibration Algorithm"),
                      QObject::tr("Warning - %1x%2 grid not found in Image %3!")
                       .arg(m_gridSize.height)
                       .arg(m_gridSize.width)
                       .arg(imageIndex+1),
                      Message::Severity_Warning,
                      extraDebugInfo);
    }
    return found;
}

void CalibrationAlgorithm::ImproveCornerAccuracy(IplImage& image, PointsVec2D& imagePoints)
{
    QApplication::processEvents();

#ifdef Deprecated_OpenCV
    cv::cornerSubPix(&image,
#else
    cv::cornerSubPix(cv::Mat(&image),
#endif
                      imagePoints,
                      cv::Size(11, 11),
                      cv::Size(-1, -1),
                      cvTermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER,
                                      30,
                                      0.1));
}

void CalibrationAlgorithm::ImproveCornerAccuracy(cv::Mat& image, PointsVec2D& imagePoints)
{
    QApplication::processEvents();

#ifdef Deprecated_OpenCV
    cv::cornerSubPix(&image,
#else
    cv::cornerSubPix(image,
#endif
                     imagePoints,
                     cv::Size(11, 11),
                     cv::Size(-1, -1),
                     cvTermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER,
                                    30,
                                    0.1));
}

bool CalibrationAlgorithm::TryToCapturePoints(const WbConfig& config, cv::Size& imgSize)
{
    bool pointsCaptureSuccessful = true;
    for (int imgIndex = 0;
          (imgIndex < NumInputImages()) && pointsCaptureSuccessful;
          ++imgIndex)
    {
        IplImage* currentImage = TryToLoadImage(config, imgIndex);
        if (currentImage)
        {
            imgSize = cvGetSize(currentImage);

            FlipImageIfNecessary(*currentImage);

            PointsVec2D currentImagePoints;
            pointsCaptureSuccessful = TryToFindImagePoints(*currentImage,
                                                            currentImagePoints,
                                                            imgIndex);

            if (pointsCaptureSuccessful)
            {
                ImproveCornerAccuracy(*currentImage, currentImagePoints);

                m_imageWithCornersIds.push_back(m_fileNamesAndIds.at(imgIndex).id);
                m_imageGridPoints.push_back(currentImagePoints);
            }

            cvReleaseImage(&currentImage);
        }
    }
    return pointsCaptureSuccessful;
}

bool CalibrationAlgorithm::Run(WbConfig config)
{
    const QTime startTime(QTime::currentTime());
    SetupParameters(config);

    cv::Size imgSize(cvSize(0, 0));

    m_worldGridPoints.clear();
    m_imageGridPoints.clear();
    m_imageGridPoints.reserve(NumInputImages());
    m_imageWithCornersIds.clear();

    bool pointsCaptureSuccessful = TryToCapturePoints(config, imgSize);

    bool calibrationSuccessful = false;
    if (pointsCaptureSuccessful)
    {
        calibrationSuccessful = RunCalibration(imgSize);

        if (calibrationSuccessful)
        {
            SaveCalibrationResults(config, imgSize, calibrationSuccessful);
        }
    }

    const double msecsToCalibrate = startTime.msecsTo(QTime::currentTime());
    PRINT_VAR(msecsToCalibrate); Q_UNUSED(msecsToCalibrate);
    return calibrationSuccessful;
}
