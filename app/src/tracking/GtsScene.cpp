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

#include "GtsScene.h"

#include "ScanMatch.h"
#include "ScanUtility.h"
#include "MathsConstants.h"
#include "CameraCalibration.h"
#include "CoverageSystem.h"
#include "ImageGrid.h"

#include "GroundPlaneUtility.h"

#include "RobotMetricsSchema.h"
#include "ExtrinsicCalibrationSchema.h"
#include "TrackRobotSchema.h"
#include "FloorPlanSchema.h"
#include "TargetSchema.h"

#include "TrackRobotWidget.h"

#include "FileUtilities.h"

#include "Logging.h"

#include <opencv/highgui.h>

#include <QTemporaryFile>
#include <QObject>

#include <stdio.h>

#include <iostream>
#include <fstream>
#include <string>
#include <set>

#define MATCH_BASE 0
#define MATCH_REFN 1

#define MATCH_NONE -1

GtsScene::GtsScene( ) :
    m_thread                    ( 0 ),
    m_filePositionInMilliseconds( 0.0 ),
    m_rateInMilliseconds        ( 0.0 ),
    m_ln                        ( 0 )
{
}

GtsScene::~GtsScene()
{
}

void GtsScene::Reset()
{
    delete m_thread;

    for ( unsigned int i = 0; i < GetNumMaxCameras(); ++i )
    {
        if ( m_view[i].IsSetup() )
        {
            m_view[i].Reset();
        }
    }

    m_ln = 0;
    m_filePositionInMilliseconds = 0.0;
}

bool GtsScene::LoadTarget( const WbConfig& targetCfg )
{
    bool successful = true;

    const KeyValue imgKey = targetCfg.GetKeyValue( TargetSchema::trackImgKey );
    m_targetFile = targetCfg.GetAbsoluteFileNameFor( imgKey.ToQString() );

    return successful;
}

bool GtsScene::LoadCameraConfig( const KeyId               camPosId,
                                 const char* const         selectedVideoFileName,
                                 const char* const         timestampFileName,
                                 const WbConfig&           cameraConfig,
                                 const WbConfig&           camPosConfig,
                                 const WbConfig&           roomConfig,
                                 const WbConfig&           robotConfig,
                                 const WbConfig&           trackConfig,
                                 RobotTracker::trackerType tracker )
{
    LOG_INFO(QObject::tr("Getting camera parameters %1.").arg(m_ln));

    const WbKeyValues::ValueIdPairList cameraMappingIds =
        trackConfig.GetKeyValues( TrackRobotSchema::PerCameraTrackingParams::positionIdKey );

    int biLevelThreshold = trackConfig.GetKeyValue(TrackRobotSchema::GlobalTrackingParams::biLevelThreshold).ToInt();
    double nccThreshold = trackConfig.GetKeyValue(TrackRobotSchema::GlobalTrackingParams::nccThreshold).ToDouble();
    int resolution = trackConfig.GetKeyValue(TrackRobotSchema::GlobalTrackingParams::resolution).ToInt();

    for (WbKeyValues::ValueIdPairList::const_iterator it = cameraMappingIds.begin(); it != cameraMappingIds.end(); ++it)
    {
        const KeyId keyId( trackConfig.GetKeyValue( TrackRobotSchema::PerCameraTrackingParams::positionIdKey, it->id ).ToKeyId() );

        if (keyId == camPosId)
        {
            bool useGlobalParams = trackConfig.GetKeyValue(TrackRobotSchema::PerCameraTrackingParams::useGlobalParams, it->id).ToBool();

            if (!useGlobalParams)
            {
                biLevelThreshold = trackConfig.GetKeyValue(TrackRobotSchema::PerCameraTrackingParams::biLevelThreshold, it->id).ToInt();
                nccThreshold = trackConfig.GetKeyValue(TrackRobotSchema::PerCameraTrackingParams::nccThreshold, it->id).ToDouble();
                resolution = trackConfig.GetKeyValue(TrackRobotSchema::PerCameraTrackingParams::resolution, it->id).ToInt();
            }

            break;
        }
    }

    // Get the floor plan configuration
    const WbConfig& floorPlanConfig = roomConfig.GetSubConfig( FloorPlanSchema::schemaName );

    // Get the robot metrics configuration
    const WbConfig& metricsConfig = robotConfig.GetSubConfig( RobotMetricsSchema::schemaName );

    const WbConfig& camPosCalConfig = camPosConfig.GetSubConfig( ExtrinsicCalibrationSchema::schemaName );

    LOG_INFO(QObject::tr("Configuring camera %1.").arg(m_ln));

    LOG_INFO(QObject::tr("Tracking param - biLevel: %1.").arg(biLevelThreshold));
    LOG_INFO(QObject::tr("Tracking param - ncc: %1.").arg(nccThreshold));
    LOG_INFO(QObject::tr("Tracking param - resolution: %1.").arg(resolution));

    m_view[m_ln].SetId( m_ln );

    m_view[m_ln].LoadMetrics ( metricsConfig, camPosCalConfig, resolution );

    bool status = m_view[m_ln].SetupCalibration( camPosId,
                                                 cameraConfig,
                                                 camPosConfig,
                                                 floorPlanConfig,
                                                 camPosCalConfig,
                                                 m_view[m_ln].GetMetrics() );
    if ( !status )
    {
        LOG_ERROR("Calibration setup failed!");

        return false;
    };

    LOG_INFO(QObject::tr("Configuring tracker %1.").arg(m_ln));

    status = m_view[m_ln].SetupTracker( tracker,
                                        m_view[m_ln].GetMetrics(),
                                        //*m_metrics,
                                        m_targetFile.toAscii().data(),
                                        biLevelThreshold );
    if ( !status )
    {
        LOG_ERROR("Tracker setup failed!");

        return false;
    };

    m_view[m_ln].SetTrackerParam( RobotTracker::PARAM_NCC_THRESHOLD, nccThreshold );

    float shutter = 411;
    float gain = 75;

    switch (tracker)
    {
        case RobotTracker::KLT_TRACKER :
        {
            shutter = -1;
            gain = -1;
        }
        break;
    }

    LOG_INFO(QObject::tr("Setting up video %1.").arg(m_ln));

    status = m_view[m_ln].SetupVideo( selectedVideoFileName, timestampFileName, shutter, gain );
    if ( !status )
    {
        LOG_ERROR("Video setup failed!");

        return false;
    }

    LOG_INFO("Done.");

    m_ln++;

    return true;
}

