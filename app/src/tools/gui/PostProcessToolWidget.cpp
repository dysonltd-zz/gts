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
                      SIGNAL( currentChanged ( const QModelIndex&,
                                               const QModelIndex& ) ),
                      m_ui->m_trackView,
                      SLOT( currentChanged( const QModelIndex&,
                                            const QModelIndex& ) ) );

    QObject::connect( m_resultsModel,
                      SIGNAL( dataChanged ( const QModelIndex&,
                                            const QModelIndex& ) ),
                      m_ui->m_trackView,
                      SLOT( dataChanged ( const QModelIndex&,
                                          const QModelIndex& ) ) );

    QAction* const toggleAction = new QAction( tr( "Toggle" ), m_ui->m_trackResults );
    toggleAction->setShortcut( QKeySequence( QKeySequence::Delete ) );
    toggleAction->setShortcutContext( Qt::WidgetShortcut );
    connect( toggleAction,
             SIGNAL( triggered() ),
             this,
             SLOT( ToggleItemTriggered() ) );

    m_ui->m_trackResults->addAction( toggleAction );
    m_ui->m_trackResults->setContextMenuPolicy( Qt::ActionsContextMenu );

    m_ui->m_trackResults->show();
    m_ui->m_trackView->show();
}

void PostProcessToolWidget::ToggleItemTriggered()
{
    QModelIndexList list = m_selectionModel->selectedRows();
    foreach (QModelIndex index, list)
    {
        bool value = m_resultsModel->data(m_resultsModel->index(index.row(), 0), Qt::UserRole).toBool();
        m_resultsModel->setData(index, QVariant(!value), Qt::UserRole);
    }
}

void PostProcessToolWidget::PostProcessButtonClicked()
{
    const WbConfig& config = GetCurrentConfig();

    LOG_TRACE("Post Process...");

    const QString fileName(
         config.GetAbsoluteFileNameFor( "results/track_result_out.csv" ) );

    m_resultsModel->toCSV(fileName, true, ',');

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
                    runConfig.GetAbsoluteFileNameFor( "results/track_result_raw.txt" ) );
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

        if ( successful )
        {
            UnknownLengthProgressDlg* const progressDialog = new UnknownLengthProgressDlg( this );
            progressDialog->Start( tr( "Working..." ), tr( "" ) );

            ExitStatus::Flags exitCode = PostProcess( config,
                                                      trackerResultsName.toAscii().data(),       // trackerResultsName
                                                      coverageIncrementName.toAscii().data(),    // coverageFile
                                                      floorPlanName.toAscii().data(),            // floorPlanFile
                                                      floorMaskName.toAscii().data(),            // floorMaskFile
//                                                    ex.GetRobotMetricsFile().toAscii().data(), // robotMetricsFile
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
                progressDialog->Complete( tr( "Processing Successful" ),
                                          tr( "Results have been computed.\n" ) );
            }
            else
            {
                progressDialog->ForceClose();

                Message::Show( 0,
                               tr( "Post-Processing" ),
                               tr( "Error - Please see log for details!" ),
                               Message::Severity_Critical );
            }
        }
    }
}

// ----------------------------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------------------

const ExitStatus::Flags PostProcessToolWidget::PostProcess( const WbConfig& postProcConfig,
                                                            char*           trackerResultsFile,    // -log
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

            LOG_INFO(QObject::tr("Read sweeper (%1) data from '%2'.").arg(sweepers.size())
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
        TrackHistory avg;
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

        if ( !metrics.LoadMetrics( metricsConfig, firstCamPosCalConfig, trackConfig ) )
        {
            LOG_ERROR("Could not load robot metrics!");

            return ExitStatus::ERRORS_OCCURRED;
        }

        if ( !ReadHistoryLog( trackerResultsFile, avg ) )
        {
            LOG_ERROR(QObject::tr("Could not load track log from '%s'!").arg(trackerResultsFile));

            return ExitStatus::ERRORS_OCCURRED;
        }

        LogSwapHandedness( avg );

        if ( relativeLogFile )
        {
            TrackHistory rel;
            ConvertToRelativeLog( avg, rel );
            WriteHistoryLog( relativeLogFile, rel);
        }

        IplImage* compImgCol = cvLoadImage( floorPlanFile );
        IplImage* compImg = cvCreateImage( cvSize( compImgCol->width,
                                                   compImgCol->height), IPL_DEPTH_8U, 1 );
        cvConvertImage( compImgCol, compImg ); // colour copy of composite image

        IplImage* headingImg = cvCreateImage( cvSize( compImgCol->width,
                                                      compImgCol->height), IPL_DEPTH_8U, 1 );
        cvConvertImage( compImgCol, headingImg ); // colour copy of composite image

        LOG_TRACE("Successfully read all inputs, processing...");

        // Segmentation and coverage
        TrackHistory avgPx = avg;

        // Setup floor-coverage system
        CoverageSystem coverage( cvSize( compImg->width, compImg->height ) );

        if ( floorMaskFile )
        {
            coverage.LoadFloorMask( floorMaskFile );
        }
        else
        {
            assert( false );
        }

        if ( coverage.GetFloorMask() )
        {
            // Create a fake tracker - only to compute brushbar dimensions
            LOG_INFO("TRACKER WARNINGS WHICH FOLLOW CAN BE SAFELY IGNORED.");

            const RobotTracker* tracker = new KltTracker( 0, &metrics, 0, 0, 0 );

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

                double dt = avgPx[p].GetTimeStamp() -
                            avgPx[p-1].GetTimeStamp(); // in seconds at this point

                if ( std::abs(dt) < 0.75 ) // Pairs must be close enough that there was no track loss between them
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

                    // Do coverage updates for any sweepers:
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
                    // only write incremental time if enough enough time has elasped since the last update
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

            LOG_INFO(QObject::tr("Total distance travelled := %1(cm)\n").arg(travelledDistance / metrics.GetScaleFactor()));

            int missCount = coverage.MissedMask( coverageMissedFile );

            LOG_INFO(QObject::tr("Missed pixels: %d.").arg(missCount));

            // Create a coloured map indicating repeated coverage:
            cvConvertImage( compImg, compImgCol ); // reset colour copy of composite image
            coverage.CreateColouredMap();
            coverage.DrawMap( compImgCol );

            cvSaveImage( coverageColourFile, compImgCol );
            cvSaveImage( headingFile, headingImg );

            coverage.CoverageHistogram( coverageHistogramFile );

            // save the raw coverage-mask as a file, where each pixel
            // value represents the number of passes made over that
            // location in this run...
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

        cvReleaseImage( &compImg );
        cvReleaseImage( &headingImg );
        cvReleaseImage( &compImgCol );

        LOG_TRACE("Finished.");
    }

    return exitStatus;
}
