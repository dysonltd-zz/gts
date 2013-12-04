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

#include "CameraHardware.h"

#include "CameraDescription.h"

CameraHardware::CameraHardware()
:
m_apis()
{
}

CameraHardware::~CameraHardware()
{
}

void CameraHardware::AddApi(CameraApiPtr& apiToAdd)
{
    m_apis.push_back(CameraApiPtr(apiToAdd.release()));
}

const CameraApi::CameraList CameraHardware::EnumerateConnectedCameras() const
{
    CameraApi::CameraList cameras;
    for (auto api = m_apis.begin(); api != m_apis.end(); ++api)
    {
        CameraApi::CameraList camerasForThisApi((*api)->EnumerateCameras());
        cameras.insert(cameras.end(), camerasForThisApi.begin(), camerasForThisApi.end());
    }
    return cameras;
}

const CameraDescription
CameraHardware::GetCameraDescriptionFromUniqueId(const std::wstring& uniqueId) const
{
    const CameraApi::CameraList list(EnumerateConnectedCameras());
    for (auto camera = list.begin(); camera != list.end(); ++camera)
    {
        if (camera->UniqueId() == uniqueId)
        {
            return *camera;
        }
    }
    return CameraDescription();
}






