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

#include "CrossCorrelation.h"

#include <QtGlobal>

namespace CrossCorrelation
{
    /**
        Computes the cross correlation of img1 (about x1,y1) with img2 (about x2,y2).
        The correlation is computed over a window of width ww, and height wh.
        The patches have to be within the images.
        @return 0 if part of the patch is outside the image.
    **/
    float Ncc2d( const IplImage* img1, const IplImage* img2, int x1, int y1, int x2, int y2, int ww, int wh)
    {
        // make window dimensions odd
        if ( ww%2 == 0 )
        {
            ww += 1;
        }
        if ( wh%2 == 0 )
        {
            wh += 1;
        }

        // compute half window sizes:
        int hww = ww/2;
        int hwh = wh/2;

        int w1 = img1->width;
        int h1 = img1->height;
        int step1 = img1->widthStep;
        int c1 = img1->nChannels;

        int w2 = img2->width;
        int h2 = img2->height;
        int step2 = img2->widthStep;
        int c2 = img2->nChannels;

        // boundary checks
        bool ok = true;
        ok &=  ( c1==c2 );
        ok &= ( x1-hww>=0 && x1+hww<w1 );
        ok &= ( y1-hwh>=0 && y1+hwh<h1 );
        ok &= ( x2-hww>=0 && x2+hww<w2 );
        ok &= ( y2-hwh>=0 && y2+hwh<h2 );
        //assert(ok);

        if ( !ok )
        {
            return 0.f;
        }

        // setup image row pointers
        const char* pStart1 = img1->imageData + ( (y1-hwh) * step1 ) + ( (x1-hww) * c1 );
        const char* pEnd1 = pStart1 + ( ww * c1 );

        const char* pStart2 = img2->imageData + ( (y2-hwh) * step2 ) + ( (x2-hww) * c2 );
        const char* pEnd2 = pStart2 + ( ww * c2 );

        // compute means
        unsigned int mean1 = 0;
        unsigned int mean2 = 0;
        unsigned int count = 0;
        for ( int y=0 ; y<wh; ++y )
        {
            const char* pImg1 = pStart1;
            pStart1 += step1;
            const char* pImg2 = pStart2;
            pStart2 += step2;

            while ( pImg2 != pEnd2 )
            {
                unsigned char val1 = *pImg1++;
                unsigned char val2 = *pImg2++;
                mean1 += val1;
                mean2 += val2;
                count++;
            }

            pEnd1 += step1;
            pEnd2 += step2;
        }

        mean1 /= count;
        mean2 /= count;

        // Reset row pointers
        pStart1 = img1->imageData + ( (y1-hwh) * step1 ) + ( (x1-hww) * c1 );
        pEnd1 = pStart1 + ( ww * c1 );

        pStart2 = img2->imageData + ( (y2-hwh) * step2 ) + ( (x2-hww) * c2 );
        pEnd2 = pStart2 + ( ww * c2 );

        // Compute cross-correlation
        int corr = 0;
        unsigned int sq1 = 0;
        unsigned int sq2 = 0;
        for ( int y=0; y<wh; ++y )
        {
            const char* pImg1 = pStart1;
            pStart1 += step1;
            const char* pImg2 = pStart2;
            pStart2 += step2;

            while ( pImg2 != pEnd2 )
            {
                unsigned char val1 = *pImg1++;
                unsigned char val2 = *pImg2++;

                int diff1 = val1 - mean1;
                int diff2 = val2 - mean2;

                corr+= diff1*diff2;
                sq1 += diff1*diff1;
                sq2 += diff2*diff2;
            }

            pEnd1 += step1;
            pEnd2 += step2;
        }

        // compute and return normalised cross correlation
        float denom = sqrtf( sq1 ) * sqrtf( sq2 );

        return corr/denom;
    }

