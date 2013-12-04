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

#include "OpenCvUtility.h"

#include "FileUtilities.h"
#include "Logging.h"

#include <opencv/highgui.h>

#include <QtGlobal>

#define LINE_INTERSECT_EPSILON 0.0000000001

namespace OpenCvUtility
{
    void BoundingBox(int nPts, const CvPoint2D32f* pts, CvPoint2D32f* min, CvPoint2D32f* dim, float margin)
    {
        assert(nPts>0);

        *min = pts[0];
        *dim = pts[0];

        // loop just finds min and max coordinates
        for (int p=1; p<nPts; ++p)
        {
            if (pts[p].x < min->x) min->x = pts[p].x;
            if (pts[p].y < min->y) min->y = pts[p].y;
            if (pts[p].x > dim->x) dim->x = pts[p].x;
            if (pts[p].y > dim->y) dim->y = pts[p].y;
        }

        // convert dim from max coord to width/height
        dim->x -= min->x;
        dim->y -= min->y;

        if (margin!=0.f)
        {
            // add a specified margin to the bounding box
            min->x -= margin;
            min->y -= margin;
            dim->x += 2.f*margin;
            dim->y += 2.f*margin;
        }
    }

    void BoundingBox(int nPts, const cv::Point2f* pts, cv::Point2f* min, cv::Point2f* dim, float margin)
    {
        assert(nPts>0);

        *min = pts[0];
        *dim = pts[0];

        // loop just finds min and max coordinates
        for (int p=1; p<nPts; ++p)
        {
            if (pts[p].x < min->x) min->x = pts[p].x;
            if (pts[p].y < min->y) min->y = pts[p].y;
            if (pts[p].x > dim->x) dim->x = pts[p].x;
            if (pts[p].y > dim->y) dim->y = pts[p].y;
        }

        // convert dim from max coord to width/height
        dim->x -= min->x;
        dim->y -= min->y;

        if (margin!=0.f)
        {
            // add a specified margin to the bounding box
            min->x -= margin;
            min->y -= margin;
            dim->x += 2.f*margin;
            dim->y += 2.f*margin;
        }
    }

    bool IntersectLines(CvPoint2D32f a1, CvPoint2D32f a2, CvPoint2D32f b1, CvPoint2D32f b2, double* s, double* t)
    {
        bool rval;

        CvPoint2D32f u;
        CvPoint2D32f v;

        u.x = a1.x - a2.x;
        u.y = a1.y - a2.y;

        v.x = b2.x - b1.x;
        v.y = b2.y - b1.y;

        double det = (u.x*v.y) - (u.y*v.x);

        if (fabs(det) < LINE_INTERSECT_EPSILON)
        {
            rval = false;
        }
        else
        {
            rval = true;

            double n = 1.0 / det;

            CvPoint2D32f d;
            d.x = a1.x - b1.x;
            d.y = a1.y - b1.y;

            *s = ((n*v.y)*d.x) - ((n*v.x)*d.y);
            *t = -((n*u.y)*d.x) + ((n*u.x)*d.y);
        }

        return rval;
    }

    bool IntersectLines(CvPoint2D32f a1, CvPoint2D32f a2, CvPoint2D32f b1, CvPoint2D32f b2, CvPoint2D32f* ip)
    {
        bool rval;

        CvPoint2D32f u;
        CvPoint2D32f v;

        u.x = a1.x - a2.x;
        u.y = a1.y - a2.y;

        v.x = b2.x - b1.x;
        v.y = b2.y - b1.y;

        double det = (u.x*v.y) - (u.y*v.x);

        if (fabs(det) < LINE_INTERSECT_EPSILON)
        {
            rval = false;
        }
        else
        {
            rval = true;

            double n = 1.0 / det;

            CvPoint2D32f d;
            d.x = a1.x - b1.x;
            d.y = a1.y - b1.y;

            double s = ((n*v.y)*d.x) - ((n*v.x)*d.y);
            // double t = -((n*u.y)*d.x) + ((n*u.x)*d.y);

#if defined(__MINGW32__) || defined(__GNUC__)
            ip->x = a1.x - s*u.x;
            ip->y = a1.y - s*u.y;
#else
            ip->x = static_cast<float>(a1.x - s*u.x);
            ip->y = static_cast<float>(a1.y - s*u.y);
#endif
        }

        return rval;
    }

    bool IntersectLineSegmentWithPolygon(int nPts, const CvPoint2D32f* poly, CvPoint2D32f a1, CvPoint2D32f a2, CvPoint2D32f* ip)
    {
        bool rval;
        CvPoint2D32f b1; // boundary vertex 1
        CvPoint2D32f b2; // boundary vertex 2
        b1 = poly[nPts-1];

        double min_s = 1.1;
        double s;
        double t;

        for (int i=0; i<nPts; ++i)
        {
            b2 = poly[i];

            {
                if (IntersectLines(a1, a2, b1, b2, &s, &t))
                {
                    if (s < min_s && s>0.0)
                    {
                        min_s = s;
                    }
                }
            }

            b1 = b2;
        }

        if (min_s <= 1.0)
        {
            rval = true;
#if defined(__MINGW32__) || defined(__GNUC__)
            ip->x = a1.x + min_s*(a2.x-a1.x);
            ip->y = a1.y + min_s*(a2.y-a1.y);
#else
            ip->x = static_cast<float>(a1.x + min_s*(a2.x-a1.x));
            ip->y = static_cast<float>(a1.y + min_s*(a2.y-a1.y));
#endif
        }
        else
        {
            rval = false;
        }

        return rval;
    }

    bool PointInPolygon(int nPts, const CvPoint2D32f* poly, CvPoint2D32f pt)
    {
        // Intersect horizontal line from point with each boundary edge

        CvPoint2D32f b1; // boundary vertex 1
        CvPoint2D32f b2; // boundary vertex 2

        b1 = poly[nPts-1];

        CvPoint2D32f d;
        CvPoint2D32f v;
        double t;
        double s;
        unsigned char inside = 0;

        for (int i=0; i<nPts; ++i)
        {
            b2 = poly[i];

            {
                d.y = pt.y - b1.y;
                v.y = b2.y - b1.y;

                t = d.y/v.y;

                if (0.0 <= t && t < 1.0)
                {
                    d.x = pt.x - b1.x;
                    v.x = b2.x - b1.x;
                    s = d.x - ((v.x/v.y) * d.y);

                    if (s > 0)
                    {
                        inside = ~inside;
                    }
                }
            }

            b1 = b2;
        }

        return inside != 0;
    }

