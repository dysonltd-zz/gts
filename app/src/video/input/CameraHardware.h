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

#ifndef CAMERAHARDWARE_H_
#define CAMERAHARDWARE_H_

#include "CameraApi.h"
#include "CameraDescription.h"
#include <memory>
#include <vector>

class CameraHardware
{
public:
    CameraHardware();
    virtual ~CameraHardware();

    typedef std::unique_ptr<CameraApi> CameraApiPtr;
    void AddApi(CameraApiPtr& apiToAdd);

    const CameraApi::CameraList EnumerateConnectedCameras() const;

    const CameraDescription
    GetCameraDescriptionFromUniqueId(const std::wstring& uniqueId) const;

private:
    std::vector< std::unique_ptr< CameraApi > > m_apis;
};

#endif // CAMERAHARDWARE_H_
