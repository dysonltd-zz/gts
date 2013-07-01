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

#ifndef GROUNDTRUTHUI_H
#define GROUNDTRUTHUI_H

#include <opencv/cv.h>

#include <QtGui/qimage.h>

class RobotTracker;
class CoverageSystem;
class RobotMetrics;
class CameraCalibration;

struct CalibViewArgs
{
	CalibViewArgs( const CameraCalibration* cal, const RobotMetrics* met ) :
		m_cal ( cal ),
		m_met ( met )
	{};

	const CameraCalibration* m_cal;
	const RobotMetrics* m_met;
};

/**
	This file contains drawing functionas and OpenCV-style mouse call-backs
	used for the ground truth system user interface.
**/
QImage showRobotTrackUndistorted( IplImage* img, const RobotTracker* tracker, int flip=0 );
QImage showRobotTrack(const RobotTracker* tracker, bool tracking );

#endif // GROUNDTRUTHUI_H
