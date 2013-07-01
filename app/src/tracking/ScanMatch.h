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

#ifndef SCANMATCH_H
#define SCANMATCH_H

#include "RobotTracker.h"

/**
	Simple struct for storing results of scan matching process
**/
struct ScanPose
{
	ScanPose() {};
	ScanPose( float x, float y, float th, float e ) : dx(x),dy(y),dth(th),error(e) {};
	float dx; ///< Translational offset in x-axis
	float dy; ///< Translational offset in y-axis.
	float dth; ///< Rotation angle in radians.
	float error; ///< Error in alignment resulting from this pose
};

void scan_remove_stationary_points( const TrackHistory& a, TrackHistory& r, float thresh );
void scan_remove_bad_points( const TrackHistory& a, TrackHistory& r, float thresh );

void scan_association_nearest(
						  const TrackHistory& a1, const TrackHistory& b1,
						  TrackHistory& a2, TrackHistory& b2,
						  float thresh, float timeOffset
						  );

void scan_association_interpolated(
							const TrackHistory& a1, const TrackHistory& b1,
							TrackHistory& a2, TrackHistory& b2,
							float thresh, float timeOffset
							);

ScanPose scan_compute_pose(const TrackHistory& a, const TrackHistory& b);
ScanPose scan_compute_pose_weighted(const TrackHistory& a, const TrackHistory& b);

float scan_compute_error( ScanPose pose, const TrackHistory& a, const TrackHistory& b );

ScanPose scan_match(
				const TrackHistory& a, const TrackHistory& b,
				TrackHistory& asc_a, TrackHistory& asc_b,
				float thresh, float dt
				);

float scan_match_temporal(
					const TrackHistory& a, const TrackHistory& b,
					TrackHistory& asc_a, TrackHistory& asc_b,
					ScanPose& pose, float timeThresh
					);

void scan_combine(
				  const TrackHistory& a1, const TrackHistory& b1, double timeThresh,
				  TrackHistory& avg
				  );

void scan_merge_and_remove(const TrackHistory& a1, const TrackHistory& b1, double timeThresh,
				  TrackHistory& avg);

void scan_average(
				  const TrackHistory& a1, const TrackHistory& b1, double fps,
				  TrackHistory& avg
				  );

void scan_time_shift( const TrackHistory& in, TrackHistory& out, double offset );

float scan_overlap( const TrackHistory& a, const TrackHistory& b, float thresh );

#endif // SCANMATCH_H
