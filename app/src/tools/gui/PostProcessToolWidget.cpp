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

#include "PostProcessToolWidget.h"

#include "ui_PostProcessToolWidget.h"

#include "UnknownLengthProgressDlg.h"

#include "ScanUtility.h"
#include "CoverageSystem.h"
#include "KltTracker.h"

#include "TrackModel.h"

#include "RoomsCollection.h"
#include "RobotsCollection.h"
#include "CameraPositionsCollection.h"

#include "TrackRobotSchema.h"
#include "ExtrinsicCalibrationSchema.h"
#include "RoomLayoutSchema.h"
#include "RobotMetricsSchema.h"
#include "FloorPlanSchema.h"
#include "RunSchema.h"

#include "Message.h"

#include "Sweeper.h"

#include "Logging.h"

#include <QtCore/QDir>
#include <QtGui/QMessageBox>
#include <QFileDialog>
#include <QApplication>
#include <QItemSelectionModel>
#include <QAction>

#include <sstream>

#ifdef _WIN32
#include "windows.h"
#else
#define MAX_PATH 255
#endif

PostProcessToolWidget::PostProcessToolWidget( QWidget* parent ) :
    Tool( parent, CreateSchema() ),
    m_ui( new Ui::PostProcessToolWidget )
{
    m_ui->setupUi( this );

    QObject::connect( m_ui->m_loadDataBtn,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( LoadDataButtonClicked() ) );
    QObject::connect( m_ui->m_postProcessBtn,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( PostProcessButtonClicked() ) );

    // set up delete button
    QAction* const toggleAction = new QAction( tr( "Toggle" ), m_ui->m_trackResults );
    toggleAction->setShortcut( QKeySequence( QKeySequence::Delete ) );
    toggleAction->setShortcutContext( Qt::WidgetShortcut );
    QObject::connect( toggleAction,
             SIGNAL( triggered() ),
             this,
             SLOT( ToggleItemTriggered() ) );

    m_ui->m_trackResults->addAction( toggleAction );
    m_ui->m_trackResults->setContextMenuPolicy( Qt::ActionsContextMenu );
}

PostProcessToolWidget::~PostProcessToolWidget()
{
    delete m_ui;
}

const QString PostProcessToolWidget::GetSubSchemaDefaultFileName() const
{
    return "postProcess.xml";
}

const WbSchema PostProcessToolWidget::CreateSchema()
{
    WbSchema schema( CreateWorkbenchSubSchema( KeyName( "postProcessing" ), tr( "Post Processing" ) ) );

    return schema;
}

void PostProcessToolWidget::LoadDataButtonClicked()
{
    const WbConfig runConfig( GetCurrentConfig().GetParent() );

     const QString fileName(
         runConfig.GetAbsoluteFileNameFor( "results/track_result_raw.csv" ) );

    m_ui->m_trackView->loadFloorPlan( runConfig );
    m_ui->m_trackView->loadMetrics( runConfig );

    QFile csv(fileName);
    csv.open(QIODevice::ReadOnly);

    m_resultsModel = new TrackModel( &csv, m_ui->m_trackResults, true, ',' );

    csv.close();

    m_ui->m_trackResults->setModel(m_resultsModel);
    m_ui->m_trackView->setModel(m_resultsModel);

    m_selectionModel = m_ui->m_trackResults->selectionModel();

    QObject::connect( m_selectionModel,
                      SIGNAL( selectionChanged ( const QItemSelection&,
                                                 const QItemSelection& ) ),
                      m_ui->m_trackView,
                      SLOT( selectionChanged( const QItemSelection&,
                                              const QItemSelection& ) ) );

    m_ui->m_trackResults->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    m_ui->m_trackResults->show();
    m_ui->m_trackView->show();
}

void PostProcessToolWidget::ToggleItemTriggered()
{
    QModelIndexList indexList = m_selectionModel->selectedRows();

    foreach (QModelIndex index, indexList)
    {
        bool value = m_resultsModel->data(m_resultsModel->index(index.row(), 0), TrackModel::IS_DELETED).toBool();

        m_resultsModel->setData(index, QVariant(!value), TrackModel::IS_DELETED);
    }
}

