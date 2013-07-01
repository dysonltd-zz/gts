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

#include <gtest/gtest.h>
#include "OptimisationTest.h"
#include "LevenbergMarquardt.h"
#include "Debugging.h"

namespace
{
    class SimpleSquareFunction
    {
    public:
        void operator () ( cv::Mat& r, const cv::Mat& x ) const
        {
            r.at<MatrixElementType>(0,0) = x.dot( x );
        }

        void jacobian( cv::Mat& J, const cv::Mat& x ) const
        {
            J.at<MatrixElementType>(0,0) = 2*x.at<MatrixElementType>(0,0);
        }

        int rangeDim()  const { return 1; }
        int domainDim() const { return 1; }
    };

    class SimpleQuarticFunction
    {
    public:
        void operator () ( cv::Mat& r, const cv::Mat& x ) const
        {
            r.at<MatrixElementType>(0,0) = std::pow(x.at<MatrixElementType>(0,0), 4.0);
        }

        void jacobian( cv::Mat& J, const cv::Mat& x ) const
        {
            J.at<MatrixElementType>(0,0) = 4*std::pow(x.at<MatrixElementType>(0,0), 3.0);
        }

        int rangeDim()  const { return 1; }
        int domainDim() const { return 1; }
    };
}

TEST(LevenbergMarquardtTests, SimpleSquareFunctionTest)
{
    SimpleSquareFunction f;
    cv::Mat x( 1, 1, OpenCvMatrixElementType );
    cv::Mat x0( 1, 1, OpenCvMatrixElementType );
    x0.at<MatrixElementType>(0,0) = 5.;

    LevenbergMarquardt( f, x0, x );

    MatrixElementType eps = 0.01;
    EXPECT_LT(x.at<MatrixElementType>(0,0), eps ) << "Square function optimises to 0";
}

//TEST(LevenbergMarquardtTests, SimpleQuarticFunctionTest)
//{
//    SimpleQuarticFunction f;
//    cv::Mat x( 1, 1, OpenCvMatrixElementType );
//    cv::Mat x0( 1, 1, OpenCvMatrixElementType );
//    x0.at<MatrixElementType>(0,0) = 0.5;
//
//    LevenbergMarquardt( f, x0, x );
//
//    MatrixElementType eps = 0.01;
//    EXPECT_LT(x.at<MatrixElementType>(0,0), eps) << "Quartic function optimises to 0";
//}

TEST(LevenbergMarquardtTests, OptimisationTest)
{
    OptimisationTest f;

    cv::Mat x( f.domainDim(), 1, OpenCvMatrixElementType );
    cv::Mat x0( f.domainDim(), 1, OpenCvMatrixElementType );

    const MatrixElementType pi = 3.1415926536;
    x0.at<MatrixElementType>(0,0) = 90*(pi/180.0);
    x0.at<MatrixElementType>(1,0) = 100*(pi/180.0);
    x0.at<MatrixElementType>(2,0) = 120*(pi/180.0);
    x0.at<MatrixElementType>(3,0) = 150*(pi/180.0);

    LevenbergMarquardt( f, x0, x );

    //fprintf( stderr, "x = %g %g %g %g\t",
    //         x.at<MatrixElementType>(0,0),
    //         x.at<MatrixElementType>(1,0),
    //         x.at<MatrixElementType>(2,0),
    //         x.at<MatrixElementType>(3,0) );

    const MatrixElementType eps = 0.01;
    EXPECT_LT(fabs( x.at<MatrixElementType>(0,0) - -0.72 ), eps ) << "Final values are close to correct values";
    EXPECT_LT(fabs( x.at<MatrixElementType>(1,0) - 0.99 ),  eps ) << "Final values are close to correct values";
    EXPECT_LT(fabs( x.at<MatrixElementType>(2,0) - 2.43 ),  eps ) << "Final values are close to correct values";
    EXPECT_LT(fabs( x.at<MatrixElementType>(3,0) - 4.28 ),  eps ) << "Final values are close to correct values";
}

