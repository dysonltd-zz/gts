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

#include "ScanMatch.h"

#include "Angles.h"
#include "MathsConstants.h"
#include "TrackHistory.h"

#include <vector>
#include <iostream>

namespace ScanMatch
{
    ///! points closer than MIN_DIST are considered superfluous and removed
    #define MIN_DIST 1.f

    /**
        Removes points whose cross correlation is below a threshold

        Points in r occur in the same order as they occur in a;

        @param a Input tracking data.
        @param r Resulting tracking data after removing points.
    **/
    void ScanRemoveBadPoints( const TrackHistory::TrackLog& a,
                              TrackHistory::TrackLog& r,
                              float thresh )
    {
        r.clear();

        for ( unsigned int i=0; i<a.size()-1; ++i )
        {
            if ( a[i].GetError()>thresh )
            {
                r.push_back(a[i]);
            }
        }
    }

    /**
        Takes points in a and compares them to neighbour.
        If distance is below threshold then the point is removed
        as being stationary (or close to stationary).

        The non-stationary points are placed in r (in the same order
        that they occur in a).

        @param a Input tracking data.
        @param r Resulting tracking data after removing points.
    **/
    void ScanRemoveStationaryPoints( const TrackHistory::TrackLog& a,
                                     TrackHistory::TrackLog& r,
                                     float thresh )
    {
        float dx,dy,d;

        r.clear();

        // remove points which were recorded on the spot
        for ( unsigned int i=0; i<a.size()-1; ++i )
        {
            dx = a[i].x() - a[i+1].x();
            dy = a[i].y() - a[i+1].y();
            d  = sqrtf( dx*dx + dy*dy );

            // only copy element to r if distance to next position is above threshold
            if ( d>thresh )
            {
                r.push_back(a[i]);
            }
        }
    }

    /**
        Associate data points based on nearest timestamps.
        @param thresh threshold for discarding matches that are too far apart temporally to be valid.
        @param timeOffset adds a time offset to b1, allows us to do brute force search for time difference between two data sets.

        Assumption: an element at LATER index can not have an EARLIER time-stamp than an element from a lower index.
    **/
    void ScanAssociationNearest( const TrackHistory::TrackLog& a1,
                                 const TrackHistory::TrackLog& b1,
                                 TrackHistory::TrackLog& a2,
                                 TrackHistory::TrackLog& b2,
                                 float thresh,
                                 float timeOffset )
    {
        // associations are stored in a2 and b2
        a2.clear();
        b2.clear();

        if ( a1.size()==0 || b1.size()==0 ) return;

        unsigned int last = 0; // last index (of b1) at which we found association
        for ( unsigned int i=0; i<a1.size(); ++i )
        {
            float tp = static_cast<float>(a1[i].t());
            float mindt = fabsf( tp - static_cast<float>(b1[last].t())+timeOffset );
            unsigned int mindx = 0;

            for ( unsigned int j=last+1; j<b1.size(); ++j )
            {
                float dt = fabsf( tp - static_cast<float>(b1[j].t())+timeOffset );

                if ( dt < mindt )
                {
                    mindt = dt;
                   mindx = j;
                }
            }

            // If the time difference for the best association is below
            // a threshold then we add it to our list of associations
            if ( mindt < thresh )
            {
                if (mindx>0) last = mindx-1; // match indices must increase monotonically (so we don't need to search whole of b1 each loop)
                a2.push_back( a1[i] );
                b2.push_back( b1[mindx] );
            }
        }
    }