void PostProcessToolWidget::PostProcessButtonClicked()
{
    const WbConfig& config = GetCurrentConfig();

    LOG_TRACE("Post Process...");

    bool successful = true;

    Collection m_rooms = RoomsCollection();

    const WbConfig runConfig( config.GetParent() );
    const WbConfig trackConfig( runConfig.GetSubConfig( TrackRobotSchema::schemaName ) );

    m_rooms.SetConfig( runConfig );

    if ( successful )
    {
        const KeyValue roomId = runConfig.GetKeyValue( RunSchema::roomIdKey );
        const WbConfig roomConfig = m_rooms.ElementById( roomId.ToKeyId() );

        const WbConfig roomLayoutConfig(
                    roomConfig.GetSubConfig( RoomLayoutSchema::schemaName ) );

        const QString floorPlanName(
                    roomLayoutConfig.GetAbsoluteFileNameFor( "floor_plan.png" ) );
        const QString floorMaskName(
                    roomLayoutConfig.GetAbsoluteFileNameFor( "floor_mask.png" ) );

        const QString trackerResultsName(
                    runConfig.GetAbsoluteFileNameFor( "results/track_result_out.csv" ) );
        const QString pixelOffsetsName(
                    runConfig.GetAbsoluteFileNameFor( "results/pixel_offsets.txt" ) );

        const QString coverageMissedName(
                    runConfig.GetAbsoluteFileNameFor( "results/coverage_missed.png" ) );
        const QString coverageColourName(
                    runConfig.GetAbsoluteFileNameFor( "results/coverage_colour.png" ) );
        const QString coverageHistogramName(
                    runConfig.GetAbsoluteFileNameFor( "results/coverage_histogram.txt" ) );
        const QString coverageOverlayName(
                    runConfig.GetAbsoluteFileNameFor( "results/coverage_overlay.png" ) );

        const QString coverageIncrementName(
                    runConfig.GetAbsoluteFileNameFor( "results/coverage_increment.txt" ) );
        const QString coverageRelativeName(
                    runConfig.GetAbsoluteFileNameFor( "results/coverage_relative.txt" ) );

        const QString trackHeadingName(
                    runConfig.GetAbsoluteFileNameFor( "results/track_heading.png" ) );

        const QString trackerResultsImgFile(
                    runConfig.GetAbsoluteFileNameFor( "results/track_result_img_out.png" ) );

        m_resultsModel->toCSV(trackerResultsName, true, ',');

        if ( successful )
        {
            UnknownLengthProgressDlg* const progressDialog = new UnknownLengthProgressDlg( this );
            progressDialog->Start( tr( "Working..." ), tr( "" ) );

            ExitStatus::Flags exitCode = PostProcess( config,
                                                      trackerResultsName.toAscii().data(),       // trackerResultsName
                                                      trackerResultsImgFile.toAscii().data(),
                                                      coverageIncrementName.toAscii().data(),    // coverageFile
                                                      floorPlanName.toAscii().data(),            // floorPlanFile
                                                      floorMaskName.toAscii().data(),            // floorMaskFile
                                                      coverageRelativeName.toAscii().data(),     // relativeLogFile
                                                      pixelOffsetsName.toAscii().data(),         // pixelOffsetsFile
                                                      coverageMissedName.toAscii().data(),
                                                      coverageColourName.toAscii().data(),
                                                      coverageHistogramName.toAscii().data(),
                                                      coverageOverlayName.toAscii().data(),
                                                      trackHeadingName.toAscii().data(),
                                                      1.0 );                                     // incTimeStep

            successful = ( exitCode == ExitStatus::OK_TO_CONTINUE );

            if ( successful )
            {
                progressDialog->Complete( tr( "Post Processing Successful" ),
                                          tr( "Results have been computed." ) );
            }
            else
            {
                progressDialog->ForceClose();

                Message::Show( 0,
                               tr( "Post Processing Failed" ),
                               tr( "See the log for details!" ),
                               Message::Severity_Critical );
            }
        }
    }
}

