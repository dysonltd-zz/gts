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

#include "CalibrationImageGridMapper.h"

#include <opencv/cv.h>

CalibrationImageGridMapper::CalibrationImageGridMapper( ImageGrid& imageGrid ) :
    ConfigKeyMapper( CalibrationSchema::imageFileKey ),
    m_grid( imageGrid ),
    m_image(),
    m_notFoundImage(":/image_not_found.png")
{
}

void CalibrationImageGridMapper::CommitData( WbConfig& config )
{
    Q_UNUSED(config);
}

void CalibrationImageGridMapper::SetConfig( const WbConfig& config )
{
    if ( !m_currentImageId.isEmpty() )
    {
        UpdateImage( config );
        OverlayCornersIfPossible( config );
        UpdateGrid();
    }
}

bool CalibrationImageGridMapper::ImageIsFound() const
{
    return !m_image.isNull();
}

void CalibrationImageGridMapper::OverlayCornersIfPossible( const WbConfig& config )
{
    if (ImageIsFound())
    {
        using namespace CalibrationSchema;
        cv::Mat cameraMtx(3, 3, CV_64F);
        cv::Mat distortionCoeffs(5, 1, CV_64F);
        bool successful = !config.GetKeyValue(rowsUsedForCalibrationKey).IsNull() &&
                          !config.GetKeyValue(columnsUsedForCalibrationKey).IsNull();

        const cv::Size gridSize( config.GetKeyValue( columnsUsedForCalibrationKey ).ToInt(),
                                 config.GetKeyValue( rowsUsedForCalibrationKey ).ToInt() );

        if (successful)
        {
            successful = config.GetKeyValue(distortionCoefficientsKey).TocvMat(distortionCoeffs);
        }

        if (successful)
        {
            successful = config.GetKeyValue(cameraMatrixKey).TocvMat(cameraMtx);
        }

        if (successful)
        {
            std::vector<cv::Point2f> reprojectedPoints;
            reprojectedPoints.reserve(gridSize.area());
            const bool canGetPoints = config.GetKeyValue(imageReprojectedPointsKey, m_currentImageId)
                                      .ToStdVectorOfCvPoint2f(reprojectedPoints);
            if (canGetPoints)
            {
                cv::Mat imageMat(m_image.height(), m_image.width(), CV_8UC3, m_image.bits());
                cv::drawChessboardCorners(imageMat, gridSize, reprojectedPoints, true);
            }
        }
    }
}

void CalibrationImageGridMapper::UpdateImage( const WbConfig& config )
{
    const KeyValue cameraImageFile(config.GetKeyValue(CalibrationSchema::imageFileKey, m_currentImageId));
    const QString fileName(config.GetAbsoluteFileNameFor(cameraImageFile.ToQString()));

    m_image = QImage(fileName).convertToFormat(QImage::Format_RGB888);
}

void CalibrationImageGridMapper::SetCurrentImage(const KeyId & imageId)
{
    m_currentImageId = imageId;
}

QImage CalibrationImageGridMapper::GetRealOrNotFoundImage() const
{
    if (ImageIsFound())
    {
        return m_image;
    }

    return m_notFoundImage;
}

void CalibrationImageGridMapper::UpdateGrid()
{
    m_grid.Clear();
    m_grid.AddImage(GetRealOrNotFoundImage());
}
