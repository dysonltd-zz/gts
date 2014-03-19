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

namespace OpenCvUtility
{
    void BoundingBox( int nPts, const CvPoint2D32f* pts, CvPoint2D32f* min, CvPoint2D32f* dim, float margin = 0.f );
    void TranslateCvMat2D( CvMat* in, CvMat* out, float tx, float ty );

    bool IntersectLines( CvPoint2D32f a1, CvPoint2D32f a2, CvPoint2D32f b1, CvPoint2D32f b2, double* s, double* t );
    bool IntersectLines( CvPoint2D32f a1, CvPoint2D32f a2, CvPoint2D32f b1, CvPoint2D32f b2, CvPoint2D32f* ip );
    bool IntersectLineSegmentWithPolygon( int nPts, const CvPoint2D32f* poly, CvPoint2D32f a1, CvPoint2D32f a2, CvPoint2D32f* ip );
    bool PointInPolygon( int nPts, const CvPoint2D32f* poly, CvPoint2D32f pt );

    void ComputeGroundPlaneWarp( const CvMat* intrinsicMatrix, const CvMat* rot, const CvMat* trans,
                                 const CvMat* distortion,
                                 const CvPoint2D32f* offset,
                                 CvMat* mapx,
                                 CvMat* mapy );
    void ComputePerspectiveWarp( const CvMat* H, CvMat* mapx, CvMat* mapy );
    void ProjectPointsSimple( CvMat* intrinsicMatrix, CvMat* rot, CvMat* trans,
                              int cornerCount, CvPoint3D32f* pObj3D, CvPoint2D32f* pPts2D );

    void LogCvMat32F(const CvMat* mat);

    void PostTranslateCvMat2D( CvMat* in, CvMat* out, float tx, float ty );
    void ApplyHomography( CvMat* H, unsigned int numPoints, CvPoint3D32f* pObject, CvPoint2D32f* pImg );
    void ExtractCvHomography( const CvMat* intrinsic, const CvMat* R, const CvMat* T, CvMat* H );
    void InvertCvIntrinsicMatrix( const CvMat* intrinsicMatrix, CvMat* inv );
    void ExtractInverseCvHomography( const CvMat* intrinsic, const CvMat* R, const CvMat* T, CvMat* H );
    void InvertGroundPlanePoints( const CvMat* inverseCoeffs, const CvMat* intrinsic, const CvMat* R, const CvMat* T,
                                  unsigned int numPoints, CvPoint3D32f* pObject, CvPoint2D32f* pImg );

    CvPoint2D32f Img2norm( CvPoint2D32f p, const CvMat* intrinsic, const CvMat* distortion );
    CvPoint2D32f Norm2img( CvPoint2D32f p, const CvMat* intrinsic, const CvMat* distortion );
    CvPoint2D32f Plane2image( CvPoint2D32f p, const CvMat* intrinsic, const CvMat* distortion, const CvMat* Rot, const CvMat* Trans );
    CvPoint2D32f Image2plane( CvPoint2D32f p, const CvMat* intrinsic, const CvMat* distortion, const CvMat* Rot, const CvMat* Trans );

    IplImage* VisualiseCvMatrix32fc1( const CvMat* mat );
    IplImage* LogNormaliseCvImage( const IplImage* img );
    CvMat* GradientMagCv32fc1( const CvMat* mat );

    void FillCvImageWithRawBytes( IplImage* img, const unsigned char* pImgData );
    void GetRawBytesFromCvImage( const IplImage* img, unsigned char* pImgData );

    void MouseCvMat32fc1Query(int event, int x, int y, void* params );

    void MotionFilter( const IplImage* src, IplImage* dst, int windowWidth, int windowHeight );
}

#endif // OPENCV_UTILITY_H
