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

#ifndef OPTIMISATION_TEST_H
#define OPTIMISATION_TEST_H
#include <opencv/cv.h>
#include "LevenbergMarquardt.h"

class OptimisationTest
{
public:
    OptimisationTest() {};
    virtual ~OptimisationTest() {};

    void operator () ( cv::Mat& r, const cv::Mat& x ) const;
    void jacobian( cv::Mat& J, const cv::Mat& x ) const;

    int rangeDim()  const { return 2; };
    int domainDim() const { return 4; };

private:
    static const MatrixElementType p1[2];
    static const MatrixElementType p2[2];
    static const MatrixElementType p3[2];
    static const MatrixElementType r1[2];
    static const MatrixElementType r4[2];
};

#endif
