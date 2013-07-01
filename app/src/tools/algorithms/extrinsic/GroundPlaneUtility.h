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

#ifndef GROUND_PLANE_UTILITY_H
#define GROUND_PLANE_UTILITY_H

#include <opencv/cv.h>

struct ScanPose;

struct Rect32f
{
	CvPoint2D32f pos; ///< Position of rectangle (x and y coords).
	CvPoint2D32f dim; ///< Dimension of rectangle (width and height).
};

CvMat* createCalibrationObject(int width, int height, float squareSize);

void computeExtrinsicParameters(const CvMat* objectPoints, const CvMat* imagePoints,
			const CvMat* intrinsicMatrix, const CvMat* distortionCoeffs,
			CvMat* rotMat, CvMat* trans);

IplImage* computeGroundPlaneWarpBatch(
	const IplImage* viewGrey, const CvMat* intrinsicMatrix,
	const CvMat* distortionCoeffs, const CvMat* inverseCoeffs,
	const CvMat* rotMat, const CvMat* trans,
	CvMat** mapx, CvMat** mapy, CvPoint2D32f* offset
);

IplImage* unwarpGroundPlane(
	const IplImage* viewGrey, const CvMat* intrinsicMatrix,
	const CvMat* distortionCoeffs, const CvMat* inverseCoeffs,
	const CvMat* rotMat, const CvMat* trans, CvPoint2D32f* offset
);

CvMat* findChessBoard(IplImage* view, const IplImage* viewGrey, CvSize boardSize, int draw=0);
CvMat* findChessBoard( IplImage* viewGrey, CvSize boardSize );

void alignGroundPlane( ScanPose pose, const IplImage* src, IplImage* dst,
                       CvPoint2D32f imgOrigin, CvPoint2D32f cmpOrigin);

void alignGroundPlane( const CvMat* transform, const IplImage* src, IplImage* dst );

void compositeImageBoundingBox( ScanPose pose, const IplImage* src,
						        CvPoint2D32f srcOrigin, CvPoint2D32f dstOrigin,
						        CvPoint2D32f* newOrigin, Rect32f* bbox );

void compositeImageBoundingBox( const CvMat* transform, const IplImage* src, Rect32f* bbox );

void createCompositeImage( const IplImage* src1, const IplImage* src2, IplImage* dst);

#endif // GROUND_PLANE_UTILITY_H