    /**
        Same as ncc2d but contributions of individual pixels are weighted by a radial mask.
    **/
    float Ncc2dRadial( const IplImage* img1, const IplImage* img2, int x1, int y1, int x2, int y2, int ww, int wh)
    {
        // make window dimensions odd
        if ( ww%2 == 0 )
        {
            ww += 1;
        }
        if ( wh%2 == 0 )
        {
            wh += 1;
        }

        // compute half window sizes:
        int hww = ww/2;
        int hwh = wh/2;

        int w1 = img1->width;
        int h1 = img1->height;
        int step1 = img1->widthStep;
        int c1 = img1->nChannels;

        int w2 = img2->width;
        int h2 = img2->height;
        int step2 = img2->widthStep;
        int c2 = img2->nChannels;

        // boundary checks
        bool ok = true;
        ok &=  ( c1==c2 );
        ok &= ( x1-hww>=0 && x1+hww<w1 );
        ok &= ( y1-hwh>=0 && y1+hwh<h1 );
        ok &= ( x2-hww>=0 && x2+hww<w2 );
        ok &= ( y2-hwh>=0 && y2+hwh<h2 );
        //assert(ok);

        if (!ok)
        {
            return 0.f;
        }

        // setup image row pointers
        const char* pStart1 = img1->imageData + ( (y1-hwh) * step1 ) + ( (x1-hww) * c1 );
        const char* pEnd1 = pStart1 + ( ww * c1 );

        const char* pStart2 = img2->imageData + ( (y2-hwh) * step2 ) + ( (x2-hww) * c2 );
        const char* pEnd2 = pStart2 + ( ww * c2 );

        // compute weighted means
        double mean1 = 0.0;
        double mean2 = 0.0;
        double wsum = 0.0;

        float ry, ry2;
        float rx, r;
        float w;
        float windowDiagonal = sqrtf(ww*ww + wh*wh);
        float radius = windowDiagonal*0.4f;

        for ( int y=0 ; y<wh; ++y )
        {
            const char* pImg1 = pStart1;
            const char* pImg2 = pStart2;

            ry = y-hwh;
            ry2 = ry*ry;

            while ( pImg2 != pEnd2 )
            {
                rx = pImg1-pStart1-hww;
                r = sqrtf(rx*rx + ry2);
                w = expf( -powf( r/radius, 8.f ) ); // Compute radial weighting
                wsum += w;
                //*pImg2 = 255*w;

                unsigned char val1 = *pImg1++;
                unsigned char val2 = *pImg2++;

                mean1 += w*val1;
                mean2 += w*val2;
            }

            pStart1 += step1;
            pStart2 += step2;
            pEnd1 += step1;
            pEnd2 += step2;
        }

        mean1 /= wsum;
        mean2 /= wsum;

        // Reset row pointers
        pStart1 = img1->imageData + ( (y1-hwh) * step1 ) + ( (x1-hww) * c1 );
        pEnd1 = pStart1 + ( ww * c1 );

        pStart2 = img2->imageData + ( (y2-hwh) * step2 ) + ( (x2-hww) * c2 );
        pEnd2 = pStart2 + ( ww * c2 );

        // Compute cross-correlation
        double corr = 0.0;
        double sq1  = 0.0;
        double sq2  = 0.0;
        for ( int y=0; y < wh; ++y )
        {
            const char* pImg1 = pStart1;
            const char* pImg2 = pStart2;

            ry = y-hwh;
            ry2 = ry*ry;

            while ( pImg2 != pEnd2 )
            {
                rx = pImg1-pStart1-hww;
                r = sqrtf(rx*rx + ry2);
                w = expf( -powf( r/radius, 8.f ) ); // Compute radial weighting

                unsigned char val1 = *pImg1++;
                unsigned char val2 = *pImg2++;

                int diff1 = val1 - mean1;
                int diff2 = val2 - mean2;

                corr += w*w*diff1*diff2;
                sq1  += w*w*diff1*diff1;
                sq2  += w*w*diff2*diff2;
            }

            pEnd1 += step1;
            pEnd2 += step2;
            pStart1 += step1;
            pStart2 += step2;
        }

        // compute and return normalised cross correlation
        float denom = sqrtf( sq1 * sq2 );
        float ncc = corr/denom;
        return ncc;
    }

    /**
        Extract a patch from the image into a uchar array.
    **/
    void GetPatch(Patch* patch)
    {
        Q_UNUSED(patch);
        assert( (patch->width * patch->height) < PATCH_SIZE );
    }
}
