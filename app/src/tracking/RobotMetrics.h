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

#ifndef ROBOTMETRICS_H
#define ROBOTMETRICS_H

#include "WbConfig.h"

/**
    This class stores the important physical robot measurements.
    These metrics are required for converting the robots tracked
    position in an image to the real-world position of the robot
    on the ground plane (in centimetres).
**/
class RobotMetrics
{

public:
    RobotMetrics();

    bool LoadMetrics( const WbConfig& metricsCfg,
                      const WbConfig& camPosCalConfig,
                      const WbConfig& trackConfig );

#ifdef SCALED_PIXELS
    void ComputePixelMetrics( float m_squarePx );
#endif

    float GetScaleFactor()         const { return m_scaleFactor; }
    float GetRadiusPx()            const { return m_radiusPx; }
    float GetRadiusCm()            const { return m_radiusCm; }
    float GetHeightPx()            const { return m_heightPx; }
    float GetHeightCm()            const { return m_heightCm; }
    float GetSquareSizePx()        const { return m_squarePx; }
    float GetSquareSizeCm()        const { return m_squareCm; }
    float GetBasePx()              const { return m_basePx;   }
    float GetBaseCm()              const { return m_baseCm;   }
    float GetBrushBarWidthPx()     const { return m_brushBarPx; }
    float GetBrushBarWidthCm()     const { return m_brushBarCm; }
    float GetBrushBarOffsetCm()    const { return m_brushBarOffsetCm; }
    float GetBrushBarOffsetPx()    const { return m_brushBarOffsetPx; }

    float GetXTargetOffsetPx()     const { return m_XTargetOffsetPx; }
    float GetXTargetOffsetCm()     const { return m_XTargetOffsetCm; }
    float GetYTargetOffsetPx()     const { return m_YTargetOffsetPx; }
    float GetYTargetOffsetCm()     const { return m_YTargetOffsetCm; }
    float GetTargetRotationRad()   const { return m_targetRotationRad; }

    float GetResolution()          const { return m_resolution; }

    bool IsValid() const { return m_valid; }

private:
#ifndef SCALED_PIXELS
    void ComputePixelMetrics();
#endif

    float m_scaleFactor;         //!< Scale factor in pixels per cm
    float m_radiusPx;            //!< Robot radius in pixels
    float m_radiusCm;            //!< Robot radius in cm
    float m_heightPx;            //!< Robot height in pixels
    float m_heightCm;            //!< Robot height in cm
    float m_squarePx;            //!< Calibration square size in pixels
    float m_squareCm;            //!< Calibration square size in cm
    float m_resolution;
    float m_basePx;              //!< Radius of the base of the robot in pixels
    float m_baseCm;              //!< Radius of the base of the robot in cm
    float m_brushBarPx;          //!< Width of brush bar in pixels
    float m_brushBarCm;          //!< Width of brush bar in cm
    float m_brushBarOffsetCm;    //!< Lateral offset in cm of brush-bar from centre of robot in direction of travel (+ve is fwd dir)
    float m_brushBarOffsetPx;    //!< Lateral offset in pixels of brush-bar from centre of robot in direction of travel (+ve is fwd dir)
    float m_XTargetOffsetPx;
    float m_XTargetOffsetCm;
    float m_YTargetOffsetPx;
    float m_YTargetOffsetCm;
    float m_targetRotationRad;

    bool  m_valid;
};

#endif // ROBOTMETRICS_H