// ----------------------------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------------------

const ExitStatus::Flags PostProcessToolWidget::PostProcess( const WbConfig& postProcConfig,
                                                            char*           trackerResultsFile,    // -log
                                                            char*           trackerResultsImgFile,
                                                            char*           coverageFile,          // -inc
                                                            char*           floorPlanFile,
                                                            char*           floorMaskFile,         // -flr
                                                            char*           relativeLogFile,       // -rel
                                                            char*           pixelOffsetsFile,
                                                            char*           coverageMissedFile,
                                                            char*           coverageColourFile,
                                                            char*           coverageHistogramFile,
                                                            char*           coverageRawFile,
                                                            char*           headingFile,
                                                            double          incTimeStep )          // -incstep
{
    ExitStatus::Flags exitStatus = ExitStatus::OK_TO_CONTINUE;

    Collection m_rooms( RoomsCollection() );
    Collection m_robots( RobotsCollection() );

    m_rooms.SetConfig( postProcConfig );
    m_robots.SetConfig( postProcConfig );

    // Get the run configuration...
    const WbConfig& runConfig = postProcConfig.GetParent();

    // Get the room configuration (for this run)...
    const KeyId roomId = runConfig.GetKeyValue( RunSchema::roomIdKey ).ToKeyId();
    const WbConfig roomConfig = m_rooms.ElementById( roomId );

    // Get the track configuration...
    const WbConfig& trackConfig = runConfig.GetSubConfig( TrackRobotSchema::schemaName );

    // Get the robot configuration (for this run)...
    const KeyId robotId = trackConfig.GetKeyValue( TrackRobotSchema::robotIdKey ).ToKeyId();
    const WbConfig& robotConfig = m_robots.ElementById( robotId );

    // Get the robot metrics configuration...
    const WbConfig& metricsConfig = robotConfig.GetSubConfig( RobotMetricsSchema::schemaName );

    // Get the first camera (position) configuration...
    std::vector<WbConfig> camPosConfigs = GetCameraPositionsConfigs(roomConfig);

    const WbConfig firstCamPosConfig(camPosConfigs.at(0));
    const WbConfig& firstCamPosCalConfig = firstCamPosConfig.GetSubConfig( ExtrinsicCalibrationSchema::schemaName );

    if ( exitStatus == ExitStatus::OK_TO_CONTINUE )
    {
        FILE* incCoverageFile = 0;

        std::vector<Sweeper> sweepers;

        if ( coverageFile )
        {
            incCoverageFile = fopen( coverageFile, "w" );
        }

#if 0
        if ( sweeperFile )
        {
            Sweeper newSweeper( sweeperFile );
            sweepers.push_back( newSweeper );

            LOG_INFO(QObject::tr("Read sweeper (%1) data from %2.").arg(sweepers.size())
                                                                   .arg(sweeperName));

            LOG_INFO(QObject::tr("x = %1, y = %2, angle = %3, width = %4.").arg(newSweeper.x)
                                                                           .arg(newSweeper.y)
                                                                           .arg(newSweeper.a)
                                                                           .arg(newSweeper.w));
        }
#endif

        // Read the tracking results,
        // robot metrics, floor plan,
        // and pixel offsets.

        RobotMetrics metrics;
        TrackHistory::TrackLog avg;
        CvPoint2D32f offset;
        float tx;
        float ty;

        FILE* fp = fopen( pixelOffsetsFile, "r" );

        if ( fp )
        {
            if( fscanf( fp, "%f %f\n", &tx, &ty ) != 2 )
            {
                return ExitStatus::ERRORS_OCCURRED;
            }
            if( fscanf( fp, "%f %f\n", &offset.x, &offset.y ) != 2 )
            {
                return ExitStatus::ERRORS_OCCURRED;
            }
            fclose( fp );
        }
        else
        {
            LOG_ERROR("Could not load pixel offsets from text file!");

            return ExitStatus::ERRORS_OCCURRED;
        }

        if ( !metrics.LoadMetrics( metricsConfig, firstCamPosCalConfig, 1 ) )
        {
            LOG_ERROR("Could not load robot metrics!");

            return ExitStatus::ERRORS_OCCURRED;
        }

        if ( !TrackHistory::ReadHistoryCsv( trackerResultsFile, avg ) )
        {
            LOG_ERROR(QObject::tr("Could not load track log from %1!").arg(trackerResultsFile));

            return ExitStatus::ERRORS_OCCURRED;
        }

        ScanUtility::LogSwapHandedness( avg );

        if ( relativeLogFile )
        {
            TrackHistory::TrackLog rel;
            ScanUtility::ConvertToRelativeLog( avg, rel );
            TrackHistory::WriteHistoryLog( relativeLogFile, rel);
        }

        IplImage* compImgCol = NULL;

        if ( floorPlanFile )
        {
            compImgCol = cvLoadImage( floorPlanFile );
        }

        if ( !compImgCol )
        {
            LOG_ERROR("Could not load floor plan image!");

            return ExitStatus::ERRORS_OCCURRED;
        }

        IplImage* compImg = cvCreateImage( cvSize( compImgCol->width,
                                                   compImgCol->height), IPL_DEPTH_8U, 1 );
        // Colour copy of composite image.
        cvConvertImage( compImgCol, compImg );

        IplImage* headingImg = cvCreateImage( cvSize( compImgCol->width,
                                                      compImgCol->height), IPL_DEPTH_8U, 1 );
        // Colour copy of composite image.
        cvConvertImage( compImgCol, headingImg );

        LOG_TRACE("Successfully read all inputs, processing...");

        // Segmentation and coverage.
        TrackHistory::TrackLog avgPx = avg;

        // Setup floor-coverage system...
        CoverageSystem coverage( cvSize( compImg->width, compImg->height ) );

        if ( floorMaskFile )
        {
            coverage.LoadFloorMask( floorMaskFile );
        }

        if ( coverage.GetFloorMask() )
        {
            // Create a fake tracker - only to compute brush bar dimensions
            LOG_INFO("TRACKER WARNINGS WHICH FOLLOW CAN BE SAFELY IGNORED.");

            const RobotTracker* tracker = new KltTracker( 0, &metrics, 0, 0 );

            float travelledDistance = 0.f;
            double lastIncTime = 0.0;

            LOG_TRACE("Computing coverage...");

            for ( unsigned int p=1; p<avgPx.size(); ++p )
            {
                CvPoint2D32f prev = avgPx[p-1].GetPosition();
                prev.x += .5f-tx;
                prev.y += .5f-ty;

                CvPoint2D32f curr = avgPx[p].GetPosition();
                curr.x += .5f-tx;
                curr.y += .5f-ty;

                float dx = curr.x - prev.x;
                float dy = curr.y - prev.y;
                travelledDistance += sqrtf( dx*dx + dy*dy );

                // Time in seconds at this point...
                double dt = avgPx[p].GetTimeStamp() -
                            avgPx[p-1].GetTimeStamp();

                 // Pairs must be close enough that there
                 // was no loss of tracking between them.
                if ( std::abs(dt) < 0.75 )
                {
                    // Compute brush bar position in the previous and current frames
                    float heading = avgPx[p-1].GetOrientation();
                    CvPoint2D32f pl = tracker->GetBrushBarLeft( prev, heading );
                    CvPoint2D32f pr = tracker->GetBrushBarRight( prev, heading );

                    heading = avgPx[p].GetOrientation();
                    CvPoint2D32f cl = tracker->GetBrushBarLeft( curr, heading );
                    CvPoint2D32f cr = tracker->GetBrushBarRight( curr, heading );

                    coverage.BrushBarUpdate( pl, pr, cl, cr );

                    cvLine( headingImg,
                            cvPoint( ( int )cl.x, ( int )cl.y ),
                            cvPoint( ( int )cr.x, ( int )cr.y ),
                            cvScalar( 0, 0, 255 ),
                            3,
                            CV_AA );

                    // Do coverage updates for any sweepers...
                    for ( unsigned int i=0 ;i<sweepers.size(); ++i )
                    {
                        heading = avgPx[p-1].GetOrientation();
                        pl = sweepers[i].GetLeft( metrics, prev, heading );
                        pr = sweepers[i].GetRight( metrics, prev, heading );

                        heading = avgPx[p].GetOrientation();
                        cl = sweepers[i].GetLeft( metrics, curr, heading );
                        cr = sweepers[i].GetRight( metrics, curr, heading );

                        coverage.BrushBarUpdate( pl, pr, cl, cr );
                    }
                }

                if ( incCoverageFile )
                {
                    // Only write incremental time if enough
                    // time has elasped since the last update
                    double tDelta = avgPx[p].GetTimeStamp() - lastIncTime;

                    if ( tDelta >= incTimeStep )
                    {
                        fprintf( incCoverageFile, "%f ", avgPx[p].GetTimeStamp() );
                        coverage.WriteIncrementalCoverage( incCoverageFile, 20 );

                        lastIncTime = avgPx[p].GetTimeStamp();
                    }
                }
            }

            LOG_TRACE("Done.");

            LOG_INFO(QObject::tr("Total distance travelled := %1(cm)\n")
                         .arg(travelledDistance / metrics.GetScaleFactor()));

            int missCount = coverage.MissedMask( coverageMissedFile );

            LOG_INFO(QObject::tr("Missed pixels: %1.").arg(missCount));

            // Create a coloured map to
            // indicate repeated coverage.
            cvConvertImage( compImg, compImgCol );
            coverage.CreateColouredMap();
            coverage.DrawMap( compImgCol );

            cvSaveImage( coverageColourFile, compImgCol );
            cvSaveImage( headingFile, headingImg );

            coverage.CoverageHistogram( coverageHistogramFile );

            // Save the raw coverage-mask as a file, where
            // each pixel value represents the number of
            // passes made over that location in this run...
#if 0
            char maskName[MAX_PATH] = "";
            sprintf( maskName, coverageRawFile, time(0) );
#endif
            coverage.SaveMask( coverageRawFile );

            // clean up
            if ( incCoverageFile )
            {
                fclose( incCoverageFile );
            }

            LOG_TRACE("Coverage data written - cleaning up...");

            delete tracker;
        }
        else
        {
            LOG_WARN("No coverage results computed!");
        }

        cvReleaseImage( &compImg );
        cvReleaseImage( &headingImg );
        cvReleaseImage( &compImgCol );

        PlotTrackLog( avg, floorPlanFile, trackerResultsImgFile );

        LOG_TRACE("Finished.");
    }

    return exitStatus;
}

