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

#ifndef SCHEMAKEYS_CALIBRATION_H_
#define SCHEMAKEYS_CALIBRATION_H_

#include "KeyName.h"

namespace CalibrationSchema
{
    extern const KeyName schemaName;

    extern const KeyName gridGroup;
    extern const KeyName gridSquareSizeInCmKey;
    extern const KeyName gridRowsKey;
    extern const KeyName gridColumnsKey;

    extern const KeyName imageGroup;
    extern const KeyName imageFileKey;
    extern const KeyName imageErrorKey;
    extern const KeyName imageReprojectedPointsKey;

    extern const KeyName advancedGroup;
    extern const KeyName noTangentialDistortionKey;
    extern const KeyName fixPrincipalPointKey;
    extern const KeyName flipImagesKey;
    extern const KeyName shouldFixAspectRatioKey;
    extern const KeyName fixedAspectRatioKey;

    extern const KeyName resultsGroup;
    extern const KeyName calibrationDateKey;
    extern const KeyName calibrationTimeKey;
    extern const KeyName calibrationSuccessfulKey;
    extern const KeyName rowsUsedForCalibrationKey;
    extern const KeyName columnsUsedForCalibrationKey;
    extern const KeyName imageHeightKey;
    extern const KeyName imageWidthKey;
    extern const KeyName cameraMatrixKey;
    extern const KeyName distortionCoefficientsKey;
    extern const KeyName invDistortionCoefficientsKey;
    extern const KeyName avgReprojectionErrorKey;
}

#endif // SCHEMAKEYS_CALIBRATION_H_