    void ComputeGroundPlaneWarp(const CvMat*        intrinsicMatrix,
                                const CvMat*        rot,
                                const CvMat*        trans,
                                const CvMat*        distortion,
                                const CvPoint2D32f* offset,
                                CvMat*              mapx,
                                CvMat*              mapy)
    {
        assert(mapx->width == mapy->width &&
               mapx->height == mapy->height &&
               mapx->step == mapy->step);

        int w    = mapx->width;
        int h    = mapy->height;
        int step = mapx->step;

        Q_UNUSED(step);

        float* R = rot->data.fl;

        float fx = intrinsicMatrix->data.fl[0];
        float cx = intrinsicMatrix->data.fl[2];
        float fy = intrinsicMatrix->data.fl[4];
        float cy = intrinsicMatrix->data.fl[5];
        float x,y,z;
        float* k = distortion->data.fl;

        // parameters for radial distortion model
        float r2, r4, r6, a1, a2, a3, cdist, xd, yd;

        for (int i = 0; i < h; ++i)
        {
            float I = i+offset->y;

            for (int j = 0; j < w; ++j)
            {
                float J = j+offset->x;

                // homography
                x  = R[0]*J + R[1]*I + *(trans->data.fl+0);
                y  = R[3]*J + R[4]*I + *(trans->data.fl+1);
                z  = R[6]*J + R[7]*I + *(trans->data.fl+2);
                z  = 1.f/z;
                x *= z;
                y *= z;

                // lens distortion
                r2 = x*x + y*y;
                r4 = r2*r2;
                r6 = r2*r4;
                a1 = 2*x*y;
                a2 = r2 + 2*x*x;
                a3 = r2 + 2*y*y;
                cdist = 1.f + k[0]*r2 + k[1]*r4 + k[4]*r6;
                xd = x*cdist + k[2]*a1 + k[3]*a2;
                yd = y*cdist + k[2]*a3 + k[3]*a1;

                x = xd*fx + cx;
                y = yd*fy + cy;

                cvmSet(mapx,i,j,x);
                cvmSet(mapy,i,j,y);
            }
        }
    }

    void ComputeGroundPlaneWarp(const cv::Mat*        intrinsicMatrix,
                                const cv::Mat*        rot,
                                const cv::Mat*        trans,
                                const cv::Mat*        distortion,
                                const cv::Point2f*    offset,
                                cv::Mat*              mapx,
                                cv::Mat*              mapy)
    {
        assert(mapx->size().width == mapy->size().width &&
               mapx->size().height == mapy->size().height &&
               mapx->step == mapy->step);

        int w = mapx->size().width;
        int h = mapy->size().height;
        const float* R = rot->ptr<float>(0);

        float fx = intrinsicMatrix->data[0];
        float cx = intrinsicMatrix->data[2];
        float fy = intrinsicMatrix->data[4];
        float cy = intrinsicMatrix->data[5];
        float x,y,z;
        const float* k = distortion->ptr<float>(0);

        // parameters for radial distortion model
        float r2, r4, r6, a1, a2, a3, cdist, xd, yd;

        for (int i = 0; i < h; ++i)
        {
            float I = i+offset->y;

            for (int j = 0; j < w; ++j)
            {
                float J = j+offset->x;

                // homography
                x  = R[0]*J + R[1]*I + *(trans->ptr<float>(0));
                y  = R[3]*J + R[4]*I + *(trans->ptr<float>(1));
                z  = R[6]*J + R[7]*I + *(trans->ptr<float>(2));
                z  = 1.f/z;
                x *= z;
                y *= z;

                // lens distortion
                r2 = x*x + y*y;
                r4 = r2*r2;
                r6 = r2*r4;
                a1 = 2*x*y;
                a2 = r2 + 2*x*x;
                a3 = r2 + 2*y*y;
                cdist = 1.f + k[0]*r2 + k[1]*r4 + k[4]*r6;
                xd = x*cdist + k[2]*a1 + k[3]*a2;
                yd = y*cdist + k[2]*a3 + k[3]*a1;

                x = xd*fx + cx;
                y = yd*fy + cy;

                mapx->at<float>(i,j) = x;
                mapy->at<float>(i,j) = y;
            }
        }
    }

    void ComputePerspectiveWarp(const CvMat* H, CvMat* mapx, CvMat* mapy)
    {
        assert(mapx->width == mapy->width &&
                mapx->height == mapy->height &&
                mapx->step == mapy->step);

        int w = mapx->width;
        int h = mapy->height;
        //int step = mapx->step;

        float* pH = H->data.fl;

        float x;
        float y;
        float z;

        for (int j = 0; j < w; ++j)
        {
            x = pH[0]*j + pH[2];
            y = pH[3]*j + pH[5];
            z = pH[6]*j + pH[8];

            for (int i = 0; i < h; ++i)
            {
                float Z = z + pH[7]*i;
                Z = 1.f/Z;
                float X = x + pH[1]*i;
                float Y = y + pH[4]*i;
                X *= Z;
                Y *= Z;
                cvmSet(mapx,i,j,X);
                cvmSet(mapy,i,j,Y);
            }
        }
    }

    void ComputePerspectiveWarp(const cv::Mat* H, cv::Mat* mapx, cv::Mat* mapy)
    {
        int w = mapx->size().width;
        int h = mapy->size().height;

        const float* pH = H->ptr<float>(0);

        float x;
        float y;
        float z;

        for (int j = 0; j < w; ++j)
        {
            x = pH[0]*j + pH[2];
            y = pH[3]*j + pH[5];
            z = pH[6]*j + pH[8];

            for (int i = 0; i < h; ++i)
            {
                float Z = z + pH[7]*i;
                Z = 1.f/Z;
                float X = x + pH[1]*i;
                float Y = y + pH[4]*i;
                X *= Z;
                Y *= Z;
                mapx->at<float>(i,j) = X;
                mapy->at<float>(i,j) = Y;
            }
        }
    }