// ----------------------------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------------------

void PostProcessToolWidget::PlotTrackLog( TrackHistory::TrackLog& log,
                                          char*                   floorPlanFile,
                                          char*                   trackerResultsImgFile )
{
    // Seconds - prob not discts within half sec. using
    const float timeThresh = 0.5f; // 2.0f / (float)m_fps;

    IplImage* compImg = 0;
    IplImage* compImgCol = 0;

    CvScalar colours[4] = { CV_RGB( 0,   0,   255 ),
                            CV_RGB( 0,   255, 0   ),
                            CV_RGB( 255, 255, 0   ),
                            CV_RGB( 0,   255, 255 ) };

    IplImage* baseImg = cvLoadImage(floorPlanFile, CV_LOAD_IMAGE_GRAYSCALE);

    compImg = cvCloneImage( baseImg );

    // Plot logs - colour copy of composite image
    compImgCol = cvCreateImage( cvSize( compImg->width,
                                        compImg->height ), compImg->depth, 3 );
    cvConvertImage( compImg, compImgCol );

    ScanUtility::PlotLog( log,
                          compImgCol,
                          colours[0],
                          cvRect( 0, 0, 0, 0 ),
                          0,
                          1,
                          timeThresh );

    // Write composite image to file...
    cvSaveImage( trackerResultsImgFile, compImgCol );

    // Clean up...
    cvReleaseImage( &compImg );
    cvReleaseImage( &compImgCol );
}
