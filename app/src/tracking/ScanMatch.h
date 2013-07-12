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

namespace ScanMatch
{
    /**
	    Simple struct for storing results of scan matching process
    **/
    struct ScanPose
    {
    	ScanPose()
    	{
    	};

	    ScanPose( float x, float y, float th, float e ) :
	      dx(x),
	      dy(y),
	      dth(th),
	      error(e)
        {
        };

	    float dx;    ///< Translational offset in x-axis
	    float dy;    ///< Translational offset in y-axis.
	    float dth;   ///< Rotation angle in radians.
	    float error; ///< Error in alignment resulting from this pose
    };

    void ScanRemoveStationaryPoints( const TrackHistory::TrackLog& a,
                                     TrackHistory::TrackLog& r,
                                     float thresh );

    void ScanRemoveBadPoints( const TrackHistory::TrackLog& a,
                              TrackHistory::TrackLog& r,
                              float thresh );

    void ScanAssociationNearest( const TrackHistory::TrackLog& a1,
                                 const TrackHistory::TrackLog& b1,
						         TrackHistory::TrackLog& a2,
						         TrackHistory::TrackLog& b2,
						         float thresh,
						         float timeOffset );

    void ScanAssociationInterpolated( const TrackHistory::TrackLog& a1,
                                      const TrackHistory::TrackLog& b1,
							          TrackHistory::TrackLog& a2,
							          TrackHistory::TrackLog& b2,
							          float thresh,
							          float timeOffset );

    ScanPose ScanComputePose( const TrackHistory::TrackLog& a,
                              const TrackHistory::TrackLog& b );

    ScanPose ScanComputePoseWeighted( const TrackHistory::TrackLog& a,
                                      const TrackHistory::TrackLog& b );

    float ScanComputeError( ScanPose pose,
                            const TrackHistory::TrackLog& a,
                            const TrackHistory::TrackLog& b );

    ScanPose ScanMatches( const TrackHistory::TrackLog& a,
                          const TrackHistory::TrackLog& b,
				          TrackHistory::TrackLog& asc_a,
				          TrackHistory::TrackLog& asc_b,
				          float thresh,
    				      float dt );

    float ScanMatchTemporal( const TrackHistory::TrackLog& a,
                             const TrackHistory::TrackLog& b,
                             TrackHistory::TrackLog& asc_a,
                             TrackHistory::TrackLog& asc_b,
                             ScanMatch::ScanPose& pose,
					         float timeThresh );

    void ScanCombine( const TrackHistory::TrackLog& a1,
                      const TrackHistory::TrackLog& b1,
                      double timeThresh,
				      TrackHistory::TrackLog& avg );

    void ScanMergeAndRemove( const TrackHistory::TrackLog& a1,
                             const TrackHistory::TrackLog& b1,
                             double timeThresh,
				             TrackHistory::TrackLog& avg );

    void ScanAverage( const TrackHistory::TrackLog& a1,
                      const TrackHistory::TrackLog& b1,
                      double fps,
				      TrackHistory::TrackLog& avg );

    void ScanTimeShift( const TrackHistory::TrackLog& in,
                        TrackHistory::TrackLog& out,
                        double offset );

    float ScanOverlap( const TrackHistory::TrackLog& a,
                       const TrackHistory::TrackLog& b,
                       float thresh );
}

#endif // SCANMATCH_H