    void ProjectPointsSimple(CvMat* intrinsicMatrix,
                             CvMat* rot,
                             CvMat* trans,
                             int cornerCount,
                             CvPoint3D32f* pObj3D,
                             CvPoint2D32f* pPts2D)
    {
        for (int p=0; p<cornerCount; ++p)
        {
            float x = pObj3D[p].x;
            float y = pObj3D[p].y;
            float z = pObj3D[p].z;

            // R*p + t
            float* R = rot->data.fl;

            float xImg = R[0]*x + R[1]*y + R[2]*z + *(trans->data.fl+0);
            float yImg = R[3]*x + R[4]*y + R[5]*z + *(trans->data.fl+1);
            float wImg = R[6]*x + R[7]*y + R[8]*z + *(trans->data.fl+2);
            float proj = 1.f/wImg;

            // Projection
            float fx = intrinsicMatrix->data.fl[0];
            float cx = intrinsicMatrix->data.fl[2];
            float fy = intrinsicMatrix->data.fl[4];
            float cy = intrinsicMatrix->data.fl[5];
            xImg = xImg*fx + cx*wImg;
            yImg = yImg*fy + cy*wImg;
            pPts2D[p].x  = xImg * proj;
            pPts2D[p].y  = yImg * proj;
        }
    }

    void LogCvMat32F(const CvMat* mat)
    {
        float* data = mat->data.fl;

        int step = mat->step / sizeof(float);

        for (int r=0; r<mat->rows; ++r)
        {
            for (int c=0; c<mat->cols; ++c)
            {
		        LOG_INFO(QObject::tr("[%1,%2] = %3").arg(r)
                                                    .arg(c)
                                                     .arg(*(data+r*step+c)));
            }
        }
    }

    void LogCvMat32F(const cv::Mat* mat)
    {
        const float* data = mat->ptr<float>(0);
        int step = mat->step / sizeof(float);

        for (int r=0; r<mat->rows; ++r)
        {
            for (int c=0; c<mat->cols; ++c)
            {
                LOG_INFO(QObject::tr("[%1,%2] = %3").arg(r)
                                                    .arg(c)
                                                    .arg(*(data+r*step+c)));
            }
        }
    }

    void TranslateCvMat2D(CvMat* in, CvMat* out, float tx, float ty)
    {
        float m[9];
        CvMat M = cvMat(3,3,CV_32F,m);

        m[0] = 1.f;
        m[1] = 0.f;
        m[2] = tx;
        m[3] = 0.f;
        m[4] = 1.f;
        m[5] = ty;
        m[6] = 0.f;
        m[7] = 0.f;
        m[8] = 1.f;

        cvMatMul(&M,in,out);
    }

    void TranslateCvMat2D(cv::Mat* in, cv::Mat* out, float tx, float ty)
    {
        float m[9];
        cv::Mat M = cv::Mat(3,3,CV_32F,m);

        m[0] = 1.f;
        m[1] = 0.f;
        m[2] = tx;
        m[3] = 0.f;
        m[4] = 1.f;
        m[5] = ty;
        m[6] = 0.f;
        m[7] = 0.f;
        m[8] = 1.f;

        /// @todo is this right? are we currently re-assigning the ptr?
        *out = M * (*in);
    }


    void PostTranslateCvMat2D(CvMat* in, CvMat* out, float tx, float ty)
    {
        float m[9];
        CvMat M = cvMat(3,3,CV_32F,m);

        m[0] = 1.f;
        m[1] = 0.f;
        m[2] = tx;
        m[3] = 0.f;
        m[4] = 1.f;
        m[5] = ty;
        m[6] = 0.f;
        m[7] = 0.f;
        m[8] = 1.f;

        cvMatMul(in,&M,out);
    }

    void PostTranslateCvMat2D(cv::Mat* in, cv::Mat* out, float tx, float ty)
    {
        float m[9];
        cv::Mat M = cv::Mat(3,3,CV_32F,m);

        m[0] = 1.f;
        m[1] = 0.f;
        m[2] = tx;
        m[3] = 0.f;
        m[4] = 1.f;
        m[5] = ty;
        m[6] = 0.f;
        m[7] = 0.f;
        m[8] = 1.f;

        /// @todo is this right? are we currently re-assigning the ptr?
        *out = M * (*in);
    }

    void ApplyHomography(CvMat* H, unsigned int numPoints, CvPoint3D32f* pObject, CvPoint2D32f* pImg)
    {
        float* hData = H->data.fl;

        for (unsigned int p=0;p<numPoints;++p)
        {
            float x = pObject[p].x;
            float y = pObject[p].y;

            float xImg = hData[0]*x + hData[1]*y + hData[2];
            float yImg = hData[3]*x + hData[4]*y + hData[5];
            float wImg = hData[6]*x + hData[7]*y + hData[8];

            float proj = 1.f/wImg;
            pImg[p].x  = xImg * proj;
            pImg[p].y  = yImg * proj;
        }
    }

    void ApplyHomography(cv::Mat* H, unsigned int numPoints, CvPoint3D32f* pObject, CvPoint2D32f* pImg)
    {
        float* hData = H->ptr<float>(0);

        for (unsigned int p=0; p<numPoints; ++p)
        {
            float x = pObject[p].x;
            float y = pObject[p].y;

            float xImg = hData[0]*x + hData[1]*y + hData[2];
            float yImg = hData[3]*x + hData[4]*y + hData[5];
            float wImg = hData[6]*x + hData[7]*y + hData[8];

            float proj = 1.f/wImg;
            pImg[p].x  = xImg * proj;
            pImg[p].y  = yImg * proj;
        }
    }

    void ExtractCvHomography(const CvMat* intrinsic, const CvMat* R, const CvMat* T, CvMat* H)
    {
        assert(intrinsic->cols==3 && intrinsic->rows==3);
        assert(H->cols==3 && H->rows==3);

        float h[9];
        CvMat hMat = cvMat(3,3,CV_32F,h);
        float* rData = R->data.fl;
        int step = R->step/sizeof(float);
        h[0] = *(rData + 0*step + 0);
        h[1] = *(rData + 0*step + 1);
        h[2] = T->data.fl[0];
        h[3] = *(rData + 1*step + 0);
        h[4] = *(rData + 1*step + 1);
        h[5] = T->data.fl[1];
        h[6] = *(rData + 2*step + 0);
        h[7] = *(rData + 2*step + 1);
        h[8] = T->data.fl[2];

        cvMatMul(intrinsic,&hMat,H);
    }