    /**
        Similar to association_nearest but performs interpolation between the two
        data points with time stamps either side to create an artificial data point
        which should reflect the missing data at the actual timestamp more accurately.

        Assumption: an element at LATER index can not have an EARLIER time-stamp than an element from a lower index.

        The overall structure of this function is very similar to scan_combine,
        hence any bugs here are likely reproduced there and any changes made
        here might need to be repeated there.

        @param a1 first input robot trajectory
        @param b1 second input robot trajectory
        @param a2 elements from a1 for which associations were found.
        @param b2 each element is the association found in b1 for the corresponding element from a2.
        @param thresh Time threshold for declaring points to be adjacent
        @param timeOffset A time offset in seconds to be added to the timestamps of b1.
    **/
    void ScanAssociationInterpolated( const TrackHistory::TrackLog& a1,
                                      const TrackHistory::TrackLog& b1,
                                      TrackHistory::TrackLog& a2,
                                      TrackHistory::TrackLog& b2,
                                      float thresh,
                                      float timeOffset )
    {
        a2.clear();
        b2.clear();

#ifndef NDEBUG
        // print timestamps
        std::cerr << "TrackHistory::TrackLog a1: "<<std::endl;
        for ( size_t i = 0; i < a1.size(); ++i )
        {
            std::cerr << a1[i].t() << " ";
        }
        std::cerr << "TrackHistory::TrackLog b1: "<<std::endl;
        for ( size_t i = 0; i < b1.size(); ++i )
        {
            std::cerr << b1[i].t() << " ";
        }
#endif

        unsigned int last = 0;
        for ( unsigned int i=0; i<a1.size(); ++i )
        {
            float    ts = static_cast<float>(a1[i].t()); // time stamp we must find in b1
            int        e_idx = -1; // earlier index
            int        l_idx = -1; // later index
            float    e_dt = 0.f;        // earlier time difference
            float    l_dt = 0.f;        // later time difference

            // for each element of a1 we find the index of the points in b1 which
            // have the closest EARLIER time-stamp and the closest LATER time-stamp
            for ( unsigned int j=last; j<b1.size(); ++j )
            {
                float dt = static_cast<float>(b1[j].t()) - (ts + timeOffset); // earlier timestamps have -ve dt

                if ( dt <= 0.f && // if its an earlier (or equal) timestamp
                    ( e_idx==-1 || dt>e_dt ) // and, we don't have an earlier index or it is a closer earlier index
                    )
                {
                    e_idx = j;
                    e_dt  = dt;
                }
                else if ( dt > 0.f && // if its a later index
                        ( l_idx==-1 || dt<l_dt ) // and, we don't have a later index or it is a closer later index
                        )
                {
                    l_idx = j;
                    l_dt  = dt;
                    j = b1.size(); // can skip rest of loop since we assume time-stamps are increasing
                }
            }

            if ( e_idx != -1 && l_idx != -1 ) // if we found an earlier AND a later index
            {
                if (
                    l_dt-e_dt < thresh
                    ) // and they are both within time threshold of each other
                {
                    // create associated point by interpolation
                    float w = l_dt / (l_dt-e_dt); // MLP's notebook 17/10/2007
                    TrackEntry assoc = TrackHistory::InterpolateEntries( b1[e_idx], b1[l_idx], w );
                    assoc.SetTimeStamp( ts );
                    a2.push_back( a1[i] );
                    b2.push_back( assoc );
                    last = e_idx; // optimisation: earlier match indices must be monotonically increasing
                }
            }
        }
    }

    /**
        Given two track history logs (which must be the same size and
        have corresponding elements in association) computes the pose
        which aligns the two tracks.

        Does not compute the error of the pose estimate, this must be
        done explicitly by calling scan_compute_error(...).

        @param a first input robot trajectory
        @param b second input robot trajectory
        @return ScanPose structure storing the translation and rotation which aligns a to b (error field is not computed)
    **/
    ScanPose ScanComputePose( const TrackHistory::TrackLog& a,
                              const TrackHistory::TrackLog& b )
    {
        float mxa = 0.f; // mean x coord
        float mya = 0.f; // mean y coord
        float mxb = 0.f; // mean x coord
        float myb = 0.f; // mean y coord

        assert( a.size()==b.size() ) ;

        // first compute means
        for ( unsigned int i=0; i<a.size(); ++i )
        {
            mxa += a[i].x();
            mya += a[i].y();
            mxb += b[i].x();
            myb += b[i].y();
        }

        float mul = 1.f/a.size();
        mxa *= mul;
        mya *= mul;
        mxb *= mul;
        myb *= mul;

        // then the cross correlation
        float sxx = 0.f;
        float sxy = 0.f;
        float syx = 0.f;
        float syy = 0.f;

        for ( unsigned int i=0; i<a.size(); ++i )
        {
            sxx += (a[i].x()-mxa) * (b[i].x()-mxb);
            sxy += (a[i].x()-mxa) * (b[i].y()-myb);
            syx += (a[i].y()-mya) * (b[i].x()-mxb);
            syy += (a[i].y()-mya) * (b[i].y()-myb);
        }

        // pose and error computation
        ScanMatch::ScanPose pose;
        pose.dth  = atan2f(sxy-syx,sxx+syy);
        pose.dx  = mxb - ( cos(pose.dth)*mxa  - sin(pose.dth)*mya );
        pose.dy  = myb - ( sin(pose.dth)*mxa  + cos(pose.dth)*mya );

        return pose;
    }

