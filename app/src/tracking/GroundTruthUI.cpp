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

#include "GroundTruthUI.h"

#include "GtsView.h"
#include "KltTracker.h"
#include "CameraCalibration.h"
#include "RobotMetrics.h"
#include "CoverageSystem.h"
#include "ScanUtility.h"
#include "MathsConstants.h"

#include <opencv/highgui.h>

#include <QtGui/QPainter>
#include <QtGui/QPen>

#include <stdio.h>

namespace GroundTruthUI
{
    /**
     Draw the robot position in the original image sequence.
     Need to know the entire camera calibration (and more) for this
     **/
    QImage showRobotTrackUndistorted(IplImage*           img,        ///< original avi image
                                      const RobotTracker* tracker,    ///< robot tracker
                                      int                 flip)      ///< flip output coords?
    {
        // Convert from IplImage to QImage
        const QSize imgSize(img->width,img->height);
        QImage qimage = QImage(imgSize, QImage::Format_RGB888);

        CvMat mtxWrapper;
        cvInitMatHeader(&mtxWrapper,
                         img->height,
                         img->width,
                         CV_8UC3,
                         qimage.bits());

        cvConvertImage(img, &mtxWrapper, flip);

        // Now draw on top of the QImage

        QPainter painter(&qimage);
        painter.setRenderHint(QPainter::Antialiasing, true);

        QPen penRed(QColor(255,0,0));
        QPen penBlue(QColor(0,0,255));
        QPen penRedGreen(QColor(255,255,0));

        CvPoint2D32f offset = *tracker->GetOffsetParams();
        const CameraCalibration* cal = tracker->GetCalibration();

        // Draw the path taken so far (but each point needs to be converted from plane, to image co-ordinates)
        const TrackHistory::TrackLog& history = tracker->GetHistory();

        if (history.size() > 1)
        {
            CvPoint2D32f pos = history[0].GetPosition();
            pos.x += offset.x;
            pos.y += offset.y;
            CvPoint2D32f oldPosf = cal->PlaneToImage(pos);

            if (flip)
            {
                oldPosf.y = img->height - oldPosf.y;
            }

            CvPoint oldPos = cvPoint((int)oldPosf.x, (int)oldPosf.y);

            for (unsigned int i = 1; i < history.size(); ++i)
            {
                double tdiff = fabs(history[i].t() - history[i - 1].t());

                if (tdiff > 2000.0 / 3.0)
                {
                    pos = history[i].GetPosition();
                    pos.x += offset.x;
                    pos.y += offset.y;
                    oldPosf = cal->PlaneToImage(pos);

                    if (flip)
                    {
                        oldPosf.y = img->height - oldPosf.y;
                    }

                    oldPos = cvPoint((int)oldPosf.x, (int)oldPosf.y);
                }
                else
                {
                    pos = history[i].GetPosition();
                    pos.x += offset.x;
                    pos.y += offset.y;
                    CvPoint2D32f newPosf = cal->PlaneToImage(pos);

                    if (flip)
                    {
                        newPosf.y = img->height - newPosf.y;
                    }

                    CvPoint newPos = cvPoint((int)newPosf.x, (int)newPosf.y);

                    //CvScalar colour;

                    //float err = expf(-powf((1.f-history[i].GetError())/0.5f, 4));
                    //float col = 255*err;
                    //colour = CV_RGB(255,0,0); //cvScalar(col, col, 255);
                    //  if (err > 0.35f)
                    //  {
                    //      colour = cvScalar(0,255,0);
                    //  }
                    //  else
                    //  {
                    //      colour = cvScalar(0,0,255);
                    //  }

                    //cvLine(img, oldPos, newPos, colour, 1, CV_AA);

                    penRed.setWidth(2);
                    painter.setPen(penRed);
                    painter.drawLine(oldPos.x, oldPos.y, newPos.x, newPos.y);
                    oldPos = newPos;
                }
            }
        }

        return qimage;
    }