    void ExtractCvHomography(const cv::Mat* intrinsic, const cv::Mat* R, const cv::Mat* T, cv::Mat* H)
    {
        assert(intrinsic->cols == 3 && intrinsic->rows == 3);
        assert(H->cols == 3 && H->rows == 3);

        float h[9];
        cv::Mat hMat = cv::Mat(3,3,CV_32F,h);
        const float* rData = R->ptr<float>(0);
        int step = R->step[0];
        h[0] = *(rData + 0*step + 0);
        h[1] = *(rData + 0*step + 1);
        h[2] = T->data[0];
        h[3] = *(rData + 1*step + 0);
        h[4] = *(rData + 1*step + 1);
        h[5] = T->data[1];
        h[6] = *(rData + 2*step + 0);
        h[7] = *(rData + 2*step + 1);
        h[8] = T->data[2];

        /// @todo is this right? are we currently re-assigning the ptr?
        *H = (*intrinsic) * hMat;
    }

    void InvertCvIntrinsicMatrix(const CvMat* intrinsicMatrix, CvMat* inv)
    {
        assert(inv->rows==3 && inv->cols==3);

        float ifx = 1.f/intrinsicMatrix->data.fl[0];
        float cx  = intrinsicMatrix->data.fl[2];
        float ify = 1.f/intrinsicMatrix->data.fl[4];
        float cy  = intrinsicMatrix->data.fl[5];

        inv->data.fl[0] = ifx;
        inv->data.fl[1] = 0.f;
        inv->data.fl[2] = -cx*ifx;
        inv->data.fl[3] = 0.f;
        inv->data.fl[4] = ify;
        inv->data.fl[5] = -cy*ify;
        inv->data.fl[6] = 0.f;
        inv->data.fl[7] = 0.f;
        inv->data.fl[8] = 1.f;
    }

    void InvertCvIntrinsicMatrix(const cv::Mat* intrinsicMatrix, cv::Mat* inv)
    {
        assert(inv->rows == 3 && inv->cols == 3);

        const float ifx = 1.f/intrinsicMatrix->data[0];
        const float cx  = intrinsicMatrix->data[2];
        const float ify = 1.f/intrinsicMatrix->data[4];
        const float cy  = intrinsicMatrix->data[5];

        inv->data[0] = ifx;
        inv->data[1] = 0.f;
        inv->data[2] = -cx*ifx;
        inv->data[3] = 0.f;
        inv->data[4] = ify;
        inv->data[5] = -cy*ify;
        inv->data[6] = 0.f;
        inv->data[7] = 0.f;
        inv->data[8] = 1.f;
    }

    void ExtractInverseCvHomography(const CvMat* intrinsic, const CvMat* R, const CvMat* T, CvMat* H)
    {
        // First we invert the calibration matrix
        float invCal[9];
        CvMat invCalMat = cvMat(3,3,CV_32F,invCal);
        InvertCvIntrinsicMatrix(intrinsic,&invCalMat);

        // Extract homography components
        float h[9];
        CvMat hMat = cvMat(3,3,CV_32F,h);
        float* rData = R->data.fl;
        int step = R->step/sizeof(float);
        h[0] = *(rData + 0*step + 0);
        h[1] = *(rData + 0*step + 1);
        h[2] = T->data.fl[0];
        h[3] = *(rData + 1*step + 0);
        h[4] = *(rData + 1*step + 1);
        h[5] = T->data.fl[1];
        h[6] = *(rData + 2*step + 0);
        h[7] = *(rData + 2*step + 1);
        h[8] = T->data.fl[2];
        cvInvert(&hMat,&hMat);

        cvMatMul(&hMat,&invCalMat,H);
    }

    void ExtractInverseCvHomography(const cv::Mat* intrinsic, const cv::Mat* R, const cv::Mat* T, cv::Mat* H)
    {
        // First we invert the calibration matrix
        float invCal[9];
        cv::Mat invCalMat = cv::Mat(3,3,CV_32F,invCal);
        InvertCvIntrinsicMatrix(intrinsic,&invCalMat);

        // Extract homography components
        float h[9];
        cv::Mat hMat = cv::Mat(3,3,CV_32F,h);
        const float* rData = R->ptr<float>();
        int step = R->step/sizeof(float);
        h[0] = *(rData + 0*step + 0);
        h[1] = *(rData + 0*step + 1);
        h[2] = T->data[0];
        h[3] = *(rData + 1*step + 0);
        h[4] = *(rData + 1*step + 1);
        h[5] = T->data[1];
        h[6] = *(rData + 2*step + 0);
        h[7] = *(rData + 2*step + 1);
        h[8] = T->data[2];
        cv::invert(hMat,hMat);

        *H = hMat * invCalMat;
    }

    void InvertGroundPlanePoints(const CvMat*  inverseCoeffs,
                                 const CvMat*  intrinsic,
                                 const CvMat*  R,
                                 const CvMat*  T,
                                 unsigned int  numPoints,
                                 CvPoint3D32f* pObject,
                                 CvPoint2D32f* pImg)
    {
        Q_UNUSED(inverseCoeffs);

        // Extract homography components and invert
        float h[9];
        CvMat hMat = cvMat(3,3,CV_32F,h);
        float* rData = R->data.fl;
        int step = R->step/sizeof(float);
        h[0] = *(rData + 0*step + 0);
        h[1] = *(rData + 0*step + 1);
        h[2] = T->data.fl[0];
        h[3] = *(rData + 1*step + 0);
        h[4] = *(rData + 1*step + 1);
        h[5] = T->data.fl[1];
        h[6] = *(rData + 2*step + 0);
        h[7] = *(rData + 2*step + 1);
        h[8] = T->data.fl[2];
        cvInvert(&hMat,&hMat);

        float fx = intrinsic->data.fl[0];
        float cx = intrinsic->data.fl[2];
        float fy = intrinsic->data.fl[4];
        float cy = intrinsic->data.fl[5];

        float* hData = hMat.data.fl;
        // float* k = inverseCoeffs->data.fl;

        for (unsigned int p=0;p<numPoints;++p)
        {
            float x = (pObject[p].x-cx)/fx;
            float y = (pObject[p].y-cy)/fy;

            // Apply inverse radial distortion - found out we can't use this to undistort image corners
            // because they are beyond the limit of the distortion model for our camera
            //float xd = (x-cx)/fx;
            //float yd = (y-cy)/fy;
            //float r2 = xd*xd + yd*yd;
            //float r4 = r2*r2;
            //float a1 = 2.f*xd*yd;
            //float a2 = r2 + 2.f*xd*xd;
            //float a3 = r2 + 2.f*yd*yd;
            //float cdist = 1.f + k[0]*r2 + k[1]*r4;
            //x = xd*cdist + k[2]*a1 + k[3]*a2;
            //y = yd*cdist + k[2]*a3 + k[3]*a1;

            // Apply inverse homography
            float xImg = hData[0]*x + hData[1]*y + hData[2];
            float yImg = hData[3]*x + hData[4]*y + hData[5];
            float wImg = hData[6]*x + hData[7]*y + hData[8];

            float proj = 1.f/wImg;
            pImg[p].x  = xImg * proj;
            pImg[p].y  = yImg * proj;
        }
    }

