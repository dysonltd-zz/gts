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

#include "ExtrinsicCalibrationSchema.h"

namespace ExtrinsicCalibrationSchema
{
    const KeyName schemaName               ( "cameraPositionCalibration" );

    const KeyName gridGroup                ( "calibrationGrid" );
    const KeyName gridSquareSizeInCmKey    ( "squareSizeCm" );
    const KeyName gridRowsKey              ( "rows" );
    const KeyName gridColumnsKey           ( "columns" );

    const KeyName calibrationImageKey      ( "calibrationImage" );

    const KeyName resultsGroup             ( "results");
    const KeyName calibrationDateKey       ( "calibrationDate" );
    const KeyName calibrationTimeKey       ( "calibrationTime" );
    const KeyName calibrationSuccessfulKey ( "calibrationSuccessful" );
    const KeyName rotationMatrixKey        ( "rotationMatrix" );
    const KeyName translationKey           ( "translation" );

    const KeyName reprojectionErrorKey     ( "reprojectionError" );
    const KeyName gridSquareSizeInPxKey    ( "squareSizePx" );
}



