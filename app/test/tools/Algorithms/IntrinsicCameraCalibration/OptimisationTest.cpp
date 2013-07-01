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

#include "OptimisationTest.h"
#include <cassert>
#include "LevenbergMarquardt.h"
#include <cmath>

using std::cos;
using std::sin;


const MatrixElementType OptimisationTest::p1[2] = {1.f,0.f};
const MatrixElementType OptimisationTest::p2[2] = {1.f,0.f};
const MatrixElementType OptimisationTest::p3[2] = {1.f,0.f};
const MatrixElementType OptimisationTest::r1[2] = {0.f,0.5f};
const MatrixElementType OptimisationTest::r4[2] = {0.5f,0.f};

void OptimisationTest::operator () ( cv::Mat& r, const cv::Mat& x ) const
{
    assert( r.rows == rangeDim() && r.cols==1 );

    MatrixElementType cosa;
    MatrixElementType sina;

    MatrixElementType y1 = 0;
    MatrixElementType y2 = 0;

    cosa = cos( x.at<MatrixElementType>(0,0) );
    sina = sin( x.at<MatrixElementType>(0,0) );
    y1 += cosa*(p1[0]-r1[0]) - sina*(p1[1]-r1[1]);
    y2 += sina*(p1[0]-r1[0]) + cosa*(p1[1]-r1[1]);

    cosa = cos( x.at<MatrixElementType>(1,0) );
    sina = sin( x.at<MatrixElementType>(1,0) );
    y1 += cosa*p2[0] - sina*p2[1] ;
    y2 += sina*p2[0] + cosa*p2[1] ;

    cosa = cos( x.at<MatrixElementType>(2,0) );
    sina = sin( x.at<MatrixElementType>(2,0) );
    y1 += cosa*p3[0] - sina*p3[1];
    y2 += sina*p3[0] + cosa*p3[1];

    cosa = cos( x.at<MatrixElementType>(3,0) );
    sina = sin( x.at<MatrixElementType>(3,0) );
    y1 += cosa*r4[0] - sina*r4[1];
    y2 += sina*r4[0] + cosa*r4[1];

    r.at<MatrixElementType>(0,0) = y1;
    r.at<MatrixElementType>(1,0) = y2;
}

void OptimisationTest::jacobian( cv::Mat& J, const cv::Mat& x ) const
{
    assert( J.rows==rangeDim() && J.cols == domainDim() );
    assert( x.rows==domainDim() && x.cols==1 );

    MatrixElementType cosa;
    MatrixElementType sina;

    MatrixElementType y1 = 0;
    MatrixElementType y2 = 0;

    int i = 0;

    cosa = cos( x.at<MatrixElementType>(0,0) );
    sina = sin( x.at<MatrixElementType>(0,0) );
    y1 = cosa*(p1[0]-r1[0]) - sina*(p1[1]-r1[1]);
    y2 = sina*(p1[0]-r1[0]) + cosa*(p1[1]-r1[1]);
    J.at<MatrixElementType>(0,i) = -y2;
    J.at<MatrixElementType>(1,i) = y1;
    i++;

    cosa = cos( x.at<MatrixElementType>(1,0) );
    sina = sin( x.at<MatrixElementType>(1,0) );
    y1 = cosa*p2[0] - sina*p2[1] ;
    y2 = sina*p2[0] + cosa*p2[1] ;
    J.at<MatrixElementType>(0,i) = -y2;
    J.at<MatrixElementType>(1,i) = y1;
    i++;

    cosa = cos( x.at<MatrixElementType>(2,0) );
    sina = sin( x.at<MatrixElementType>(2,0) );
    y1 = cosa*p3[0] - sina*p3[1];
    y2 = sina*p3[0] + cosa*p3[1];
    J.at<MatrixElementType>(0,i) = -y2;
    J.at<MatrixElementType>(1,i) = y1;
    i++;

    cosa = cos( x.at<MatrixElementType>(3,0) );
    sina = sin( x.at<MatrixElementType>(3,0) );
    y1 = cosa*r4[0] - sina*r4[1];
    y2 = sina*r4[0] + cosa*r4[1];
    J.at<MatrixElementType>(0,i) = -y2;
    J.at<MatrixElementType>(1,i) = y1;
    i++;
}


