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

#include "RobotMetricsSchema.h"

namespace RobotMetricsSchema
{
    const KeyName schemaName              ( "metrics" );

    const KeyName dimensionsGroup         ( "dimensions" );
    const KeyName dimensionsHeightKey     ( "heightCm" );
    const KeyName dimensionsBaseRadiusKey ( "baseRadiusCm" );

    const KeyName targetGroup             ( "target" );
    const KeyName targetDiagonalCmKey     ( "targetDiagonalCm" );
    const KeyName targetOffsetXKey        ( "targetOffsetXCm" );
    const KeyName targetOffsetYKey        ( "targetOffsetYCm" );
    const KeyName targetRotationKey       ( "targetRotationDegrees" );
    const KeyName targetTypeKey           ( "targetType" );

    const KeyName brushBarGroup           ( "brushBar" );
    const KeyName brushBarLengthKey       ( "lengthCm" );
    const KeyName brushBarOffsetKey       ( "offsetCm" );
}