    void InvertGroundPlanePoints(const cv::Mat*  inverseCoeffs,
                                 const cv::Mat*  intrinsic,
                                 const cv::Mat*  R,
                                 const cv::Mat*  T,
                                 unsigned int    numPoints,
                                 cv::Point3f*    pObject,
                                 cv::Point2f*    pImg)
    {
        Q_UNUSED(inverseCoeffs);

        // Extract homography components and invert
        float h[9];
        cv::Mat hMat = cv::Mat(3,3,CV_32F,h);
        const float* rData = R->ptr<float>(0);
        int step = R->step/sizeof(float);
        h[0] = *(rData + 0*step + 0);
        h[1] = *(rData + 0*step + 1);
        h[2] = T->data[0];
        h[3] = *(rData + 1*step + 0);
        h[4] = *(rData + 1*step + 1);
        h[5] = T->data[1];
        h[6] = *(rData + 2*step + 0);
        h[7] = *(rData + 2*step + 1);
        h[8] = T->data[2];
        hMat.inv();

        float fx = intrinsic->data[0];
        float cx = intrinsic->data[2];
        float fy = intrinsic->data[4];
        float cy = intrinsic->data[5];

        const float* hData = hMat.ptr<float>(0);
        // float* k = inverseCoeffs->data.fl;

        for (unsigned int p=0;p<numPoints;++p)
        {
            float x = (pObject[p].x-cx)/fx;
            float y = (pObject[p].y-cy)/fy;

            // Apply inverse radial distortion - found out we can't use this to undistort image corners
            // because they are beyond the limit of the distortion model for our camera
            //float xd = (x-cx)/fx;
            //float yd = (y-cy)/fy;
            //float r2 = xd*xd + yd*yd;
            //float r4 = r2*r2;
            //float a1 = 2.f*xd*yd;
            //float a2 = r2 + 2.f*xd*xd;
            //float a3 = r2 + 2.f*yd*yd;
            //float cdist = 1.f + k[0]*r2 + k[1]*r4;
            //x = xd*cdist + k[2]*a1 + k[3]*a2;
            //y = yd*cdist + k[2]*a3 + k[3]*a1;

            // Apply inverse homography
            float xImg = hData[0]*x + hData[1]*y + hData[2];
            float yImg = hData[3]*x + hData[4]*y + hData[5];
            float wImg = hData[6]*x + hData[7]*y + hData[8];

            float proj = 1.f/wImg;
            pImg[p].x  = xImg * proj;
            pImg[p].y  = yImg * proj;
        }
    }

    CvPoint2D32f ImgToNorm(CvPoint2D32f p, const CvMat* intrinsic, const CvMat* distortion)
    {
        float fx = intrinsic->data.fl[0];
        float cx = intrinsic->data.fl[2];
        float fy = intrinsic->data.fl[4];
        float cy = intrinsic->data.fl[5];
        float* k = distortion->data.fl;

        float xd = (p.x-cx)/fx;
        float yd = (p.y-cy)/fy;
        float r2 = xd*xd + yd*yd;
        float r4 = r2*r2;
        float a1 = 2.f*xd*yd;
        float a2 = r2 + 2.f*xd*xd;
        float a3 = r2 + 2.f*yd*yd;
        float cdist = 1.f + k[0]*r2 + k[1]*r4;
        p.x = xd*cdist + k[2]*a1 + k[3]*a2;
        p.y = yd*cdist + k[2]*a3 + k[3]*a1;

        return cvPoint2D32f(p.x,p.y);
    }

    cv::Point2f ImgToNorm(cv::Point2f p, const cv::Mat* intrinsic, const cv::Mat* distortion)
    {
        float fx = intrinsic->data[0];
        float cx = intrinsic->data[2];
        float fy = intrinsic->data[4];
        float cy = intrinsic->data[5];
        const float* k = distortion->ptr<float>(0);

        float xd = (p.x-cx)/fx;
        float yd = (p.y-cy)/fy;
        float r2 = xd*xd + yd*yd;
        float r4 = r2*r2;
        float a1 = 2.f*xd*yd;
        float a2 = r2 + 2.f*xd*xd;
        float a3 = r2 + 2.f*yd*yd;
        float cdist = 1.f + k[0]*r2 + k[1]*r4;
        p.x = xd*cdist + k[2]*a1 + k[3]*a2;
        p.y = yd*cdist + k[2]*a3 + k[3]*a1;

        return cv::Point2f(p.x,p.y);
    }

    CvPoint2D32f NormToImg(CvPoint2D32f p, const CvMat* intrinsic, const CvMat* distortion)
    {
        float fx = intrinsic->data.fl[0];
        float cx = intrinsic->data.fl[2];
        float fy = intrinsic->data.fl[4];
        float cy = intrinsic->data.fl[5];
        float* k = distortion->data.fl;

        // lens distortion
        float r2 = p.x*p.x + p.y*p.y;
        float r4 = r2*r2;
        float a1 = 2*p.x*p.y;
        float a2 = r2 + 2.f*p.x*p.x;
        float a3 = r2 + 2.f*p.y*p.y;
        float cdist = 1.f + k[0]*r2 + k[1]*r4;
        float xd = p.x*cdist + k[2]*a1 + k[3]*a2;
        float yd = p.y*cdist + k[2]*a3 + k[3]*a1;

        p.x = xd*fx + cx;
        p.y = yd*fy + cy;

        return cvPoint2D32f(p.x,p.y);
    }

