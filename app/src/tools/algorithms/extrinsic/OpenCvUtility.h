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

#ifndef OPENCV_UTILITY_H
#define OPENCV_UTILITY_H

#include <stdio.h>

#include <opencv/cv.h>

void boundingBox( int nPts, const CvPoint2D32f* pts, CvPoint2D32f* min, CvPoint2D32f* dim, float margin = 0.f );
void translateCvMat2D( CvMat* in, CvMat* out, float tx, float ty );

bool intersectLines( CvPoint2D32f a1, CvPoint2D32f a2, CvPoint2D32f b1, CvPoint2D32f b2, double* s, double* t );
bool intersectLines( CvPoint2D32f a1, CvPoint2D32f a2, CvPoint2D32f b1, CvPoint2D32f b2, CvPoint2D32f* ip );
bool intersectLineSegmentWithPolygon( int nPts, const CvPoint2D32f* poly, CvPoint2D32f a1, CvPoint2D32f a2, CvPoint2D32f* ip );
bool pointInPolygon( int nPts, const CvPoint2D32f* poly, CvPoint2D32f pt );

void computeGroundPlaneWarp( const CvMat* intrinsicMatrix, const CvMat* rot, const CvMat* trans,
                             const CvMat* distortion,
                             const CvPoint2D32f* offset,
                             CvMat* mapx,
                             CvMat* mapy );
void computePerspectiveWarp( const CvMat* H, CvMat* mapx, CvMat* mapy );
void projectPointsSimple( CvMat* intrinsicMatrix, CvMat* rot, CvMat* trans,
                          int cornerCount, CvPoint3D32f* pObj3D, CvPoint2D32f* pPts2D );

void logCvMat32F(const CvMat* mat);

void postTranslateCvMat2D( CvMat* in, CvMat* out, float tx, float ty );
void applyHomography( CvMat* H, unsigned int numPoints, CvPoint3D32f* pObject, CvPoint2D32f* pImg );
void extractCvHomography( const CvMat* intrinsic, const CvMat* R, const CvMat* T, CvMat* H );
void invertCvIntrinsicMatrix( const CvMat* intrinsicMatrix, CvMat* inv );
void extractInverseCvHomography( const CvMat* intrinsic, const CvMat* R, const CvMat* T, CvMat* H );
void invertGroundPlanePoints( const CvMat* inverseCoeffs, const CvMat* intrinsic, const CvMat* R, const CvMat* T,
                              unsigned int numPoints, CvPoint3D32f* pObject, CvPoint2D32f* pImg );

CvPoint2D32f  img2norm( CvPoint2D32f p, const CvMat* intrinsic, const CvMat* distortion );
CvPoint2D32f  norm2img( CvPoint2D32f p, const CvMat* intrinsic, const CvMat* distortion );
CvPoint2D32f plane2image( CvPoint2D32f p, const CvMat* intrinsic, const CvMat* distortion, const CvMat* Rot, const CvMat* Trans );
CvPoint2D32f image2plane( CvPoint2D32f p, const CvMat* intrinsic, const CvMat* distortion, const CvMat* Rot, const CvMat* Trans );

IplImage* VisualiseCvMatrix32fc1( const CvMat* mat );
IplImage* LogNormaliseCvImage( const IplImage* img );
CvMat* GradientMagCv32fc1( const CvMat* mat );

void FillCvImageWithRawBytes( IplImage* img, const unsigned char* pImgData );
void GetRawBytesFromCvImage( const IplImage* img, unsigned char* pImgData );

void mouseCvMat32fc1Query(int event, int x, int y, void* params );

void lineSkip( FILE* f );
int lineCount( FILE* fp );
bool fileExists( const char* file );

void motionFilter( const IplImage* src, IplImage* dst, int windowWidth, int windowHeight );

#endif // OPENCV_UTILITY_H
