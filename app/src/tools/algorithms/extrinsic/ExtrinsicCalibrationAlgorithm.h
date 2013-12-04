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

#ifndef EXTRINSICCALIBRATIONALGORITHM_H_
#define EXTRINSICCALIBRATIONALGORITHM_H_

#include <QtCore/QString>

#include <opencv/cv.h>
#include <opencv/highgui.h>

class WbConfig;

/**
 @brief Responsible for calibrating a 'position' in a room from a checkerboard pattern image.
 Handles image loading, camera calibration loading, position/extrinsic calibration and saving
 results to workbench config.
 */
class ExtrinsicCalibrationAlgorithm
{
public:
    ExtrinsicCalibrationAlgorithm();
    bool Run(WbConfig config);

private:
    /**
      @brief Loads an image file in to a OpenCV image from a given file path
      @param imageFileName File path to image
      @return True if successful
    **/
    bool LoadInputImage(const QString& imageFileName);

    /**
      @brief Wrapper around GroundPlaneUtilities to take loaded image and detect a chessboard/checkboard
      patern from a given grid size (Rows, Columns).
      @param gridSize (Rows, Columns) loaded from config i.e. set in GUI and saved.
      @return True if successful
    **/
    bool DetectChessBoardPattern(const CvSize& gridSize);

    /**
      @brief Load camera calibration parameters from workbench config and
      populate camera calibration matrices
      @param config Workbench config
      @return True if successful
    **/
    bool LoadCameraCalibration(const WbConfig& config);

    /**
      @brief Load camera config from workbench config
      @param calibCfg Camera calibration config
      @param cameraCfg Camera config
      @return True if successful
    **/
    bool LoadCameraConfig(const WbConfig& calibCfg, WbConfig* const cameraCfg);

    /**
      @brief Populate the Camera Calibration Matrices for a given camera config
      @param cameraCfg Config for camera
      @return True if successful
    **/
    bool PopulateCameraCalibrationMatrices(const WbConfig& cameraCfg);

    /**
      @brief Calibrate camera from given @a gridSize (rows, columns) and @a squareSizeInCm
      @param gridSize (rows, columns)
      @param squareSizeInCm Square size in cm (pre-conversion to pixels)
      @return True if successful
    **/
    bool CalibrateCamera(const CvSize& gridSize, const double squareSizeInCm);

    /**
      @brief Compute the square size in pixels
    **/
    void ComputeSquareSize();

    /**
      @brief Write the calibration to the config
      @param config The config you want to write the calibration to
    **/
    void RecordCalibration(WbConfig config);

    IplImage*         m_inputImageAsGrey;
    CvMat*            m_imagePoints;
    QString           m_errorString;
    CvMat*            m_intrinsicMatrix;
    CvMat*            m_distortionCoeffs;
    CvMat*            m_inverseCoeffs;
    CvMat*            m_rot;
    CvMat*            m_trans;
    double            m_squareSizePx;
};


#endif // EXTRINSICCALIBRATIONALGORITHM_H_
