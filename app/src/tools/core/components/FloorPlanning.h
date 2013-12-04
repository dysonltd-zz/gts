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

#ifndef FLOORPLANNING_H
#define FLOORPLANNING_H

#include "WbConfig.h"

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include <QImage>
#include <QTableWidget>
#include <QWidget>

namespace FloorPlanning
{
    bool LoadFile(WbConfig config, QString cameraPosition, IplImage** camImg, QString fileName, CvPoint2D32f* offset, bool unWarp);

    bool CheckMappingIsComplete(WbConfig config);

    bool IsBase(WbConfig config, KeyId camId);
    bool IsRef(WbConfig config, KeyId camId);

	std::vector<KeyId> FindRoot(WbConfig config);
	std::vector<KeyId> FindChain(WbConfig config, KeyId camId, KeyId rootId, std::vector<KeyId> mappingChain);

	bool CheckRootMapping(WbConfig config, KeyId rootId);

	void ComputeTransform(WbConfig config, KeyId refId, std::vector<KeyId> chain, CvMat* transform);
}

#endif // FLOORPLANNING_H
