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

#ifndef CALIBRATIONIMAGEVIEWMAPPER_H
#define CALIBRATIONIMAGEVIEWMAPPER_H

#include "ConfigKeyMapper.h"
#include "KeyName.h"
#include "ImageView.h"

#include "Collection.h"
#include "CamerasCollection.h"

#include "CameraPositionSchema.h"
#include "ExtrinsicCalibrationSchema.h"
#include "CalibrationSchema.h"

#include "GroundPlaneUtility.h"

#include <opencv/highgui.h>

class CalibrationImageViewMapper: public ConfigKeyMapper
{
    Q_OBJECT

public:
    CalibrationImageViewMapper( ImageView& imageView ) :
        ConfigKeyMapper( ExtrinsicCalibrationSchema::calibrationImageKey ),
        m_view     ( imageView ),
        m_unwarped ( false )
    {
    }

    virtual void CommitData( WbConfig& config )
    {
        Q_UNUSED(config);
    }

    virtual void SetConfig( const WbConfig& config )
    {
        m_config = config;

        ShowImage();
    }

    void ShowImage()
    {
        if (m_unwarped)
        {
            ShowUnwarped();
        }
        else
        {
            ShowWarped();
        }
    }

    void ToggleWarping( bool unwarped )
    {
        m_unwarped = unwarped;

        ShowImage();
    }

private:
    void ShowWarped()
    {
        const QString calibrationImageRelativeFileName(
            m_config.GetKeyValue( ExtrinsicCalibrationSchema::calibrationImageKey )
            .ToQString() );
        const QString calibrationImageAbsoluteFileName(
            m_config.GetAbsoluteFileNameFor( calibrationImageRelativeFileName ) );

        m_view.SetImage( calibrationImageAbsoluteFileName );
        m_view.update();
    }

    void ShowUnwarped()
    {
        const WbConfig camPosCfg( m_config.GetParent() );
        const KeyId cameraId( camPosCfg
                        .GetKeyValue( CameraPositionSchema::cameraIdKey ).ToQString() );

        Collection cameras( CamerasCollection() );
        cameras.SetConfig( m_config );

        const WbConfig cameraCfg = cameras.ElementById( cameraId );

        if ( !cameraCfg.IsNull() )
        {
            CvMat* intrinsicMatrix = cvCreateMat( 3, 3, CV_32F );
            CvMat* distortionCoeffs = cvCreateMat( 5, 1, CV_32F );
            CvMat* inverseCoeffs = cvCreateMat( 5, 1, CV_32F );

            CvMat* rotMat = cvCreateMat( 3, 3, CV_32F );
            CvMat* trans = cvCreateMat( 1, 3, CV_32F );

            const WbConfig cameraIntrisicConfig( cameraCfg.GetSubConfig( CalibrationSchema::schemaName ) );

            const bool cameraMtxValid = cameraIntrisicConfig
                            .GetKeyValue( CalibrationSchema::cameraMatrixKey )
                            .ToCvMat( *intrinsicMatrix );
            const bool distortionCoeffsValid = cameraIntrisicConfig
                            .GetKeyValue( CalibrationSchema::distortionCoefficientsKey )
                            .ToCvMat( *distortionCoeffs );
            const bool inverseCoeffsValid = cameraIntrisicConfig
                            .GetKeyValue( CalibrationSchema::invDistortionCoefficientsKey )
                            .ToCvMat( *inverseCoeffs );

            const bool rotMatValid = m_config
                            .GetKeyValue( ExtrinsicCalibrationSchema::rotationMatrixKey )
                            .ToCvMat( *rotMat );
            const bool transValid = m_config
                            .GetKeyValue( ExtrinsicCalibrationSchema::translationKey )
                            .ToCvMat( *trans );

            if ( cameraMtxValid &&
                 distortionCoeffsValid &&
                 inverseCoeffsValid &&
                 rotMatValid &&
                 transValid )
            {
                const QString calibrationImageRelativeFileName(
                    m_config.GetKeyValue( ExtrinsicCalibrationSchema::calibrationImageKey )
                    .ToQString() );
                const QString calibrationImageAbsoluteFileName(
                    m_config.GetAbsoluteFileNameFor( calibrationImageRelativeFileName ) );

                IplImage* imgGrey = cvLoadImage( calibrationImageAbsoluteFileName.toAscii(),
                                                 CV_LOAD_IMAGE_GRAYSCALE );
                if ( imgGrey )
                {
                    IplImage* imgWarp = GroundPlaneUtility::unwarpGroundPlane( imgGrey,
                                                                               intrinsicMatrix,
                                                                               distortionCoeffs,
                                                                               inverseCoeffs,
                                                                               rotMat,
                                                                               trans,
                                                                               &m_offset );

                    QImage qimage;

                    const int DONT_FLIP = 0;
                    int flipFlag = DONT_FLIP;

                    IplImage* img = cvCreateImage( cvSize( imgWarp->width,
                                                           imgWarp->height ), IPL_DEPTH_8U, 3 );
                    cvConvertImage( imgWarp, img );

                    // Convert from IplImage to QImage
                    const QSize imgSize( img->width, img->height );
                    qimage = QImage( imgSize, QImage::Format_RGB888 );


                    CvMat mtxWrapper;
                    cvInitMatHeader( &mtxWrapper,
                                     img->height,
                                     img->width,
                                     CV_8UC3,
                                     qimage.bits() );

                    cvConvertImage( img, &mtxWrapper, flipFlag );
                    cvReleaseImage( &img );

                    m_view.SetImage( qimage );
                    m_view.update();
                }
            }
        }
    }

private:
    ImageView& m_view;

    CvPoint2D32f m_offset;

    WbConfig m_config;

    bool m_unwarped;
};

#endif
