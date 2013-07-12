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

#ifndef ANGLES_H
#define ANGLES_H

#include "TrackHistory.h"

namespace Angles
{
    // Calculates poses of th2 at the times of th1 - dt, returns poses at res
    void InterpolatePose( double dt,
                          TrackHistory::TrackLog *th1,
                          TrackHistory::TrackLog *th2,
                          TrackHistory::TrackLog *res );

    double NormAngle( double fi );///< normalize angle to (-pi,pi>

    double DiffAngle( double a, double b );///< calculate the shorter difference between 2 angles

    //interpolate; set angle=true for angles
    double Interpolate( double t1,
                        double v1,
                        double t2,
                        double v2,
                        double t,
                        bool angle );
}

#endif // ANGLES_H