    cv::Point2f NormToImg(cv::Point2f p, const cv::Mat* intrinsic, const cv::Mat* distortion)
    {
        float fx = intrinsic->data[0];
        float cx = intrinsic->data[2];
        float fy = intrinsic->data[4];
        float cy = intrinsic->data[5];
        const float* k = distortion->ptr<float>(0);

        // lens distortion
        float r2 = p.x*p.x + p.y*p.y;
        float r4 = r2*r2;
        float a1 = 2*p.x*p.y;
        float a2 = r2 + 2.f*p.x*p.x;
        float a3 = r2 + 2.f*p.y*p.y;
        float cdist = 1.f + k[0]*r2 + k[1]*r4;
        float xd = p.x*cdist + k[2]*a1 + k[3]*a2;
        float yd = p.y*cdist + k[2]*a3 + k[3]*a1;

        p.x = xd*fx + cx;
        p.y = yd*fy + cy;

        return cv::Point2f(p.x,p.y);
    }

    CvPoint2D32f PlaneToImage(CvPoint2D32f p, const CvMat* intrinsic, const CvMat* distortion, const CvMat* Rot, const CvMat* Trans)
    {
        float* R = Rot->data.fl;
        float fx = intrinsic->data.fl[0];
        float cx = intrinsic->data.fl[2];
        float fy = intrinsic->data.fl[4];
        float cy = intrinsic->data.fl[5];
        float x,y,z;
        float* k = distortion->data.fl;
        float r2, r4, a1, a2, a3, cdist, xd, yd; // parameters for radial distortion model

        // homography
        x = R[0]*p.x + R[1]*p.y + *(Trans->data.fl+0);
        y = R[3]*p.x + R[4]*p.y + *(Trans->data.fl+1);
        z = R[6]*p.x + R[7]*p.y + *(Trans->data.fl+2);
        z = 1.f/z;
        x *= z;
        y *= z;

        // lens distortion
        r2 = x*x + y*y;
        r4 = r2*r2;
        a1 = 2*x*y;
        a2 = r2 + 2*x*x;
        a3 = r2 + 2*y*y;
        cdist = 1.f + k[0]*r2 + k[1]*r4;
        xd = x*cdist + k[2]*a1 + k[3]*a2;
        yd = y*cdist + k[2]*a3 + k[3]*a1;
        x  = xd*fx + cx;
        y  = yd*fy + cy;

        return cvPoint2D32f(x,y);
    }

    cv::Point2f PlaneToImage(cv::Point2f p, const cv::Mat* intrinsic, const cv::Mat* distortion, const cv::Mat* Rot, const cv::Mat* Trans)
    {
        const float* R = Rot->ptr<float>(0);
        float fx = intrinsic->data[0];
        float cx = intrinsic->data[2];
        float fy = intrinsic->data[4];
        float cy = intrinsic->data[5];
        float x,y,z;
        const float* k = distortion->ptr<float>(0);
        float r2, r4, a1, a2, a3, cdist, xd, yd; // parameters for radial distortion model

        // homography
        x = R[0]*p.x + R[1]*p.y + *(Trans->data+0);
        y = R[3]*p.x + R[4]*p.y + *(Trans->data+1);
        z = R[6]*p.x + R[7]*p.y + *(Trans->data+2);
        z = 1.f/z;
        x *= z;
        y *= z;

        // lens distortion
        r2 = x*x + y*y;
        r4 = r2*r2;
        a1 = 2*x*y;
        a2 = r2 + 2*x*x;
        a3 = r2 + 2*y*y;
        cdist = 1.f + k[0]*r2 + k[1]*r4;
        xd = x*cdist + k[2]*a1 + k[3]*a2;
        yd = y*cdist + k[2]*a3 + k[3]*a1;
        x  = xd*fx + cx;
        y  = yd*fy + cy;

        return cv::Point2f(x,y);
    }

    CvPoint2D32f ImageToPlane(CvPoint2D32f p, const CvMat* intrinsic, const CvMat* distortion, const CvMat* R, const CvMat* T)
    {
        // Extract homography components and invert
        float    h[9];
        CvMat    hMat  = cvMat(3,3,CV_32F,h);
        float*  rData = R->data.fl;
        int        step  = R->step/sizeof(float);

        h[0] = *(rData + 0*step + 0);
        h[1] = *(rData + 0*step + 1);
        h[2] = T->data.fl[0];
        h[3] = *(rData + 1*step + 0);
        h[4] = *(rData + 1*step + 1);
        h[5] = T->data.fl[1];
        h[6] = *(rData + 2*step + 0);
        h[7] = *(rData + 2*step + 1);
        h[8] = T->data.fl[2];
        cvInvert(&hMat,&hMat);

        float fx = intrinsic->data.fl[0];
        float cx = intrinsic->data.fl[2];
        float fy = intrinsic->data.fl[4];
        float cy = intrinsic->data.fl[5];

        float* hData = hMat.data.fl;
        float* k = distortion->data.fl;

        float x = p.x;
        float y = p.y;

        // Apply inverse radial distortion - found out we can't use this to undistort image corners
        // because they are beyond the limit of the distortion model for our camera
        float xd = (x-cx)/fx;
        float yd = (y-cy)/fy;
        float r2 = xd*xd + yd*yd;
        float r4 = r2*r2;
        float a1 = 2.f*xd*yd;
        float a2 = r2 + 2.f*xd*xd;
        float a3 = r2 + 2.f*yd*yd;
        float cdist = 1.f + k[0]*r2 + k[1]*r4;
        x = xd*cdist + k[2]*a1 + k[3]*a2;
        y = yd*cdist + k[2]*a3 + k[3]*a1;

        // Apply inverse homography
        float xImg = hData[0]*x + hData[1]*y + hData[2];
        float yImg = hData[3]*x + hData[4]*y + hData[5];
        float wImg = hData[6]*x + hData[7]*y + hData[8];

        float proj = 1.f/wImg;

        return cvPoint2D32f(xImg*proj, yImg*proj);
    }

