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

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include <QtCore/qstring.h>

class WbConfig;

class ExtrinsicCalibrationAlgorithm
{
public:
    ExtrinsicCalibrationAlgorithm();
    bool Run( WbConfig config );

private:
    bool LoadInputImage( const QString& imageFileName );
    bool DetectChessBoardPattern( const CvSize& gridSize );

    bool LoadCameraCalibration( const WbConfig& config );
    bool LoadCameraConfig( const WbConfig& calibCfg,
                           WbConfig* const cameraCfg );
    bool PopulateCameraCalibrationMatrices( const WbConfig& cameraCfg );
    bool CalibrateCamera( const CvSize& gridSize,
                          const double squareSizeInCm );
    void ComputeSquareSize();
    void RecordCalibration( WbConfig config );

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
