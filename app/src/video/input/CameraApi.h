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

#ifndef CAMERAAPI_H_
#define CAMERAAPI_H_

#include <vector>
#include <string>

class CameraDescription;
class VideoSequence;


/** @brief Abstraction of a camera API.
 *
 */
class CameraApi
{
public:
    typedef std::vector< CameraDescription > CameraList; ///< Type representing a list of cameras.

    virtual ~CameraApi() {}

    /** @brief Find all cameras this API can operate
     *
     *  @return The list of all cameras the API knows about.
     */
    virtual const CameraList EnumerateCameras() const = 0;

    /** @brief Create a VideoSequence for the specified camera.
     *
     *  @param apiIndex The index of the camera in this API (should be ultimately derived from
     *  a CameraDescription returned from EnumerateCameras().
     *  @return A newly-allocated VideoSequence. The caller takes ownership.
     */
    virtual VideoSequence* const CreateVideoSequenceForCamera( const CameraDescription& camera ) const = 0;
};

#endif // CAMERAAPI_H_