    /**
        Same as scan_compute_pose but weights contributions from features based on the
        correlation error (the closer the correlation to 1.0 the higher the weighting).

        @param a first input robot trajectory
        @param b second input robot trajectory
        @param thresh Threshold for defining weighting function (actually a soft cutoff about this threshold).
        @return ScanPose structure storing the translation and rotation which aligns a to b (error field is not computed)
    **/
    ScanPose ScanComputePoseWeighted( const TrackHistory::TrackLog& a,
                                      const TrackHistory::TrackLog& b )
    {
        if (a.size()==0 || b.size()==0)
        {
            return ScanPose(0.f,0.f,0.f,0.f);
        }

        float mxa = 0.f; // mean x coord
        float mya = 0.f; // mean y coord
        float mxb = 0.f; // mean x coord
        float myb = 0.f; // mean y coord

        assert( a.size()==b.size() ) ;

        // first compute means
        float wsum = 0.f;
        for ( unsigned int i=0; i<a.size(); ++i )
        {
            float w = a[i].GetWeighting();

            mxa += a[i].x() * w;
            mya += a[i].y() * w;
            mxb += b[i].x() * w;
            myb += b[i].y() * w;
            wsum += w;
        }

        float mul = 1.f/wsum;
        mxa *= mul;
        mya *= mul;
        mxb *= mul;
        myb *= mul;

        // then the cross correlation
        float sxx = 0.f;
        float sxy = 0.f;
        float syx = 0.f;
        float syy = 0.f;

        for ( unsigned int i=0; i<a.size(); ++i )
        {
            float w = a[i].GetWeighting();

            sxx += (a[i].x()-mxa) * (b[i].x()-mxb) * w;
            sxy += (a[i].x()-mxa) * (b[i].y()-myb) * w;
            syx += (a[i].y()-mya) * (b[i].x()-mxb) * w;
            syy += (a[i].y()-mya) * (b[i].y()-myb) * w;
        }

        // pose and error computation
        ScanMatch::ScanPose pose;
        pose.dth  = atan2f(sxy-syx,sxx+syy);
        pose.dx  = mxb - ( cos(pose.dth)*mxa  - sin(pose.dth)*mya );
        pose.dy  = myb - ( sin(pose.dth)*mxa  + cos(pose.dth)*mya );

        return pose;
    }

    /**
        Compute mean error when using specified pose to transform a so that it is aligned with b.

        @param pose ScanPose structure storing transformation which aligns a to b.
        @param a first input robot trajectory.
        @param b second input robot trajectory.
        @return the mean error for the specified pose.
    **/
    float ScanComputeError( ScanPose pose,
                            const TrackHistory::TrackLog& a,
                            const TrackHistory::TrackLog& b )
    {
        float err = 0.f;

        float costh = cos(pose.dth);
        float sinth = sin(pose.dth);

        for ( unsigned int i=0; i<a.size(); ++i )
        {
            float nx = costh*a[i].x() - sinth*a[i].y() + pose.dx;
            float ny = sinth*a[i].x() + costh*a[i].y() + pose.dy;
            float ex = nx-b[i].x();
            float ey = ny-b[i].y();
            err += ex*ex + ey*ey;
        }

        return sqrtf( err ) / a.size();
    }

    /**
        Scan match two robot trajectories.
        Both trajectories must be correctly time-stamped as this is used for data association.
        The data association used is stored in corresponding elements of a2 and b2.

        @param a first input robot trajectory
        @param b second input robot trajectory
        @param timeOffset (default=0) allows us to run the matching procedure with different offsets added to
        a's time stamps. From this we can compute the actual time offset between two video sequences.
        @param thresh Time threshold for comparing time-stamps.

        @return ScanPose structure storing the translation and rotation which aligns a to b and the associated mean-squared error
    **/
    ScanPose ScanMatches( const TrackHistory::TrackLog& a,
                          const TrackHistory::TrackLog& b,
                          TrackHistory::TrackLog& a2,
                          TrackHistory::TrackLog& b2,
                          float thresh,
                          float timeOffset )
    {
        TrackHistory::TrackLog a1,am;
        TrackHistory::TrackLog b1,bm;

        ScanRemoveStationaryPoints( a, a1, MIN_DIST );
        //scan_remove_bad_points( am, a1, 0.5 );

        ScanRemoveStationaryPoints( b, b1, MIN_DIST );
        //scan_remove_bad_points( bm, b1, 0.5 );

        ScanAssociationInterpolated( a1, b1, a2, b2, thresh, timeOffset );
        //scan_association_nearest( a1, b1, a2, b2, thresh, timeOffset );

        assert( !a2.empty() );
        assert( !b2.empty() );

        ScanPose pose = ScanComputePoseWeighted(a2, b2);
        pose.error = ScanComputeError( pose, a2, b2 );

        return pose;
    }

