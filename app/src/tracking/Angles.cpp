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

#include "Angles.h"

#include "RobotTracker.h"
#include "MathsConstants.h"

#include "Logging.h"

#include <iostream>
#include <fstream>

using namespace std;

//operation on angles - put them into macros as inline if they wokr OK.
/** Normalize angles - to the range of <pi,-pi)
* Watch out when calculating Jacobians!
*/
double norm_angle(double fi)
{
    double b = floor(fi/(2.0*D_PI));
    double c = fi - b*2.0*D_PI;
    if(c > D_PI)
    {
        c -= 2.0*D_PI;
    }
    return c;
    //matlab: a=[-5*pi:0.001:5*pi];b=floor(a/(2*pi));
    //c = a-b*2*pi;i=find(c>pi);c(i)=c(i)-2*pi;clf;plot(a,c,'.-');grid;
}//double norm_angle(double fi)

/** Calculate the shorter difference fi1-fi2 of angles
* Watch out when calculating Jacobians!
* make it more efficient later!
*/
double diff_angle(double a, double b)
{
    //should use norm_angle(a-b);
    a = norm_angle(a);
    b = norm_angle(b);
    if(a<0)
        a += 2.0*D_PI;
    if(b<0)
        b += 2.0*D_PI;

    double delta = a - b;

    if(fabs(delta) > D_PI)//did we get the longer difference?
    {
        if( delta >0)
            delta -= 2.0*D_PI;
        else
            delta +=  2.0*D_PI;
    }
    return(delta);
}//diff_angle

/**interpolates between (t1,v1), (t2,v2) at time t. If angle = true then
* v1,v2 are treated as angles (normalization - special way to calculate the diff.)
* \return interpolated value
*/
double interpolate(double t1,double v1,double t2, double v2, double t, bool angle)
{
    double v;

    if(angle)
    {
        v1 = norm_angle(v1);
        v2 = norm_angle(v2);
        ///the interpolation has to work on the shorter angle
        if( fabs(v1-v2)>D_PI) ///longer angle?
        {
            if(v1<0)
                v1 += 2.0*D_PI;
            if(v2<0)
                v2 += 2.0*D_PI;
        }
    }

    if(fabs(t1-t2)>0.00001)//large enough not to get an unstable division?
    {
        v = v1 + (v2-v1)*(t-t1)/(t2-t1);
    }
    else
    {
        v = (v1+v2)/2.0; //can't divide -> do the best

        LOG_WARN(QObject::tr("Interpolate - difference (%1) too small.").arg(t1-t2));
    }
    if(angle)
    {
        v = norm_angle(v);
    }
    return(v);

}//double interpolate
/** Calculates poses of th2 at the times of th1 - dt, returns poses at res
*/
void interpolate_pose(double dt,TrackHistory *th1, TrackHistory *th2,TrackHistory *res)
{
    res->clear();
    for(unsigned int i = 0;i < th1->size();i++)
    {
        double t = th1->at(i).t();
        double min_t = 1000000;
        int min_j=-1;
        //find the closest smaller time
        for(unsigned  int j = 0; j < (th2->size()-1);j++)
        {
            double t2 = t - (th2->at(j).t() + dt);
            if( t2 >= 0 && t2 < min_t )
            {
                min_t = t2;
                min_j = j;
            }
        }//for j
        if( min_t > 2.0 ) // time difference should be smaller than 2s
        {
            LOG_WARN(QObject::tr("Interpolate - no assoc at %1.").arg(t));

            res->push_back( TrackEntry( cvPoint2D32f(0,0), 0, 0, t, 0 ) );
        }
        else //we can interpolate between min_j and min_j+1
        {

            double x,y,th,wgm;

            x = interpolate(th2->at(min_j).t()+dt,  th2->at(min_j).x(),
                            th2->at(min_j+1).t()+dt,th2->at(min_j+1).x(),
                            t, false);

            y = interpolate(th2->at(min_j).t()+dt,  th2->at(min_j).y(),
                            th2->at(min_j+1).t()+dt,th2->at(min_j+1).y(),
                            t, false);

            th = interpolate(th2->at(min_j).t()+dt,  th2->at(min_j).th(),
                             th2->at(min_j+1).t()+dt,th2->at(min_j+1).th(),
                             t, true);

/*			if(dt>0 && fabs(th)<0.1)
			{
				LOG_FATAL("The bug!");
	            th = interpolate(th2->at(min_j).t()+dt,  th2->at(min_j).th(),
                             th2->at(min_j+1).t()+dt,th2->at(min_j+1).th(),
                             t, true);
			}*/

            wgm = interpolate(th2->at(min_j).t()+dt,  th2->at(min_j).wgm(),
                             th2->at(min_j+1).t()+dt,th2->at(min_j+1).wgm(),
                             t, true);

            res->push_back( TrackEntry( cvPoint2D32f((float)x,(float)y), (float)th,(float)th2->at(min_j).e(), t, (float)wgm ) );
        }
    }//for i

}//void interpolate_pose