    cv::Point2f ImageToPlane(cv::Point2f p, const cv::Mat* intrinsic, const cv::Mat* distortion, const cv::Mat* R, const cv::Mat* T)
    {
        // Extract homography components and invert
        float h[9];
        cv::Mat hMat  = cv::Mat(3,3,CV_32F,h);
        const float* rData = R->ptr<float>(0);
        int step = R->step/sizeof(float);

        h[0] = *(rData + 0*step + 0);
        h[1] = *(rData + 0*step + 1);
        h[2] = T->data[0];
        h[3] = *(rData + 1*step + 0);
        h[4] = *(rData + 1*step + 1);
        h[5] = T->data[1];
        h[6] = *(rData + 2*step + 0);
        h[7] = *(rData + 2*step + 1);
        h[8] = T->data[2];

        hMat.inv();

        float fx = intrinsic->data[0];
        float cx = intrinsic->data[2];
        float fy = intrinsic->data[4];
        float cy = intrinsic->data[5];

        const float* hData = hMat.ptr<float>(0);
        const float* k = distortion->ptr<float>(0);

        float x = p.x;
        float y = p.y;

        // Apply inverse radial distortion - found out we can't use this to undistort image corners
        // because they are beyond the limit of the distortion model for our camera
        float xd = (x-cx)/fx;
        float yd = (y-cy)/fy;
        float r2 = xd*xd + yd*yd;
        float r4 = r2*r2;
        float a1 = 2.f*xd*yd;
        float a2 = r2 + 2.f*xd*xd;
        float a3 = r2 + 2.f*yd*yd;
        float cdist = 1.f + k[0]*r2 + k[1]*r4;
        x = xd*cdist + k[2]*a1 + k[3]*a2;
        y = yd*cdist + k[2]*a3 + k[3]*a1;

        // Apply inverse homography
        float xImg = hData[0]*x + hData[1]*y + hData[2];
        float yImg = hData[3]*x + hData[4]*y + hData[5];
        float wImg = hData[6]*x + hData[7]*y + hData[8];

        float proj = 1.f/wImg;

        return cv::Point2f(xImg*proj, yImg*proj);
    }

    IplImage* VisualiseCvMatrix32fc1(const CvMat* mat)
    {
        float min = CV_MAT_ELEM(*mat, float, 0, 0);
        float max = min;

        for (int i=0; i<mat->height; ++i)
        {
            for (int j=0; j<mat->width; ++j)
            {
                float val = (CV_MAT_ELEM(*mat, float, i, j));

                if (val>max)
                {
                    max = val;
                }

                if(val<min)
                {
                    min = val;
                }
            }
        }

        float scale = 255.f/(max-min);

        LOG_INFO(QObject::tr("Visualise grad-mag: min = %1, max = %2.").arg(min)
                                                                       .arg(max));

        IplImage* img = cvCreateImage(cvSize(mat->width, mat->height), IPL_DEPTH_8U, 1);

        for (int i=0; i<mat->height; ++i)
        {
            for (int j=0; j<mat->width; ++j)
            {
                float val = (CV_MAT_ELEM(*mat, float, i, j));
                val = (val - min) * scale;

                CV_IMAGE_ELEM(img, uchar, i, j) = (unsigned char)val;
            }
        }

        return img;
    }

    std::shared_ptr<cv::Mat> VisualiseCvMatrix32fc1(const cv::Mat* mat)
    {
        float min = mat->at<float>(0,0);
        float max = min;

        for (int i=0; i<mat->size().height; ++i)
        {
            for (int j=0; j<mat->size().width; ++j)
            {
                float val = mat->at<float>( i, j);

                if (val>max)
                {
                    max = val;
                }

                if(val<min)
                {
                    min = val;
                }
            }
        }

        float scale = 255.f/(max-min);

        LOG_INFO(QObject::tr("Visualise grad-mag: min = %1, max = %2.").arg(min)
                                                                       .arg(max));

        std::shared_ptr<cv::Mat> img(new cv::Mat(mat->size().width, mat->size().height, CV_8U, 1));
        for (int i=0; i<mat->size().height; ++i)
        {
            for (int j=0; j<mat->size().width; ++j)
            {
                float val = mat->at<float>(i, j);
                val = (val - min) * scale;

                img->at<char>(i, j) = (unsigned char)val;
            }
        }

        return img;
    }

    IplImage* LogNormaliseCvImage(const IplImage* img)
    {
        CvMat* mat = cvCreateMat(img->height, img->width, CV_32F);

        for (int i=0; i<mat->height; ++i)
        {
            for (int j=0; j<mat->width; ++j)
            {
                CV_MAT_ELEM(*mat, float, i, j) = logf(1.f + CV_IMAGE_ELEM(img, uchar, i, j));
            }
        }

        IplImage* result = VisualiseCvMatrix32fc1(mat);

        cvReleaseMat(&mat);

        return result;
    }

    std::shared_ptr<cv::Mat> LogNormaliseCvImage(const cv::Mat* img)
    {
        cv::Mat mat = cv::Mat(img->size().height, img->size().width, CV_32F);

        for (int i=0; i<mat.size().height; ++i)
        {
            for (int j=0; j<mat.size().width; ++j)
            {
                mat.at<float>(i,j) = logf(1.f + img->at<uchar>(i,j));
            }
        }

        std::shared_ptr<cv::Mat> result = VisualiseCvMatrix32fc1(&mat);
        return result;
    }

    CvMat* GradientMagCv32fc1(const CvMat* mat)
    {
        CvMat* grad = cvCloneMat(mat);

        for (int i=0; i<mat->height-1; ++i)
        {
            for (int j=0; j<mat->width-1; ++j)
            {
                float vo = CV_MAT_ELEM(*mat, float, i, j);
                float vx = CV_MAT_ELEM(*mat, float, i, j+1);
                float vy = CV_MAT_ELEM(*mat, float, i+1, j);

                float dx = vx-vo;
                float dy = vy-vo;
                CV_MAT_ELEM(*grad, float, i, j) = sqrtf(dx*dx + dy*dy);
            }
        }

        // fill in right and bottom edge pixels with replicated values
        for (int i=0; i<grad->height; ++i)
        {
            CV_MAT_ELEM(*grad, float, i, grad->width-1) = CV_MAT_ELEM(*grad, float, i, grad->width-2);
        }

        for (int j=0; j<grad->width; ++j)
        {
            CV_MAT_ELEM(*grad, float, grad->height-1, j) = CV_MAT_ELEM(*grad, float, grad->height-2, j);
        }

        return grad;
    }

