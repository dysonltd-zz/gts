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

#include "CameraPositionsCollection.h"

#include "CameraPositionSchema.h"
#include "RoomLayoutSchema.h"

#include "Logging.h"

Collection CameraPositionsCollection()
{
    return Collection( KeyName( "cameraPositions" ),
                       CameraPositionSchema::schemaName );
}

std::vector<WbConfig> GetCameraPositionsConfigs(const WbConfig roomConfig)
{
    Collection camerasPositions = CameraPositionsCollection();
    camerasPositions.SetConfig( roomConfig );

    std::vector<WbConfig> camPosConfigs;
    const WbConfig roomLayoutConfig = roomConfig.GetSubConfig( RoomLayoutSchema::schemaName );
    const KeyValue camPosIds = roomLayoutConfig.GetKeyValue( RoomLayoutSchema::cameraPositionIdsKey );
    const QStringList strCamPosIds = camPosIds.ToQStringList();

    LOG_INFO(QObject::tr("Got %1 camera positions:").arg(strCamPosIds.size()));

    for (int n = 0; n < strCamPosIds.size(); ++n)
    {
        const KeyId camPosId = strCamPosIds.at( n );

        LOG_INFO(QObject::tr(" - %1").arg(camPosId));

        const WbConfig camPosConfig = camerasPositions.ElementById( camPosId );
        camPosConfigs.push_back(camPosConfig);
    }

    return camPosConfigs;
}