    /**
     Draw the robot into a copy of the tracking image
     before displaying in a named window.

     TODO: this function has grown too big - need to refactor!
     **/
    QImage showRobotTrack(const RobotTracker* tracker, bool tracking)
    {
        const int DONT_FLIP = 0;

        const IplImage* const currentImg = tracker->GetCurrentImage();

        IplImage* img = cvCreateImage(cvSize(currentImg->width,
                                               currentImg->height), IPL_DEPTH_8U, 3);
        cvConvertImage(currentImg, img);

        CvPoint2D32f oldPosf, newPosf;

        // Draw the path taken so far (but each point needs
        // to be converted from plane, to image co-ordinates)
        const TrackHistory::TrackLog& history = tracker->GetHistory();

        if (history.size() > 1)
        {
            CvPoint2D32f pos = history[0].GetPosition();
            float angle = history[0].GetOrientation();
            oldPosf = tracker->AdjustTrackForRobotHeight(pos, angle);
            CvPoint oldPos = cvPoint((int)oldPosf.x, (int)oldPosf.y);

            for (unsigned int i = 1; i < history.size(); ++i)
            {
                double tdiff = fabs(history[i].t() - history[i - 1].t());
                pos = history[i].GetPosition();
                angle = history[i].GetOrientation();

                if (tdiff > 2000.0 / 3.0)
                {
                    oldPosf = tracker->AdjustTrackForRobotHeight(pos, angle);
                    oldPos = cvPoint((int)oldPosf.x, (int)oldPosf.y);
                }
                else
                {
                    newPosf = tracker->AdjustTrackForRobotHeight(pos, angle);
                    CvPoint newPos = cvPoint((int)newPosf.x, (int)newPosf.y);

                    CvScalar colour;
                    colour = cvScalar(0, 0, 255);
                    cvLine(img, oldPos, newPos, colour, 1, CV_AA);

                    oldPos = newPos;
                }
            }
        }

        if (tracking)
        {
            // Draw circle around base of robot
            CvPoint2D32f pos = tracker->GetPosition();
            float angle = tracker->GetHeading();

            float baseRadius = tracker->GetMetrics()->GetBasePx();
            float robotRadius = tracker->GetMetrics()->GetRadiusPx();

            CvPoint2D32f basePos = tracker->AdjustTrackForRobotHeight(pos, angle);
            CvPoint pb = cvPoint((int)(basePos.x + .5f), (int)(basePos.y + .5f));

            cvCircle(img, pb, (int)(baseRadius), CV_RGB(250,250,0), 1, CV_AA);

            // Draw circle around top of robot
            pos = tracker->GetPosition();
            CvPoint p = cvPoint((int)(pos.x + .5f), (int)(pos.y + .5f));

            cvCircle(img, p, (int)(robotRadius + 1.5f), cvScalar(255, 0, 0), 1, CV_AA);

            // Draw heading
            float heading = tracker->GetHeading();
            float offset = MathsConstants::F_PI * (2.0f / 3.0f);
            CvPoint2D32f h;
            CvPoint a, b;

            for (unsigned int c = 0; c < 3; ++c)
            {
                switch (c)
                {
                    case 0:
                    {
                        angle = heading;
                        break;
                    }
                    case 1:
                    {
                        angle = heading + offset;
                        break;
                    }
                    case 2:
                    {
                        angle = heading - offset;
                        break;
                    }
                    default:
                        angle = 0.f;
                        break;
                }

                h = cvPoint2D32f(cos(-angle), sin(-angle));
                a = cvPoint((int)(p.x - ((robotRadius - 5.f) * h.x + .5f)),
                             (int)(p.y - ((robotRadius - 5.f) * h.y + .5f)));
                b = cvPoint((int)(p.x - ((robotRadius + 7.f) * h.x + .5f)),
                             (int)(p.y - ((robotRadius + 7.f) * h.y + .5f)));

                cvLine(img, a, b, cvScalar(255, 0, 0), 2, CV_AA);
            }

            // Draw brush bar
            CvPoint2D32f pl = tracker->GetBrushBarLeft(basePos, heading);
            CvPoint2D32f pr = tracker->GetBrushBarRight(basePos, heading);

            cvLine(img, cvPoint((int)pl.x, (int)pl.y),
                         cvPoint((int)pr.x, (int)pr.y),
                         cvScalar(0, 0, 255),
                         3,
                         CV_AA);
        }

        // Convert from IplImage to QImage
        const QSize imgSize(img->width, img->height);
        QImage qimage = QImage(imgSize, QImage::Format_RGB888);

        CvMat mtxWrapper;
        cvInitMatHeader(&mtxWrapper,
                         img->height,
                         img->width,
                         CV_8UC3,
                         qimage.bits());

        cvConvertImage(img, &mtxWrapper, DONT_FLIP);

        cvReleaseImage(&img);

        return qimage;
    }
}
