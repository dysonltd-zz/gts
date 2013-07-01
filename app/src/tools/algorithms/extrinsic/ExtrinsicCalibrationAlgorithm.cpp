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

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include "ExtrinsicCalibrationAlgorithm.h"

#include "AlgorithmInterface.h"
#include "GroundPlaneUtility.h"

#include "CalibrationSchema.h"
#include "CameraPositionSchema.h"
#include "ExtrinsicCalibrationSchema.h"
#include "Message.h"
#include "CamerasCollection.h"

#include <QtCore/qstring.h>
#include <QtCore/QTime>

#include "OpenCvUtility.h"

#include "Logging.h"

ExtrinsicCalibrationAlgorithm::ExtrinsicCalibrationAlgorithm() :
    m_inputImageAsGrey ( 0 ),
    m_imagePoints      ( 0 ),
    m_errorString      ( ),
    m_intrinsicMatrix  ( 0 ),
    m_distortionCoeffs ( 0 ),
    m_inverseCoeffs    ( 0 ),
    m_rot              ( ),
    m_trans            ( )
{
    m_intrinsicMatrix  = cvCreateMat( 3, 3, CV_32F );
    m_distortionCoeffs = cvCreateMat( 5, 1, CV_32F );
    m_inverseCoeffs    = cvCreateMat( 5, 1, CV_32F );
}

bool ExtrinsicCalibrationAlgorithm::LoadInputImage( const QString& imageFileName )
{
    bool successful = true;
    m_inputImageAsGrey = cvLoadImage( imageFileName.toAscii(),
                                      CV_LOAD_IMAGE_GRAYSCALE );
    if ( !m_inputImageAsGrey )
    {
        m_errorString = QObject::tr( "Could not open image: %1!" )
                                   .arg( imageFileName );
        successful = false;
    }
    return successful;
}

bool ExtrinsicCalibrationAlgorithm::DetectChessBoardPattern( const CvSize& gridSize )
{
    bool successful = true;

    // Chess board pattern detection
    m_imagePoints = findChessBoard( m_inputImageAsGrey, gridSize );

    if ( !m_imagePoints )
    {
        m_errorString =  QObject::tr( "%1x%2 grid not found in the image.  "
                                      "Check image and parameters and try again.",
                                      "ExtrinsicCalibrationAlgorithm" )
                                    .arg( gridSize.height )
                                    .arg( gridSize.width );
        successful = false;
    }
    return successful;
}

bool ExtrinsicCalibrationAlgorithm::PopulateCameraCalibrationMatrices( const WbConfig& cameraCfg )
{
    using namespace CalibrationSchema;
    bool successful = true;
    const WbConfig cameraIntrisicConfig( cameraCfg.GetSubConfig( schemaName ) );

    const bool calibrationWasSuccessful = cameraIntrisicConfig
                    .GetKeyValue( calibrationSuccessfulKey )
                    .ToBool();
    const bool cameraMtxValid = cameraIntrisicConfig
                    .GetKeyValue( cameraMatrixKey )
                    .ToCvMat( *m_intrinsicMatrix );
    const bool distortionCoeffsValid = cameraIntrisicConfig
                    .GetKeyValue( distortionCoefficientsKey )
                    .ToCvMat( *m_distortionCoeffs );
    const bool inverseCoeffsValid = cameraIntrisicConfig
                    .GetKeyValue( invDistortionCoefficientsKey )
                    .ToCvMat( *m_inverseCoeffs );

    if ( !calibrationWasSuccessful ||
         !cameraMtxValid ||
         !distortionCoeffsValid ||
         !inverseCoeffsValid )
    {
        m_errorString = QObject::tr( "The specified camera is not correctly calibrated."
                                     " Please return to the Calibrate Camera tab,"
                                     " repeat the calibration then try again",
                                     "ExtrinsicCalibrationAlgorithm" );
        successful = false;
    }
    return successful;
}

bool ExtrinsicCalibrationAlgorithm::LoadCameraCalibration( const WbConfig& config )
{
    WbConfig cameraCfg;

    bool successful = LoadCameraConfig( config, &cameraCfg );

    if ( successful )
    {
        successful = PopulateCameraCalibrationMatrices( cameraCfg );
    }

    return successful;
}

bool ExtrinsicCalibrationAlgorithm::LoadCameraConfig( const WbConfig& calibCfg,
                                                      WbConfig* const cameraCfg )
{
    bool successful = true;

    if ( cameraCfg == 0 )
    {
        m_errorString =
            QObject::tr( "Error - cameraCfg cannot be null",
                         "ExtrinsicCalibrationAlgorithm" );
        successful = false;
    }

    if ( successful )
    {
        // Load calibration
        const WbConfig camPosCfg( calibCfg.GetParent() );
        const KeyId cameraId( camPosCfg
                        .GetKeyValue( CameraPositionSchema::cameraIdKey ).ToQString() );
        Collection cameras( CamerasCollection() );
        cameras.SetConfig( calibCfg );
        *cameraCfg = cameras.ElementById( cameraId );

        if ( cameraCfg->IsNull() )
        {
            m_errorString = QObject::tr( "Error - Configuration missing for camera: %1!" )
                                       .arg( cameraId );
            successful = false;
        }
    }

    return successful;
}