/**
 Create OpenCV named windows for displaying tracker results.
 Also setup mouse call backs for each window.
 **/
void GtsScene::SetupViewWindows( TrackRobotWidget* tool, ImageGrid* imageGrid )
{
    for ( unsigned int i = 0; i < GetNumMaxCameras(); ++i )
    {
        if ( m_view[i].IsSetup() )
        {
            m_view[i].SetupView( tool, imageGrid );
        }
    }
}

/**
 Destroy all OpenCV named windows.
 **/
void GtsScene::DestroyViewWindows( ImageGrid* imageGrid )
{
    imageGrid->Clear();
}

/**
  @return a TrackResult indicating how many trackers are currently active,
  and how many of those are lost.
 **/
GtsScene::TrackStatus GtsScene::StepTrackers( const bool forward, const bool seek )
{
    m_filePositionInMilliseconds = forward ? m_filePositionInMilliseconds+m_rateInMilliseconds : MAX(m_filePositionInMilliseconds-m_rateInMilliseconds, 0);
    TrackStatus status = { m_filePositionInMilliseconds, 0, 0, false };

    for (unsigned int i = 0; i < GetNumMaxCameras(); ++i)
    {
        if ( m_view[i].IsSetup() )
        {
            bool ready;
            if ( seek )
            {
                ready = m_view[i].ReadySeekFrame( m_filePositionInMilliseconds );
            }
            else
            {
                ready = m_view[i].ReadyNextFrame();
                if ( ready )
                {
                    m_filePositionInMilliseconds = m_view[i].GetSeekPositionInMilliseconds();
                }
            }

            if ( ready && m_view[i].GetNextFrame() )
            {
                m_view[i].StepTracker( forward );

                RobotTracker& tracker = m_view[i].GetTracker();

                if ( tracker.IsLost() )
                {
                    status.numTrackersLost++;

                    // Note that "lost" trackers are also
                    // considered active for this purpose

                    status.numTrackersActive++;
                }
                else if ( tracker.IsActive() )
                {
                    status.numTrackersActive++;
                }
            }
            else
            {
                status.eof = true;
            }
        }
    }

    return status;
}

void GtsScene::SetupThread( TrackRobotWidget* tool )
{
    m_thread = new TrackThread( *this );

    // Note, thread "runs" on creation)!

    QObject::connect((QObject*)m_thread,
                     SIGNAL( paused( bool ) ),
                     (QObject*)tool,
                     SLOT( ThreadPaused( bool ) ),
                     Qt::AutoConnection );

    QObject::connect((QObject*)m_thread,
                     SIGNAL( finished() ),
                     (QObject*)tool,
                     SLOT( ThreadFinished() ),
                     Qt::AutoConnection );

    QObject::connect((QObject*)m_thread,
                     SIGNAL( position( double ) ),
                     (QObject*)tool,
                     SLOT( SetPosition ( double ) ),
                     Qt::AutoConnection );
}

