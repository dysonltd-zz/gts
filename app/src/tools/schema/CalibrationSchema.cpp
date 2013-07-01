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

#include "CalibrationSchema.h"

namespace CalibrationSchema
{
    const KeyName schemaName                  ( "cameraCalibration" );

    const KeyName gridGroup                   ( "calibrationGrid" );
    const KeyName gridSquareSizeInCmKey       ( "squareSizeCm" );
    const KeyName gridRowsKey                 ( "rows" );
    const KeyName gridColumnsKey              ( "columns" );

    const KeyName imageGroup                  ( "calibrationImage" );
    const KeyName imageFileKey                ( "calibrationImageFile" );
    const KeyName imageErrorKey               ( "calibrationImageError" );
    const KeyName imageReprojectedPointsKey   ( "calibrationImageReprojectedPoints" );

    const KeyName advancedGroup               ( "advancedIntrinsicCalibration" );
    const KeyName noTangentialDistortionKey   ( "noTangentialDistortion" );
    const KeyName fixPrincipalPointKey        ( "fixPrincipalPoint" );
    const KeyName flipImagesKey               ( "flipImages" );
    const KeyName shouldFixAspectRatioKey     ( "shouldFixAspectRatio" );
    const KeyName fixedAspectRatioKey         ( "fixedAspectRatio" );

    const KeyName resultsGroup                ( "calibrationResults" );
    const KeyName calibrationDateKey          ( "calibrationDate" );
    const KeyName calibrationTimeKey          ( "calibrationTime" );
    const KeyName calibrationSuccessfulKey    ( "calibrationSuccessful" );
    const KeyName rowsUsedForCalibrationKey   ( "rowsUsedForCalibration" );
    const KeyName columnsUsedForCalibrationKey( "columnsUsedForCalibration" );
    const KeyName imageHeightKey              ( "imageHeight" );
    const KeyName imageWidthKey               ( "imageWidth" );
    const KeyName cameraMatrixKey             ( "cameraMatrix" );
    const KeyName distortionCoefficientsKey   ( "distortionCoefficients" );
    const KeyName invDistortionCoefficientsKey( "invDistortionCoefficients" );
    const KeyName avgReprojectionErrorKey     ( "avgReprojectionError" );
}



