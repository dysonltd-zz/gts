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

#include "TrackRobotSchema.h"

namespace TrackRobotSchema
{
    const KeyName schemaName("trackerOutput");

    const KeyName robotIdKey("robotId");

    const KeyName trackedVideoGroup  ("trackedVideo");
    const KeyName videoFileNameKey   ("videoFileName");
    const KeyName cameraPositionIdKey("cameraPositionId");

    namespace GlobalTrackingParams
    {
        const KeyName group           ("trackingParameters");
        const KeyName biLevelThreshold("biLevelThreshold");
        const KeyName nccThreshold    ("nccThreshold");
        const KeyName resolution      ("resolution");
    }

    namespace PerCameraTrackingParams
    {
        const KeyName group           ("camTrackingParameters");
        const KeyName positionIdKey   ("camPositionId");
        const KeyName useGlobalParams ("camUseGlobalParams");
        const KeyName biLevelThreshold("camBiLevelThreshold");
        const KeyName nccThreshold    ("camNccThreshold");
        const KeyName resolution      ("camResolution");
    }
}