void GtsScene::StartThread( double rate, bool trackingActive,
                                         bool singleStep,
                                         bool runForward )
{
    if ( !trackingActive )
    {
        for ( unsigned int i = 0; i < GetNumMaxCameras(); ++i )
        {
            if ( m_view[i].IsSetup() )
            {
                m_view[i].GetTracker().Deactivate();
            }
        }
    }

    runForward ? m_thread->RunForward() : m_thread->RunBackward();

    trackingActive ? m_thread->TrackingOn() : m_thread->TrackingOff();
    singleStep ? m_thread->SteppingOn() : m_thread->SteppingOff();

    m_rateInMilliseconds = rate;

    m_thread->Run();
}

void GtsScene::PauseThread()
{
    m_thread->Pause();
}

void GtsScene::StopThread()
{
    m_thread->Stop();
}

void GtsScene::SetRate( double rate )
{
    m_rateInMilliseconds = rate;
}

void GtsScene::PostProcessMultiCamera( TrackHistory::TrackLog& avg,
                                       CvPoint2D32f&           offset,
                                       IplImage**              compImgCol,
                                       float                   timeThresh,
                                       char*                   floorPlanFile,
                                       unsigned int            baseIndex,
                                       QString trackResultsTemplate )
{
    IplImage* compImg;

    CvScalar colours[4] = { CV_RGB( 0,   0,   255 ),
                            CV_RGB( 0,   255, 0   ),
                            CV_RGB( 255, 255, 0   ),
                            CV_RGB( 0,   255, 255 ) };

    IplImage* baseImg = cvLoadImage(floorPlanFile, CV_LOAD_IMAGE_GRAYSCALE);

    compImg = cvCloneImage( baseImg );

    // Plot logs - colour copy of composite image
    *compImgCol = cvCreateImage( cvSize( compImg->width,
                                         compImg->height ), compImg->depth, 3 );
    cvConvertImage( compImg, *compImgCol );

    for ( unsigned int i = 0; i < GtsScene::kMaxCameras; ++i )
    {
        if (m_view[i].IsSetup())
        {
            LOG_TRACE(QObject::tr("Processing view [ %1 ]").arg(i));

            offset.x = -m_origin[i].x;
            offset.y = -m_origin[i].y;

            // Reverse effect of previous
            // call to ConvertTrackToCm()
            ScanUtility::LogCmToPx( m_log[i],
                                    m_logPx[i],
                                    m_view[i].GetMetrics().GetScaleFactor(),
                                    offset );

            TrackHistory::TrackLog m_logImage;

            // Map px to image using scaled calibration parameters
            ScanUtility::LogPxToImage( m_logPx[i], m_logImage, m_view[i].GetScaledCalibration(),
                                                               m_view[i].GetScaledCalibration()->GetUnwarpOffset() );

            // Map image (back) to px using non-scaled calibration parameters
            ScanUtility::LogImageToPx( m_logImage, m_logPx[i], m_view[i].GetNormalCalibration(),
                                                               m_view[i].GetNormalCalibration()->GetUnwarpOffset() );

            TrackHistory::TrackLog tlog;
            // Transform then overwrite old log with transformed log
            ScanUtility::TransformLog( m_logPx[i], tlog, m_view[i].GetTracker().GetCalibration()->GetCameraTransform() );
            m_logPx[i] = tlog;

            // Write the individual transformed logs as these can be useful for external analysis:
            const QString fileName(FileUtilities::GetUniqueFileName(trackResultsTemplate));
            TrackHistory::WriteHistoryLog( fileName.toAscii().data(), tlog );

            ScanUtility::PlotLog( m_logPx[i],
                                 *compImgCol,
                                 colours[(i + 1) % 4],
                                 cvRect( 0, 0, 0, 0 ),
                                 0,
                                 1,
                                 timeThresh );
        }
    }

    avg = m_logPx[baseIndex];

    for ( unsigned int i = 1; i < GtsScene::kMaxCameras; ++i )
    {
        if (m_view[i].IsSetup())
        {
            if (i != baseIndex)
            {
                TrackHistory::TrackLog tmpLog;

                if ( avg.empty() )
                {
                    avg = m_logPx[i];
                }
                else if ( m_logPx[i].empty() )
                {
                    // just continue
                }
                else
                {
                    ScanMatch::ScanAverage( avg, m_logPx[i], 7.5f, tmpLog );
                    avg = tmpLog;
                }
            }
        }
    }

    /*ScanUtility::PlotLog( avg,
                          *compImgCol,
                          cvScalar( 255, 0, 0, 128 ),
                          cvRect( 0, 0, 0, 0 ),
                          0,
                          1,
                          timeThresh );*/

    cvReleaseImage( &compImg );
    cvReleaseImage( &baseImg );
}