bool ExtrinsicCalibrationAlgorithm::CalibrateCamera( const CvSize& gridSize,
                                                     const double squareSizeInCm )
{
    bool successful = true; /// @todo Shouldn't assume success here!

    CvMat* objectPoints = createCalibrationObject( gridSize.width,
                                                   gridSize.height,
                                                   squareSizeInCm );

    // Compute extrinsic camera parameters
    m_rot = cvCreateMat( 3, 3, CV_32F );
    m_trans  = cvCreateMat( 1, 3, CV_32F );

    LOG_TRACE("Computing extrinsic params...");

    computeExtrinsicParameters( objectPoints,
                                m_imagePoints,
                                m_intrinsicMatrix,
                                m_distortionCoeffs,
                                m_rot,
                                m_trans );

    LOG_INFO("Rotation matrix:");
    logCvMat32F(m_rot);

    LOG_INFO("Translation matrix:");
    logCvMat32F(m_trans);

    cvReleaseMat( &objectPoints );

    return successful;
}

void ExtrinsicCalibrationAlgorithm::ComputeSquareSize()
{
    CvPoint2D32f* pPts2D = ((CvPoint2D32f*)m_imagePoints->data.fl);

    CvPoint2D32f p1 = *pPts2D++;
    CvPoint2D32f p2 = *pPts2D++;

    double x = p1.x - p2.x;
    double y = p1.y - p2.y;

    m_squareSizePx = sqrt(x*x + y*y);
}

void ExtrinsicCalibrationAlgorithm::RecordCalibration( WbConfig config )
{
    using namespace ExtrinsicCalibrationSchema;

    const QString currentDateString(
        QDate::currentDate().toString( QObject::tr( "d MMMM yyyy", "Calibration Date Format" ) ) );
    const QString currentTimeString(
        QTime::currentTime().toString( QObject::tr( "h.mmap", "Calibration Time Format" ) ) );

    config.SetKeyValue( calibrationSuccessfulKey, KeyValue::from( true ) );
    config.SetKeyValue( calibrationDateKey, KeyValue::from( currentDateString ) );
    config.SetKeyValue( calibrationTimeKey, KeyValue::from( currentTimeString ) );

    config.SetKeyValue( rotationMatrixKey, KeyValue::from( *m_rot ) );
    config.SetKeyValue( translationKey, KeyValue::from( *m_trans ) );

    config.SetKeyValue( gridSquareSizeInPxKey, KeyValue::from( m_squareSizePx ) );
}

/** @todo @bug here and in the Intrinsic algorithm need to be sure grid size >= 3x3
 * or OpenCV throws exception.  Currently limited by GUI, but should check here too.
 *
 */
bool ExtrinsicCalibrationAlgorithm::Run( WbConfig config )
{
    using namespace ExtrinsicCalibrationSchema;

    const QString imageFileName( config.GetAbsoluteFileNameFor(
                                        config.GetKeyValue( calibrationImageKey )
                                                                    .ToQString() ) );

    const double squareSizeInCm = config.GetKeyValue( gridSquareSizeInCmKey ).ToDouble();
    const int    gridWidth      = config.GetKeyValue( gridColumnsKey ).ToInt();
    const int    gridHeight     = config.GetKeyValue( gridRowsKey ).ToInt();
    const CvSize gridSize( cvSize( gridWidth, gridHeight ) );

    bool successful = LoadInputImage( imageFileName );

    if ( successful )
    {
        successful = DetectChessBoardPattern( gridSize );
    }

    WbConfig cameraConfig;

    if ( successful )
    {
        successful = LoadCameraCalibration( config );
    }

    if ( successful )
    {
        ComputeSquareSize();
    }

    if ( successful )
    {
#if WRONG
        successful = CalibrateCamera( gridSize, m_squareSizePx );
#else
        successful = CalibrateCamera( gridSize, squareSizeInCm );
#endif
    }

    if ( successful )
    {
        RecordCalibration( config );
    }
    else
    {
        Message::Show( 0,
                       QObject::tr( "Extrinsic Calibration" ),
                       m_errorString,
                       Message::Severity_Critical );

    }

    // Clean-up
    cvReleaseMat(&m_imagePoints);
    cvReleaseMat(&m_intrinsicMatrix);
    cvReleaseMat(&m_distortionCoeffs);
    cvReleaseMat(&m_inverseCoeffs);
    cvReleaseImage(&m_inputImageAsGrey);

    cvReleaseMat(&m_rot);
    cvReleaseMat(&m_trans);

    return successful;
}
