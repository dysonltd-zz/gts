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

#ifndef SCANUTILITY_H
#define SCANUTILITY_H

#include "ScanMatch.h"
#include "TrackHistory.h"

class CameraCalibration;

namespace ScanUtility
{
    double AverageFpsSec(const TrackHistory::TrackLog& in);

    void PlotLog(const TrackHistory::TrackLog& log,
                  IplImage* img,
                  CvScalar col,
                  CvRect rect,
                  int margin,
                  int scale,
                  double timeThresh);

    void PlotPoint(const TrackEntry& entry,
                    IplImage* img,
                    CvScalar col,
                    CvRect rect,
                    int margin,
                    int scale);

    void TransformLog(const TrackHistory::TrackLog& log,
                       TrackHistory::TrackLog& newlog,
                       const CvMat* H);

    void LogCmToPx(const TrackHistory::TrackLog& in,
                    TrackHistory::TrackLog& out,
                    float scale,
                    CvPoint2D32f offset);

    void LogPxToCm(const TrackHistory::TrackLog& in,
                    TrackHistory::TrackLog& out,
                    float scale,
                    CvPoint2D32f offset);

    void LogPxToImage(const TrackHistory::TrackLog& in,
                       TrackHistory::TrackLog& out,
                       const CameraCalibration* cal,
                       const CvPoint2D32f* offset);

    void LogImageToPx(const TrackHistory::TrackLog& in,
                       TrackHistory::TrackLog& out,
                       const CameraCalibration* cal,
                       const CvPoint2D32f* offset);

    void LogSwapHandedness(TrackHistory::TrackLog& log);

    void ConvertToRelativeLog(const TrackHistory::TrackLog& in,
                               TrackHistory::TrackLog& out);
}

#endif // SCANUTILITY_H
