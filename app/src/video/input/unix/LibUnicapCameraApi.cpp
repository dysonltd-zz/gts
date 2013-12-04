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

#include "LibUnicapCameraApi.h"

#include "VideoCaptureCv.h"
#include "CameraDescription.h"
#include <unicap.h>
#include <sstream>
#include <convert.h>

#define MAX_DEVICES 64

LibUnicapCameraApi::LibUnicapCameraApi()
{
}

LibUnicapCameraApi::~LibUnicapCameraApi()
{
}

/** @copydoc CameraApi::EnumerateCameras()
 *
 */
const CameraApi::CameraList LibUnicapCameraApi::EnumerateCameras() const
{
    CameraApi::CameraList cameraList;
    unicap_device_t device;
    int i = 0;
    while(SUCCESS(unicap_enumerate_devices(NULL, &device, i++)))
    {
        cameraList.push_back(CameraDescription(*this)
                    .WithName       (Convert::ToStdWString(device.model_name))
                    .WithDescription(Convert::ToStdWString(device.vendor_id))
                    .WithUniqueId   (Convert::ToStdWString(device.identifier))
       );
    }
    return cameraList;
}

/** @copydoc CameraApi::CreateVideoSequenceForCamera()
 *
 */
VideoSequence* const LibUnicapCameraApi::CreateVideoSequenceForCamera(const CameraDescription& camera) const
{
    const CameraApi::CameraList cameras(EnumerateCameras());
    for (size_t i = 0; i < cameras.size(); ++i)
    {
        if (cameras.at(i).UniqueId() == camera.UniqueId())
        {
            VideoSequence* sequence = new VideoCaptureCv(CV_CAP_DC1394 + static_cast<int>(i));

            if (sequence->IsSetup())
            {
                return sequence;
            }
            else
            {
                // if libdc1394 is unavailable in opencv highgui library, we fall back to any interface that supports this camera index
                return new VideoCaptureCv(CV_CAP_V4L + static_cast<int>(i));
            }
        }
    }
    return nullptr;
}

