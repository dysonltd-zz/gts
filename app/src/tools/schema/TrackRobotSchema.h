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

#ifndef TRACK_ROBOT_SCHEMA_H
#define TRACK_ROBOT_SCHEMA_H

#include "KeyName.h"

namespace TrackRobotSchema
{
    extern const KeyName schemaName;

    extern const KeyName robotIdKey;

    extern const KeyName capturedVideoGroup;
    extern const KeyName videoFileNameKey;
    extern const KeyName cameraPositionIdKey;

    namespace GlobalTrackingParams
    {
        extern const KeyName group;
        extern const KeyName biLevelThreshold;
        extern const KeyName nccThreshold;
        extern const KeyName resolution;
    }

    namespace PerCameraTrackingParams
    {
        extern const KeyName group;
        extern const KeyName showImage;
        extern const KeyName useGlobalParams;
        extern const KeyName biLevelThreshold;
        extern const KeyName nccThreshold;
        extern const KeyName resolution;
    }
}

#endif // TRACK_ROBOT_SCHEMA_H