    std::shared_ptr<cv::Mat> GradientMagCv32fc1(const cv::Mat* mat)
    {
        std::shared_ptr<cv::Mat> grad(new cv::Mat(*mat));

        for (int i=0; i<mat->size().height-1; ++i)
        {
            for (int j=0; j<mat->size().width-1; ++j)
            {
                float vo = mat->at<float>(i,j);
                float vx = mat->at<float>(i,j+1);
                float vy = mat->at<float>(i+1,j);

                float dx = vx-vo;
                float dy = vy-vo;
                grad->at<float>(i,j) = sqrtf(dx*dx + dy*dy);
            }
        }

        // fill in right and bottom edge pixels with replicated values
        for (int i=0; i<grad->size().height; ++i)
        {
            grad->at<float>(i, grad->size().width-1) = grad->at<float>(i, grad->size().width-2);
        }

        for (int j=0; j<grad->size().width; ++j)
        {
            grad->at<float>(grad->size().height-1, j) = grad->at<float>(grad->size().height-2, j);
        }

        return grad;
    }

    void MouseCvMat32fc1Query(int event, int x, int y, void* params)
    {
        const CvMat* mat = (const CvMat*)params;

        if (event == CV_EVENT_LBUTTONUP && mat)
        {
            if (x>=0 && x<mat->width && y>=0 && y<mat->height)
            {
                float val = CV_MAT_ELEM(*mat, float, x, y);

                LOG_INFO(QObject::tr("Pixel value: %1.").arg(val));
            }
        }
    }

    /*
    void MouseCvMat32fc1Query(int event, int x, int y, void* params)
    {
        const cv::Mat* mat = (const cv::Mat*)params;

        if (event == CV_EVENT_LBUTTONUP && mat)
        {
            if (x>=0 && x<mat->width && y>=0 && y<mat->height)
            {
                float val = mat->at<float>(x,y);

                LOG_INFO(QObject::tr("Pixel value: %1.").arg(val));
            }
        }
    }
    */

    void FillCvImageWithRawBytes(IplImage* img, const unsigned char* pImgData)
    {
        int step = img->widthStep;
        for (int r=0; r<img->height; ++r)
        {
            char* pImg             = img->imageData + (r * step);
            const char* const pEnd = pImg + (img->width*img->nChannels);

            while (pImg != pEnd)
            {
                *pImg++ = *pImgData++;
            }
        }
    }

    void FillCvImageWithRawBytes(cv::Mat* img, const unsigned char* pImgData)
    {
        int step = img->step[0];
        for (int r=0; r < img->size().height; ++r)
        {
            char* pImg             = img->ptr<char>(0,0) + (r * step);
            const char* const pEnd = pImg + (img->size().width * img->channels());

            while (pImg != pEnd)
            {
                *pImg++ = *pImgData++;
            }
        }
    }

    void GetRawBytesFromCvImage(const IplImage* img, unsigned char* pImgData)
    {
        int step = img->widthStep;

        for (int r=0; r<img->height; ++r)
        {
            char* pImg = img->imageData + (r * step);

            const char* const pEnd = pImg + (img->width*img->nChannels);

            while (pImg != pEnd)
            {
                *pImgData++ = *pImg++;
            }
        }
    }

    void GetRawBytesFromCvImage(const cv::Mat* img, unsigned char* pImgData)
    {
        int step = img->step[0];

        for (int r=0; r < img->size().height; ++r)
        {
            const char* pImg = img->ptr<char>(0,0) + (r * step);

            const char* const pEnd = pImg + (img->size().width * img->channels());

            while (pImg != pEnd)
            {
                *pImgData++ = *pImg++;
            }
        }
    }

    void MotionFilter(const IplImage* src, IplImage* dst, int ww, int wh)
    {
        IplImage* sum = cvCreateImage(cvSize(src->width+1,src->height+1), IPL_DEPTH_32S, 1);

        cvIntegral(src, sum);

        int w = src->width;
        int h = src->height;
        int sumStep = sum->widthStep/sizeof(int);
        int dstStep = dst->widthStep/sizeof(int);

        for (int j=wh; j<h-wh; ++j)
        {
            for (int i=ww; i<w-ww; ++i)
            {
                int* pSum = reinterpret_cast<int*>(sum->imageData);
                int* pDst = reinterpret_cast<int*>(dst->imageData);
                pSum += j*sumStep + i;
                pDst += j*dstStep + i;

                int* p1 = pSum - (wh/2)*sumStep - (ww/2);
                int* p2 = p1 + ww;
                int* p3 = p1 + wh*sumStep;
                int* p4 = p3 + ww;

                int val = *p1 - *p2 - *p3 + *p4;
                *pDst = val/255;
            }
        }

        cvReleaseImage(&sum);
    }


    void MotionFilter(const cv::Mat* src, cv::Mat* dst, int ww, int wh)
    {
        cv::Mat sum = cv::Mat(src->size().width+1,src->size().height+1, CV_32S, 1);

        cv::integral(*src,sum);

        int w = src->size().width;
        int h = src->size().height;
        int sumStep = sum.size[0]/sizeof(int);
        int dstStep = dst->size[0]/sizeof(int);

        for (int j=wh; j<h-wh; ++j)
        {
            for (int i=ww; i<w-ww; ++i)
            {
                int* pSum = sum.ptr<int>(0,0);
                int* pDst = dst->ptr<int>(0,0);
                pSum += j*sumStep + i;
                pDst += j*dstStep + i;

                int* p1 = pSum - (wh/2)*sumStep - (ww/2);
                int* p2 = p1 + ww;
                int* p3 = p1 + wh*sumStep;
                int* p4 = p3 + ww;

                int val = *p1 - *p2 - *p3 + *p4;
                *pDst = val/255;
            }
        }
    }
}