/**
 Ground truth system post processing. Deals with the case of
 single or multiple cameras, computing a single output log
 in both cases.
 **/
void GtsScene::SaveData( char* floorPlanFile,
                         char* trackerResultsTxtFile,
                         char* trackerResultsCsvFile,
                         char* trackerResultsImgFile,
                         char* pixelOffsetsFile,
                         QString trackResultsTemplate,
                         QString pixelOffsetsTemplate )
{
    int baseLog = OrganiseLogs( m_log, pixelOffsetsTemplate );

    IplImage* compImg = 0;
    IplImage* compImgCol = 0;
    CvPoint2D32f offset;
    TrackHistory::TrackLog avg;
    float tx = 0.f;
    float ty = 0.f;

    // Seconds - prob not discts within half sec. using
    const float timeThresh = 0.5f; // 2.0f / (float)m_fps;

    if ( baseLog < 0)
        return;

    offset.x = -m_origin[0].x;
    offset.y = -m_origin[0].y;

    PostProcessMultiCamera( avg, offset, &compImgCol, timeThresh, floorPlanFile, baseLog, trackResultsTemplate );

    // Write composite image to file
    cvSaveImage( trackerResultsImgFile, compImgCol );

    // Write average log to file
    if ( trackerResultsTxtFile )
    {
        TrackHistory::WriteHistoryLog( trackerResultsTxtFile, avg );
    }

    if ( trackerResultsCsvFile )
    {
        TrackHistory::WriteHistoryCsv( trackerResultsCsvFile, avg );
    }

    // Write origin-offset to file
    FILE* fp = fopen( pixelOffsetsFile, "w" );

    if ( fp )
    {
        fprintf( fp, "%f %f\n", tx, ty );
        fprintf( fp, "%f %f", offset.x, offset.y );

        fclose( fp );
    }
    else
    {
        LOG_ERROR("Could not write pixel offsets!");
    }

    // Clean up
    cvReleaseImage( &compImg );
    cvReleaseImage( &compImgCol );
}

/**
 * Take logs from m_view objects and convert them
 * for processing then store into specified array.
 *
 * Also computes average frame-rate over all logs.
 **/
int GtsScene::OrganiseLogs( TrackHistory::TrackLog* log,
                            QString pixelOffsetsTemplate )
{
    Q_UNUSED(pixelOffsetsTemplate);

    int baseLog = -1;
    bool first = true;
    unsigned int nLogs = 0;
    m_fps = 0.0;

    for ( unsigned int i = 0; i < GetNumMaxCameras(); ++i )
    {
        if ( m_view[i].IsSetup() )
        {
            m_view[i].GetTracker().ConvertLogForProcessing( log[i], false );

            // Ignore empty logs!
            if ( log[i].size() > 1 )
            {
                if (first)
                {
                    baseLog = i;
                    first = false;
                }

                // Organise ground plane images and origins too
                m_origin[i] = *(m_view[i].GetTracker().GetCalibration()->GetUnwarpOffset());

                m_origin[i].x = -m_origin[i].x;
                m_origin[i].y = -m_origin[i].y;

                m_gpImg[i] = m_view[i].GetTracker().GetCalibration()->GetWarpedCalibrationImage();

                m_fps += ScanUtility::AverageFpsSec( log[i] );

                // Do some checks!
                assert( m_gpImg[i] != 0 );

                LOG_INFO(QObject::tr("Origin %1: %2 %3.").arg(nLogs)
                                                         .arg(m_origin[i].x)
                                                         .arg(m_origin[i].y));
                nLogs++;
            }
        }
    }

    m_fps /= nLogs;

    LOG_INFO(QObject::tr("Average video rate: %1 (fps).").arg(m_fps));

    return baseLog;
}

void GtsScene::SetTrackPosition( int id, int x, int y )
{
    m_view[id].GetTracker().SetPosition( cvPoint2D32f(x, y) );
    m_view[id].GetTracker().Activate();

    m_view[id].ShowRobotTrack();
}

void GtsScene::ClrTrackPosition( int id )
{
    m_view[id].GetTracker().Deactivate();
    m_view[id].HideRobotTrack();
}

