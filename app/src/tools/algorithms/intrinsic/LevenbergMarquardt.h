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

#ifndef LEVENBERG_MARQUARDT_H
#define LEVENBERG_MARQUARDT_H

#include <math.h>
#include <stdio.h>

#include <opencv/cv.h>

#include <cassert>
#include <algorithm>

#include "Debugging.h"

/**
    Function for performing non-linear optimisation.

    Uses template function/function object pattern to
    provide the function to optimise and the jacobian.
    The class T must support the interface:

        void T::operator () ( cv::Mat& r, const cv::Mat& x ) const;
            : evaluates the objective function at x returning residual r ( r = f(x)-z ).

        void T::jacobian( cv::Mat& J, const cv::Mat& x ) const;
            : evaluate the Jacobian of the objective function at x

        int T::domainDim() const;


            : Return the dimension of the parameter vector.

        int T::rangeDim() const;
            : Return the dimension of the residual vector.



    @param f Function object describing the function to minimise. This is a vector function mapping from R**domainDim() to R**rangeDim().
    @param x0 Vector of starting parameters for optimisation (initial guess).
    @param x Vector of optimised parameters.

    x and x0 must be same dimension (and compatible with f).

    @param W W is an optional weight matrix. If W is not null then the algorithm optimises the residual weighted by W. W must have dimension rangeDim() x rangeDim() and should be the INVERSE of the residual's covariance matrix.
**/
typedef double MatrixElementType;

namespace
{
    const int OpenCvMatrixElementType = CV_64FC1;
    inline const MatrixElementType SumOfDiagonalElementsOf( const cv::Mat& mtx )
    {
        return cv::sum( mtx.diag() ).val[0];
    }

    inline void MakeSymmetric( cv::Mat& mtx )
    {
        assert( mtx.cols == mtx.rows );
        if ( mtx.cols == mtx.rows )
        {
            mtx += mtx.t();
            mtx /= 2.;
        }
    }

    void AddToDiagonals( cv::Mat& mtx, const MatrixElementType& scalar )
    {
        const int n = std::min( mtx.rows, mtx.cols );
        for ( int i = 0; i < n; ++i )
        {
            mtx.at<MatrixElementType>( i, i ) += scalar;
        }
    }
}

template <class T>
MatrixElementType LevenbergMarquardt( const T& f,
                                      const cv::Mat& x0,
                                      cv::Mat& x,
                                      cv::Mat* W = 0 )
{
    const int LM_MAX_ITR  = 100;
    const MatrixElementType LM_ERROR_THRESH = 1e-6;

    assert( f.domainDim() == x0.rows && x0.cols == 1 );
    assert( x0.rows == x.rows && x0.cols == x.cols );

    if ( W != 0 )
    {
        assert( W->cols == f.rangeDim() && W->rows == W->cols );
    }

    unsigned int step = 1;
    MatrixElementType lambda;
    MatrixElementType dErr;
    MatrixElementType oldErr;
    MatrixElementType newErr;

    cv::Mat J( f.rangeDim(), f.domainDim(), OpenCvMatrixElementType );
    cv::Mat Jt( f.domainDim(), f.rangeDim(), OpenCvMatrixElementType );
    cv::Mat xh( f.domainDim(), 1, OpenCvMatrixElementType );
    cv::Mat dx( f.domainDim(), 1, OpenCvMatrixElementType );
    cv::Mat N( f.domainDim(), f.domainDim(), OpenCvMatrixElementType );
    cv::Mat n( f.rangeDim(), 1, OpenCvMatrixElementType );
    cv::Mat r( f.rangeDim(), 1, OpenCvMatrixElementType );
    cv::Mat JtW( f.domainDim(), f.rangeDim(), OpenCvMatrixElementType );
    cv::Mat Ni;

    x = x0;
    f.jacobian( J, x ); // J = df/d(p=x)
    //J.show();
    f( r, x ); // r = f(x)
    //r.show();
    oldErr = r.dot( r );

    Jt = J.t();
    N = Jt * J;
    lambda = SumOfDiagonalElementsOf( N );
    lambda *= 10e-4;
    lambda /= f.domainDim();

    dErr = LM_ERROR_THRESH + 1;
    while ( dErr >= LM_ERROR_THRESH )
    {
        unsigned int count = LM_MAX_ITR;
        while ( count-- )
        {
            xh = x; // new hypothesis
            f.jacobian( J, xh ); // new jacobian

            // form and solve normal equations
            Jt = J.t(); // Jt = J.transpose()

            if ( W )
            {
                // In this case we want to solve the weighted normal equations:
                JtW = Jt * (*W);
                N = JtW * J;
            }
            else
            {
                N = Jt * J;
            }

            AddToDiagonals( N, lambda );
            MakeSymmetric( N );
            Ni  = N.inv( CV_CHOLESKY );

            if ( W )
            {
                n = (*W) * r;
                r = Jt * n;
                n = r;
            }
            else
            {
                n = Jt * r;
            }

            dx = Ni * n;

            // update solution
            xh -= dx;
            f( r, xh ); // new residual
            newErr = r.dot( r );
            dErr = oldErr - newErr;
            if ( dErr < 0 )
            {
                lambda *= 10.f;
            }
            else
            {
                lambda *= 0.1f;
                count = 0; // will end the loop
            }
        }

        dErr = fabs( dErr );
        oldErr = newErr;
        x = xh;
        step++;
    }

    return oldErr;
}

#endif // LEVENBERG_MARQUARDT_H