    /**
        Repeatedly calls scan_match with different temporal offsets in
        order to determine the time alignment of two video sequences.
        Currently only offsets between -1 and 1 second are evaluated.

        @param pose Pose estimate at the best found time offset.
        @return The temporal offset giving minimal alignment error.
    **/
    float ScanMatchTemporal( const TrackHistory::TrackLog& a,
                             const TrackHistory::TrackLog& b,
                             TrackHistory::TrackLog& asc_a,
                             TrackHistory::TrackLog& asc_b,
                             ScanPose& pose,
                             float timeThresh )
    {
        pose.error = 9999999.f;
        float min_dt = 0.f; // will store the best offset found

        // Repeat scan matching for a number of time offsets from -1 to 1 second
        float dt = -1.f;
        while( dt <= 1.f )
        {
            ScanPose result = ScanMatches( a,
                                           b,
                                           asc_a,
                                           asc_b,
                                           timeThresh,
                                           dt );

            if ( result.error < pose.error )
            {
                pose = result;
                min_dt = dt;
            }
            dt += 0.05f;
        }

        return min_dt;
    }

    /**
        Takes in two tracks and a temporal offset,
        returns a form of 'interpolated-average' between the two tracks

        The two tracks should already have been scan matched and one of the
        tracks transformed into the coordinate system of the other using the result.
        The time offset between the two tracks should either have been computed using
        scan_match_temporal or known, and passed in as the timeOffset parameter.

        The overall structure of this function is very similar to scan_match_interpolated,
        hence any bugs here are likely reproduced there and any changes made here might
        need to be repeated there.

        @param a1 First track to be averaged with b1.
        @param b1 Second track to be averaged with a1.
        @param thresh Time threshold so that we do not interpolate across long gaps
        @param avg The average track.
        @param timeOffset The time offset between the two tracks.

    **/
    void ScanCombine( const TrackHistory::TrackLog& a1,
                      const TrackHistory::TrackLog& b1,
                      double thresh,
                      TrackHistory::TrackLog& avg )
    {
        avg.clear();

        unsigned int last = 0;
        for ( unsigned int i=0; i<a1.size(); ++i )
        {
            float ts = static_cast<float>(a1[i].t()); // time stamp we must find in b1
            int e_idx = -1;        // earlier index
            int l_idx = -1;        // later index
            float e_dt = 0.f;    // earlier time difference
            float l_dt = 0.f;    // later time difference

            // for each element of a1 we find the index of the points in b1 which
            // have the closest EARLIER time-stamp and the closest LATER time-stamp
            for ( unsigned int j=last; j<b1.size(); ++j )
            {
                float dt = static_cast<float>(b1[j].t()) - ts; // earlier timestamps have -ve dt

                if ( dt <= 0.f && // if its an earlier (or equal) timestamp
                    ( e_idx==-1 || dt>e_dt ) // and, we don't have an earlier index or it is a closer earlier index
                    )
                {
                    e_idx = j;
                    e_dt  = dt;
                }
                else if ( dt > 0.f && // if its a later index
                        ( l_idx==-1 || dt<l_dt ) // and, we don't have a later index or it is a closer later index
                        )
                {
                    l_idx = j;
                    l_dt  = dt;
                    j = b1.size(); // can skip rest of loop since we assume time-stamps are increasing
                }
            }

            if (
                e_idx != -1 && l_idx != -1    // if we found an earlier AND a later index
                && l_dt-e_dt < thresh        // and they are within time threshold of each other
                )
            {
                // create associated point by interpolation
                float w = l_dt / (l_dt-e_dt);
                assert( w >= 0.0 );
                assert( w <= 1.0 );
                TrackEntry assoc = TrackHistory::InterpolateEntries( b1[e_idx], b1[l_idx], w );

                // Now average interpolated point with point from a1
                // (but weight the average based weighting functions)
                float wa = a1[i].GetWeighting();
                float wi = assoc.GetWeighting();

                assert( wa >= 0.0 );
                assert( wi >= 0.0 );

                w = wi / (wa+wi);
                assert( w >= 0.0 );
                assert( w <= 1.0 );
                TrackEntry average = TrackHistory::InterpolateEntries( a1[i], assoc, w );
                average.SetTimeStamp( ts );

                avg.push_back( average );

                last = e_idx; // optimisation: earlier match indices must be monotonically increasing
            }
            else
            {
                // If we can't find an association then we just add in the original point
                avg.push_back( a1[i] );
            }
        }
    }

