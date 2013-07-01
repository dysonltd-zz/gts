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

#ifndef SWEEPER_H
#define SWEEPER_H

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include <stdio.h>

#include "MathsConstants.h"
#include "RobotMetrics.h"
#include "OpenCvUtility.h"

/**
    Facilitate specification of robot sweepers.
**/
struct Sweeper
{
    /**
        Construct sweeper data struct
        by loading params from a file.
    **/
    Sweeper( const char* infoFile )
    {
        FILE* fp = fopen( infoFile, "r" );

        if ( fp )
        {
            fscanf( fp, "x_offset: %f", &x );
            lineSkip( fp );
            fscanf( fp, "y_offset: %f", &y );
            lineSkip( fp );
            fscanf( fp, "angle: %f", &a );
            lineSkip( fp );
            fscanf( fp, "width: %f", &w );
            lineSkip( fp );

            fclose( fp );
        }
    };

    CvPoint2D32f GetLeft( const RobotMetrics& metrics, CvPoint2D32f position, float heading ) const
    {
        heading += 3.14159265359f/2.f;

        // convert to pixels from cm
        float sf = metrics.GetScaleFactor();
        float px = sf * -w/2.f;
        float py;

        // first rotate sweeper locally (x-coord only)
        float ang = a*3.14159265359f/180.f;
        float cosa = cos( ang );
        float sina = sin( ang );
        float tx = cosa*px;
        float ty = sina*px;

        // translate sweeper into position on robot
        tx += x*sf;
        ty += y*sf;

        // rotate by robots heading
        cosa = cos( -heading );
        sina = sin( -heading );
        px = tx*cosa - ty*sina;
        py = tx*sina + ty*cosa;

        return cvPoint2D32f( position.x+px, position.y+py );
    };

    CvPoint2D32f GetRight( const RobotMetrics& metrics, CvPoint2D32f position, float heading ) const
    {
        heading += F_PI/2.f;

        // convert to pixels from cm
        float sf = metrics.GetScaleFactor();
        float px = sf * w/2.f;
        float py;

        // first rotate sweeper locally (x-coord only)
        float ang = a*3.14159265359f/180.f;
        float cosa = cos( ang );
        float sina = sin( ang );
        float tx = cosa*px;
        float ty = sina*px;

        // translate sweeper into position on robot
        tx += x*sf;
        ty += y*sf;

        // rotate by robots heading
        cosa = cos( -heading );
        sina = sin( -heading );
        px = tx*cosa - ty*sina;
        py = tx*sina + ty*cosa;

        return cvPoint2D32f( position.x+px, position.y+py );
    };

    float x,y,a,w;
};

#endif // SWEEPER_H
