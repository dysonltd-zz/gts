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

#include "CameraTools.h"

#include "CameraSchema.h"
#include "CameraDescription.h"
#include "CameraHardware.h"
#include "Message.h"
#include "WbDefaultKeys.h"
#include "WbConfig.h"

namespace CameraTools
{
    const CameraDescription GetCameraForStreamingIfOk(const CameraHardware& cameraHardware,
                                                       const WbConfig& cameraConfig)
    {
        const QString cameraName(cameraConfig
                                  .GetKeyValue(WbDefaultKeys::displayNameKey)
                                  .ToQString());

        CameraDescription camera;
        bool isValid = !cameraConfig.IsNull();
        QString errorMsg = QObject::tr("No camera specified!");

        const KeyValue uniqueIdKeyValue(
                    cameraConfig.GetKeyValue(KeyName(CameraSchema::uniqueIdKey)));

        if (isValid)
        {
            isValid = !uniqueIdKeyValue.IsNull();
            errorMsg = QObject::tr("Camera %1 has no unique id!")
                                    .arg(cameraName);
        }

        if (isValid)
        {
            camera = cameraHardware.GetCameraDescriptionFromUniqueId(
                                                    uniqueIdKeyValue.ToWString());
            isValid = camera.IsValid();
            errorMsg =  QObject::tr("Camera %1 not connected!")
                                     .arg(cameraName);
        }

        if (!isValid)
        {
            Message::Show(0,
                           QObject::tr("Camera Tools"),
                           errorMsg,
                           Message::Severity_Critical);
        }

        return camera;
    }
}