    /**
        Takes in two tracks and merges them.
        If two points have time-stamps that are too
        close (less than timeThresh) then the point
        with the better weight-score is kept and the
        other discarded.

        @param a First track in merge.
        @param b Second track merge.
        @param m Merged track..
        @param timethresh The minimum timeoffset between two points in final merged track.
    **/
    void ScanMergeAndRemove( const TrackHistory::TrackLog& a,
                             const TrackHistory::TrackLog& b,
                             double timeThresh,
                             TrackHistory::TrackLog& m )
    {
        m.clear();

        // set current time-stamp earlier than either a or b's first
        double cts = a[0].t();
        if ( b[0].t() < a[0].t() )
        {
            cts = b[0].t();
        }
        cts -= 1.f;

        unsigned int aidx=0;
        unsigned int bidx=0;

        while ( aidx<a.size() && bidx<b.size() )
        {
            double ats = a[aidx].t();
            double bts = b[bidx].t();
            double diff = fabs(ats - bts);

            if ( diff<timeThresh )
            {
                // We have to decide which to keep and which to lose based on weighting.
                float wa = a[aidx].GetWeighting();
                float wb = b[bidx].GetWeighting();

                if ( wa > wb )
                {
                    m.push_back( a[aidx] );
                }
                else
                {
                    m.push_back( b[bidx] );
                }
            }
            else
            if ( bts<ats )
            {
                // take from b
                m.push_back( b[bidx] );
            }
            else
            {
                // take from a
                m.push_back( a[aidx] );
            }

            cts = m.back().t();

            // increment indices until we get next time-stamp later than cts for each stream
            while(
                aidx < a.size()
                && a[aidx].t() <= cts
                )
            {
                aidx++;
            }
            while (
                bidx < b.size()
                && b[bidx].t() <= cts
                )
            {
                bidx++;
            }
        }

        // If one history was bigger we need to add all remaining elements to merged list
        // (At most one of these for loops will be entered)
        for (; aidx < a.size(); ++aidx )
        {
            m.push_back( a[aidx] );
        }

        for (; bidx < b.size(); ++bidx )
        {
            m.push_back( b[bidx] );
        }
    }

    /**
        Adds an offset to thetime stamp of the input track.
    **/
    void ScanTimeShift( const TrackHistory::TrackLog& in,
                        TrackHistory::TrackLog& out,
                        double offset )
    {
        out.clear();

        for ( unsigned int i=0; i<in.size(); ++i )
        {
            out.push_back( in[i] );
            out[out.size()-1].SetTimeStamp( in[i].t() + offset );
        }
    }

    /**
        Symmetric scan averaging algorithm.

        First creates two scans one of a combined with b, then another of b combined with a.
        The points in these scans are interpolated if possible within the fucntion scan_combine.
        Next we merge the two scans, removing points which are too close in time (closer than timeThresh).

        @param a First track in average
        @param b Second track in average
        @param fps Data rate in Hz (frames per second).
        @param avg The final averaged/merged track is returned through this parameter.
    **/
    void ScanAverage( const TrackHistory::TrackLog& a,
                      const TrackHistory::TrackLog& b,
                      double fps,
                      TrackHistory::TrackLog& avg )
    {
        TrackHistory::TrackLog ab, ba;
        ScanCombine( a, b, 2.0/fps, ab );
        ScanCombine( b, a, 2.0/fps, ba );
        ScanMergeAndRemove( ab, ba, 1.0/(fps), avg );
    }

    /**
        Return a number indicating the proportion of overlap between the two scans.

        This is almost symmetric measure of overlap defined as
        number of associated elements / number of elements in A and B;
        (almost because scan_association_nearest may not be perfectly
        symmetric in A and B - but is as good as for most purposes).

        @param A First scan data.
        @param B Second scan data.
        @param thresh Thresh to pass to association function.
    **/
    float ScanOverlap( const TrackHistory::TrackLog& A,
                       const TrackHistory::TrackLog& B,
                       float thresh )
    {
        TrackHistory::TrackLog assocA, assocB;
        ScanAssociationNearest( A, B, assocA, assocB, thresh, 0.f );
        assert( assocA.size() == assocB.size() );

        return assocA.size() / ((float)(A.size() + B.size()));
    }
}
