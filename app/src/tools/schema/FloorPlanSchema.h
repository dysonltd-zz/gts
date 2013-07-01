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

#ifndef FLOORPLANSCHEMA_H_
#define FLOORPLANSCHEMA_H_

#include "KeyName.h"

namespace FloorPlanSchema
{
    extern const KeyName schemaName;

	extern const KeyName calGridGroup;
	extern const KeyName calGridSizeKey;
	extern const KeyName calGridRowsKey;
	extern const KeyName calGridColsKey;

	extern const KeyName camGridGroup;
    extern const KeyName camIdKey;
	extern const KeyName camGridRowKey;
	extern const KeyName camGridColKey;
    extern const KeyName camRotationKey;

	extern const KeyName mappingGroup;
    extern const KeyName camera1IdKey;
    extern const KeyName camera2IdKey;
    extern const KeyName camera1ImgKey;
    extern const KeyName camera2ImgKey;
    extern const KeyName homographyKey;

	extern const KeyName transformGroup;
    extern const KeyName cameraIdKey;
    extern const KeyName transformKey;
}

#endif // FLOORPLANSCHEMA_H_
