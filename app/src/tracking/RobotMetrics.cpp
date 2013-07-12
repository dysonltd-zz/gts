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

#include "RobotMetrics.h"

#include "OpenCvUtility.h"

#include "MathsConstants.h"

#include "RobotMetricsSchema.h"
#include "ExtrinsicCalibrationSchema.h"
#include "TrackRobotSchema.h"
#include "FileUtilities.h"

#include "Logging.h"

#include <QtGlobal>
#include <QObject>

/**
    construct a null RobotMetrics.  Must call LoadMetrics before use
**/
RobotMetrics::RobotMetrics() :
    m_scaleFactor         (0.f),
    m_radiusPx            (0.f),
    m_radiusCm            (0.f),
    m_heightPx            (0.f),
    m_heightCm            (0.f),
    m_squarePx            (0.f),
    m_squareCm            (0.f),
    m_resolution          (0.f),
    m_basePx              (0.f),
    m_baseCm              (0.f),
    m_brushBarCm          (0.f),
    m_brushBarOffsetCm    (0.f),
    m_brushBarOffsetPx    (0.f),
    m_XTargetOffsetPx     (0.f),
    m_XTargetOffsetCm     (0.f),
    m_YTargetOffsetPx     (0.f),
    m_YTargetOffsetCm     (0.f),
    m_targetRotationRad   (0.f),

    m_valid               (false)
{
}

namespace
{
    void SkipAllCommentLines(FILE* fp)
    {
        bool finished = false;
        while (!finished)
        {
            fpos_t startOfLine;
            int result = fgetpos(fp, &startOfLine);
            Q_UNUSED(result);
            assert(result == 0);
            if (fgetc(fp) == '#')
            {
                FileUtilities::LineSkip(fp);
            }
            else
            {
                int result = fsetpos(fp, &startOfLine);
                Q_UNUSED(result);
                assert(result == 0);
                finished = true;
            }
        }
    }
}

/**
    Load measurements from config.
**/
bool RobotMetrics::LoadMetrics( const WbConfig& metricsCfg,
                                const WbConfig& camPosCalCfg,
                                const WbConfig& trackCfg )
{
    m_valid = false;

    m_heightCm = metricsCfg.GetKeyValue( RobotMetricsSchema::dimensionsHeightKey ).ToDouble();
    m_radiusCm = metricsCfg.GetKeyValue( RobotMetricsSchema::targetDiagonalCmKey ).ToDouble()/2.0;

    m_baseCm = metricsCfg.GetKeyValue( RobotMetricsSchema::dimensionsBaseRadiusKey ).ToDouble();
    m_brushBarCm = metricsCfg.GetKeyValue( RobotMetricsSchema::brushBarLengthKey ).ToDouble();
    m_brushBarOffsetCm = metricsCfg.GetKeyValue( RobotMetricsSchema::brushBarOffsetKey ).ToDouble();
    m_XTargetOffsetCm = metricsCfg.GetKeyValue( RobotMetricsSchema::targetOffsetXKey ).ToDouble();
    m_YTargetOffsetCm = metricsCfg.GetKeyValue( RobotMetricsSchema::targetOffsetYKey ).ToDouble();
    m_targetRotationRad = metricsCfg.GetKeyValue( RobotMetricsSchema::targetRotationKey ).ToDouble();

    m_squareCm = camPosCalCfg.GetKeyValue( ExtrinsicCalibrationSchema::gridSquareSizeInCmKey ).ToDouble();
    m_squarePx = camPosCalCfg.GetKeyValue( ExtrinsicCalibrationSchema::gridSquareSizeInPxKey ).ToDouble();

    m_resolution = trackCfg.GetKeyValue(TrackRobotSchema::GlobalTrackingParams::resolution).ToDouble();

    m_targetRotationRad /= 180.f;
    m_targetRotationRad *= MathsConstants::F_PI;

#ifdef SCALED_PIXELS
    ComputePixelMetrics( m_squarePx );
#else
    ComputePixelMetrics();
#endif

    m_valid = true;
    return m_valid;
}

/**
    Compute the scale factor between pixels an centimetres
    and convert metrics (those given in cm only) to pixels.
**/
#ifdef SCALED_PIXELS
void RobotMetrics::ComputePixelMetrics( float squarePx )
{
    m_squarePx          = squarePx;

    m_scaleFactor       = (m_squarePx * m_resolution) / m_squareCm;

    m_radiusPx          = m_scaleFactor * m_radiusCm;
    m_heightPx          = m_scaleFactor * m_heightCm;
    m_basePx            = m_scaleFactor * m_baseCm;
    m_brushBarPx        = m_scaleFactor * m_brushBarCm;
    m_brushBarOffsetPx  = m_scaleFactor * m_brushBarOffsetCm;

    m_XTargetOffsetPx   = m_scaleFactor * m_XTargetOffsetCm;
    m_YTargetOffsetPx   = m_scaleFactor * m_YTargetOffsetCm;

    LOG_INFO("Computed metrics:");
    LOG_INFO(QObject::tr(" - calibration square size: %1 px;").arg(m_squarePx));
    LOG_INFO(QObject::tr(" - robot radius: %1 px;").arg(m_radiusPx));
    LOG_INFO(QObject::tr(" - robot base: %1 px;").arg(m_basePx));
    LOG_INFO(QObject::tr(" - scale factor: %1.").arg(m_scaleFactor));
}
#else
void RobotMetrics::ComputePixelMetrics()
{
    m_squarePx          = m_resolution;

    m_scaleFactor       = m_squarePx / m_squareCm;

    m_radiusPx          = m_scaleFactor * m_radiusCm;
    m_heightPx          = m_scaleFactor * m_heightCm;
    m_basePx            = m_scaleFactor * m_baseCm;
    m_brushBarPx        = m_scaleFactor * m_brushBarCm;
    m_brushBarOffsetPx  = m_scaleFactor * m_brushBarOffsetCm;

    m_XTargetOffsetPx   = m_scaleFactor * m_XTargetOffsetCm;
    m_YTargetOffsetPx   = m_scaleFactor * m_YTargetOffsetCm;

    LOG_INFO("Computed metrics:");
    LOG_INFO(QObject::tr(" - calibration square size: %1 px;").arg(m_squarePx));
    LOG_INFO(QObject::tr(" - robot radius: %1 px;").arg(m_radiusPx));
    LOG_INFO(QObject::tr(" - robot base: %1 px;").arg(m_basePx));
    LOG_INFO(QObject::tr(" - scale factor: %1.").arg(m_scaleFactor));
}
#endif
