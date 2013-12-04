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

#ifndef CROSSCORRELATION_H
#define CROSSCORRELATION_H

#include <opencv/cv.h>

#define PATCH_SIZE 256

namespace CrossCorrelation
{
    struct Patch
    {
	    int width;
	    int height;

	    unsigned char pData[PATCH_SIZE];
    };

    float Ncc2d(const IplImage*,
                 const IplImage*,
                 int x1,
                 int y1,
                 int x2,
                 int y2,
                 int w,
                 int h);

    float Ncc2dRadial(const IplImage*,
                       const IplImage*,
                       int x1,
                       int y1,
                       int x2,
                       int y2,
                       int w,
                       int h);

    void GetPatch(Patch* patch);

    float NccPatch(const IplImage* img, int x1, int y1, const unsigned char* patch, int width, int height);
}

#endif // CROSSCORRELATION_H
