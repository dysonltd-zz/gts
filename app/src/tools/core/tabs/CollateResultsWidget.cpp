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

#include "CollateResultsWidget.h"

#include "ui_CollateResultsWidget.h"

#include "Collection.h"
#include "RunsCollection.h"
#include "RoomsCollection.h"
#include "RoomLayoutSchema.h"
#include "RunSchema.h"

#include "WbConfigTools.h"
#include "WbDefaultKeys.h"

#include "OpenCvTools.h"

#include "FileUtilities.h"
#include "FileDialogs.h"
#include "Message.h"
#include "UnknownLengthProgressDlg.h"
#include "Logging.h"

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include <QtCore/QDir>
#include <QtGui/QMessageBox>
#include <QFileDialog>
#include <QApplication>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QTemporaryFile>
#include <QDirIterator>
#include <QTextStream>

#include <fstream>

#ifdef _WIN32
#include "windows.h"
#else
#define MAX_PATH 255
#endif

CollateResultsWidget::CollateResultsWidget( QWidget* parent ) :
    Tool( parent, CreateSchema() ),
    m_ui( new Ui::CollateResultsWidget )
{
    m_ui->setupUi( this );

    QObject::connect( m_ui->m_loadBtn,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( LoadRunsButtonClicked() ) );
    QObject::connect( m_ui->m_analyseBtn,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( AnalyseResultsButtonClicked() ) );
    QObject::connect( m_ui->m_selectAllCheckBox,
                      SIGNAL( stateChanged(int) ),
                      this,
                      SLOT( SelectAllCheckBoxChecked(int) ) );
}

CollateResultsWidget::~CollateResultsWidget()
{
    delete m_ui;
}

const QString CollateResultsWidget::GetSubSchemaDefaultFileName() const
{
    return "collateResults.xml";
}

bool CollateResultsWidget::CanClose() const
{
    return true;
}

const QString CollateResultsWidget::CannotCloseReason() const
{
    return tr("Please complete all highlighted boxes before leaving this tab.");
}

const WbSchema CollateResultsWidget::CreateSchema()
{
    WbSchema schema( CreateWorkbenchSubSchema( KeyName( "collatedCoverageResults" ),
                                               tr( "Collated Coverage" ) ) );

    schema.AddSingleValueKey( KeyName( "room" ),
                              WbSchemaElement::Multiplicity::One );
    return schema;
}

const KeyId CollateResultsWidget::GetRoomIdToCollate() const
{
    const WbConfig collateResultsConfig( GetCurrentConfig() );
    const WbConfig analysisResultsConfig( collateResultsConfig.GetParent() );
    const KeyId roomIdToCollate( analysisResultsConfig.GetKeyValue( RunSchema::roomIdKey ).ToKeyId() );

    return roomIdToCollate;
}

#define TABLE_COL_USE  0
#define TABLE_COL_RUN  1
#define TABLE_COL_ROOM 2

void CollateResultsWidget::LoadRunsButtonClicked()
{
    const KeyId roomIdToCollate = GetRoomIdToCollate();

    tableModel = new QStandardItemModel();
    tableModel->setHorizontalHeaderItem(TABLE_COL_USE, new QStandardItem(QString("")));
    tableModel->setHorizontalHeaderItem(TABLE_COL_RUN, new QStandardItem(QString("Run")));
    tableModel->setHorizontalHeaderItem(TABLE_COL_ROOM, new QStandardItem(QString("Room")));

    Collection runsCollection = RunsCollection();
    Collection roomsCollection = RoomsCollection();
    runsCollection.SetConfig( GetCurrentConfig() );
    roomsCollection.SetConfig( GetCurrentConfig() );

    for (int n = 0; n < (int)runsCollection.NumElements(); ++n)
    {
        const WbConfig runConfig = runsCollection.ElementAt( n ).value;
        const KeyId roomId = runConfig.GetKeyValue( RunSchema::roomIdKey ).ToKeyId();
        if (roomId != roomIdToCollate)
        {
            break;
        }
        const WbConfig roomConfig = roomsCollection.ElementById( roomId );
        const QString runName = runConfig.GetKeyValue( WbDefaultKeys::displayNameKey ).ToQString();
        const QString roomName = roomConfig.GetKeyValue( WbDefaultKeys::displayNameKey ).ToQString();

        // Create check box item
        QStandardItem* item = new QStandardItem(true);
        item->setCheckable(true);
        item->setCheckState(Qt::Unchecked);
        tableModel->setItem(n, TABLE_COL_USE, item);

        // Create text item(s)
        tableModel->setItem(n, TABLE_COL_RUN, new QStandardItem(runName));
        tableModel->setItem(n, TABLE_COL_ROOM, new QStandardItem(roomName));

    }

    // resize header column
    m_ui->m_runsTable->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);

    // Set model
    m_ui->m_runsTable->setModel( tableModel );
    m_ui->m_analyseBtn->setEnabled(true);
    m_ui->m_selectAllCheckBox->setEnabled(true);
    m_ui->m_passCapSpinBox->setEnabled(true);

    if ( m_ui->m_selectAllCheckBox->isChecked() )
    {
        SelectAllCheckBoxChecked( Qt::Checked );
    }
}

void CollateResultsWidget::AnalyseResultsButtonClicked()
{
    LOG_TRACE("Analysing results");

    bool successful = true;
    const WbConfig& config = GetCurrentConfig();
    const KeyId roomIdToCollate = GetRoomIdToCollate();

    Collection rooms = RoomsCollection();
    rooms.SetConfig( config );

    Collection runsCollection = RunsCollection();
    runsCollection.SetConfig( config );

    const WbConfig roomConfig = rooms.ElementById( roomIdToCollate );
    const WbConfig roomLayoutConfig( roomConfig.GetSubConfig( RoomLayoutSchema::schemaName ) );

    const QString floorPlanName( roomLayoutConfig.GetAbsoluteFileNameFor( "floor_plan.png" ) );
    const QString floorMaskName( roomLayoutConfig.GetAbsoluteFileNameFor( "floor_mask.png" ) );
    const QString totalCoverageCsvName( config.GetAbsoluteFileNameFor( "results/total_coverage.csv" ) );
    const QString totalCoverageImgName( config.GetAbsoluteFileNameFor( "results/total_coverage.png" ) );

    QTemporaryFile tmpFile( QDir::tempPath() + "/files.txt");
    if (tmpFile.open())
    {
        QTextStream fileStrm( &tmpFile );
        for (int n = 0; n < tableModel->rowCount(); ++n)
        {
            QStandardItem* item = tableModel->item(n, TABLE_COL_USE);

            if ( item->checkState() == Qt::Checked )
            {
                QFileInfo runDir = runsCollection.ElementAt( n ).value.GetAbsoluteFileInfo();
                QDirIterator dirIterator(runDir.absolutePath(),
                                         QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot,
                                         QDirIterator::Subdirectories);

                while (dirIterator.hasNext())
                {
                    dirIterator.next();

                    if (dirIterator.fileInfo().isDir())
                    {
                        foreach( QFileInfo fileInfo, QDir(dirIterator.filePath()).entryInfoList( QStringList("coverage_overlay.png"),
                                                                                                 QDir::Files | QDir::NoDotAndDotDot ) )
                        {
                            fileStrm << fileInfo.filePath() << "\n";
                        }
                    }
                }
            }
        }

        tmpFile.close();
    }
    else
    {
        LOG_ERROR("Coverage Collation - failed to create temporary directory.");
        QMessageBox::critical(this,
                              tr( "Coverage Collation Failed" ),
                              tr( "Temporary file missing." ));
        return;
    }

    if ( !CreateAnalysisResultDirectory( config ) )
    {
        Message::Show( this,
                       tr( "Coverage Collation Successful" ),
                       tr( "Failed to create results directory." ),
                       Message::Severity_Critical );

        return;
    }

    // Run data analysis
    UnknownLengthProgressDlg* const progressDialog = new UnknownLengthProgressDlg( this );
    progressDialog->Start( tr( "Processing" ), tr( "" ) );

    ExitStatus::Flags exitCode = CollateCoverageResults( floorPlanName.toAscii().data(),        // floorPlanName
                                                 floorMaskName.toAscii().data(),        // floorMaskName
                                                 tmpFile.fileName().toAscii().data(),   // inputFileList
                                                 totalCoverageCsvName.toAscii().data(),
                                                 totalCoverageImgName.toAscii().data() );

    IplImage* img = cvLoadImage(totalCoverageImgName.toAscii(), CV_LOAD_IMAGE_COLOR);

    if( img )
    {
        ShowImage(m_ui->m_analysisImage, img);
        cvReleaseImage( &img );
    }
    else
    {
        successful = false;
        LOG_ERROR("Could not load collated coverage image to display on screen");
        Message::Show( this,
                       tr( "Coverage Collation Failed" ),
                       tr( "Could not load collated coverage image to display on screen"),
                       Message::Severity_Critical );
    }

    successful = ( exitCode == ExitStatus::OK_TO_CONTINUE );

    if ( successful )
    {
        const QString analysisResultsDirectory( config.GetAbsoluteFileNameFor( "results" ) );
        progressDialog->Complete( tr( "Coverage Collation Successful" ),
                                  tr( "Results located at: %1")
                                  .arg( analysisResultsDirectory ),
                                  analysisResultsDirectory );

    }
    else
    {
        LOG_ERROR("Could not complete coverage collation");
        progressDialog->ForceClose();
        Message::Show( this,
                       tr( "Coverage Collation Failed" ),
                       tr( "Coverage collation failed, please check log file." ),
                       Message::Severity_Critical );
    }
}

void CollateResultsWidget::ShowImage(ImageView* view, const IplImage* image)
{
    // Convert image
    IplImage* imgTmp = cvCreateImage( cvSize( image->width, image->height ), IPL_DEPTH_8U, 3 );
    cvConvertImage( image, imgTmp, CV_CVTIMG_SWAP_RB);

    const QSize imgSize( imgTmp->width, imgTmp->height );
    QImage qImg = QImage( imgSize, QImage::Format_RGB888 );

    CvMat mtxWrapper;
    cvInitMatHeader( &mtxWrapper,
                     imgTmp->height,
                     imgTmp->width,
                     CV_8UC3,
                     qImg.bits() );

    cvConvertImage( imgTmp, &mtxWrapper, 0 );

    // Display image
    view->Clear();
    view->SetImage( qImg );
    view->update();

    cvReleaseImage( &imgTmp );
}

void CollateResultsWidget::SelectAllCheckBoxChecked(int state)
{
    for (int n = 0; n < tableModel->rowCount(); ++n)
    {
        QStandardItem* item = tableModel->item(n, TABLE_COL_USE);
        if ( state == Qt::Checked )
        {
            item->setCheckState( Qt::Checked );
        }
        else
        {
            item->setCheckState( Qt::Unchecked);
        }
    }
}

const ExitStatus::Flags CollateResultsWidget::CollateCoverageResults( char* floorPlanName,
                                                                      char* floorMaskName,         // -flr
                                                                      char* overlayListFileName,   // -files
                                                                      char* totalCoverageCsvName,
                                                                      char* totalCoverageImgName ) // -out
{
    ExitStatus::Flags exitStatus = ExitStatus::OK_TO_CONTINUE;

    IplImage* floorMaskImg = NULL;
    if ( floorMaskName )
    {
        floorMaskImg = OpenCvTools::LoadSingleChannelImage( floorMaskName );
    }

    if ( !floorMaskImg )
    {
        LOG_ERROR("Could not load floor mask image");
        Message::Show( this,
                       QObject::tr("Coverage Collation Failed" ),
                       QObject::tr("Could not load floor mask image."
                                   "\nPlease generate a floor mask from the 'Rooms' tab"),
                       Message::Severity_Critical );
        return ExitStatus::ERRORS_OCCURRED;
    }

    IplImage* floorPlanImg = NULL;
    if ( floorPlanName )
    {
        floorPlanImg = cvLoadImage( floorPlanName );
    }

    if ( !floorPlanImg )
    {
        LOG_ERROR("Could not load floor plan image");
        Message::Show( this,
                       QObject::tr("Coverage Collation Failed" ),
                       QObject::tr("Could not load floor plan image."
                                   "\nPlease generate a floor plan from the 'Rooms' tab"),
                       Message::Severity_Critical );

        cvReleaseImage( &floorMaskImg ); // release floor mask image as it won't be used now

        return ExitStatus::ERRORS_OCCURRED;
    }
    else if ((floorPlanImg->width != floorMaskImg->width) ||
             (floorPlanImg->height != floorMaskImg->height))
    {
        LOG_ERROR("Coverage Collation - Floor plan and mask sizes differ!");
        Message::Show( this,
                       QObject::tr("Coverage Collation Failed" ),
                       QObject::tr("Floor plan and floor mask sizes differ."
                                   "\nPlease check sizes."),
                       Message::Severity_Critical );

        // release floor mask and plan image as they won't be used
        cvReleaseImage( &floorMaskImg );
        cvReleaseImage( &floorPlanImg );

        return ExitStatus::ERRORS_OCCURRED;
    }

    std::vector<std::string> fileNames;
    FILE* f = fopen( overlayListFileName, "r" );
    while (f && !feof(f))
    {
        char fileName[MAX_PATH];
        if ( fgets( fileName, sizeof(fileName), f ) )
        {
            // Strip trailing newline
            const size_t len = strlen( fileName );
            fileName[len-1] = '\0';
            fileNames.push_back(fileName);
        }
    }

    if ( fileNames.size() == 0 )
    {
        LOG_WARN("No coverage images supplied!");
        Message::Show( this,
                       QObject::tr( "Coverage Collation Failed" ),
                       QObject::tr("No coverage image(s) generated from run(s). Please re-process runs and try again."),
                       Message::Severity_Critical );

        // release floor mask and plan image as they won't be used
        cvReleaseImage( &floorMaskImg );
        cvReleaseImage( &floorPlanImg );

        return ExitStatus::ERRORS_OCCURRED;
    }

    // Number of non-zero pixels in floor mask is total number of
    // pixels of interest - the size of the area we are looking at.
    const int nTotalPixels = floorMaskImg->width * floorMaskImg->height;
    const int nFloorPixels = cvCountNonZero( floorMaskImg );

    LOG_INFO(QObject::tr("Total pixels (WxH) = %1.").arg(nTotalPixels));
    LOG_INFO(QObject::tr("Total floor pixels = %2.").arg(nFloorPixels));

    // Overlay floor mask onto floor image
    OpenCvTools::DrawColouredOverlay( floorPlanImg,
                                      floorMaskImg,
                                      CV_RGB(100,0,0),
                                      std::bind2nd(std::equal_to<int>(), 255) );

    // Keep track of total coverage counts in a separate map
    IplImage* totalCoverageImg = cvCreateImage( cvSize( floorMaskImg->width,
                                                        floorMaskImg->height), IPL_DEPTH_8U, 1 );
    cvZero( totalCoverageImg );

    FILE* fp = fopen( totalCoverageCsvName, "w" );

    PrintCsvHeaders(fp, m_ui->m_passCapSpinBox->value());

    int run = 0;
    for (std::vector<std::string>::const_iterator i = fileNames.begin(); i != fileNames.end(); ++i)
    {
        LOG_INFO(QObject::tr("Run: %1 (file: %2).").arg(++run).arg(i->c_str()));

        // Load the coverage mask, limit it to the floor mask area
        IplImage* coverageMaskImg = OpenCvTools::LoadSingleChannelImage( i->c_str() );

        if ((coverageMaskImg->height != floorMaskImg->height) ||
            (coverageMaskImg->width != floorMaskImg->width))
        {
            LOG_ERROR(QObject::tr("Coverage image (%1) and floor mask sizes differ!").arg(i->c_str()));
            Message::Show( this,
                           QObject::tr("Coverage Collation Failed" ),
                           QObject::tr("Coverage image (%1) and floor mask sizes differ."
                                       "\nPlease check sizes of floor plan and mask and then reload.").arg(i->c_str()),
                           Message::Severity_Critical );
            cvReleaseImage( &coverageMaskImg );
            continue;
        }

        // Constrain coverage to the floor mask area
        cvAnd( coverageMaskImg, floorMaskImg, coverageMaskImg );

        // Add this coverage to the total
        cvAdd( coverageMaskImg, totalCoverageImg, totalCoverageImg );

        // Write to CSV the collated coverage results
        PrintCsvLineForPass(fp, run, totalCoverageImg, nFloorPixels, m_ui->m_passCapSpinBox->value());

        // Clean up
        cvReleaseImage( &coverageMaskImg );
    }


    // Update floor plan image with total coverage.
    for (int level = 1; level < m_ui->m_passCapSpinBox->value(); ++level)
    {
        OpenCvTools::DrawColouredOverlay( floorPlanImg,
                                          totalCoverageImg,
                                          CV_RGB(0,level*40,0),
                                          std::bind2nd(std::equal_to<int>(), level) );
    }


    OpenCvTools::DrawColouredOverlay( floorPlanImg,
                                      totalCoverageImg,
                                      CV_RGB(0,255,0),
                                      std::bind2nd(std::greater_equal<int>(), 10) );

    // Clean up
    if ( f ) { fclose( f ); }
    if ( fp ) { fclose( fp ); }
    cvSaveImage( totalCoverageImgName, floorPlanImg );
    cvReleaseImage( &floorMaskImg );
    cvReleaseImage( &floorPlanImg );

    LOG_TRACE("Collating coverage results complete");

    return exitStatus;
}

bool CollateResultsWidget::CreateAnalysisResultDirectory(const WbConfig& config)
{
    QDir m_resultDir( config.GetAbsoluteFileNameFor( "results" ) );
    const QString resultDirName = m_resultDir.dirName();
    QDir resultDirParent( m_resultDir );

    if ( !resultDirParent.cdUp() )
    {
        Message::Show( this,
                       tr( "Coverage Collation Failed" ),
                       tr( "Please save your workbench before continuing." ),
                       Message::Severity_Critical );
        return false;
    }

    if (resultDirParent.exists( resultDirName ))
    {

        const int result = QMessageBox::question( this,
                                                  "Confirm delete",
                                                  QObject::tr("Are you sure you want to overwrite already saved data?"),
                                                  QMessageBox::Yes|QMessageBox::No );

        if (result == QMessageBox::No)
        {
            return false;
        }
        else
        {
            FileUtilities::DeleteDirectory( resultDirParent.absoluteFilePath(resultDirName) );
        }
    }

    if ( !resultDirParent.mkdir( resultDirName ) || !resultDirParent.cd( resultDirName ))
    {
        Message::Show( this,
                       tr( "Coverage Collation Failed" ),
                       tr( "Analysis results folder cannot be found or is currently open. Please close and try again." ),
                       Message::Severity_Critical );
        return false;
    }

    return true;
}

void CollateResultsWidget::PrintCsvHeaders( FILE* fp, const int passCap )
{
    fprintf(fp, "Run, ");

    for (int i = 0; i < passCap; ++i)
    {
        int numberOfPasses = i + 1;
        fprintf(fp, "%d Pass", numberOfPasses);

        if (numberOfPasses > 1)
        {
            fprintf(fp, "es");
        }
        if (i < passCap-1)
        {
            fprintf(fp, ", ");
        }
        else
        {
            fprintf(fp, "+\n");
        }
    }
}

void CollateResultsWidget::PrintCsvLineForPass( FILE* fp,
                          const int run,
                          IplImage* totalCoverageImg,
                          const int nFloorPixels,
                          const int passCap)
{
    fprintf( fp, "%d", run);

    for (int level = 0; level < passCap; ++level)
    {
        const int numTimesCovered = level+1;
        const bool isTopNumPasses = (level == (passCap-1));
        int comparisonOp = CV_CMP_EQ;

        if ( isTopNumPasses )
        {
            comparisonOp = CV_CMP_GE;
        }

        int nPixels = OpenCvTools::GetPixelCoverageCount( totalCoverageImg,
                                                          numTimesCovered,
                                                          comparisonOp );
        float percent = nPixels * (100.f / nFloorPixels);
        fprintf(fp, ", %f", percent);

        LOG_INFO(QObject::tr("%1 pixels (%2%) were covered %3 time(s).").arg(nPixels)
                                                                         .arg(percent)
                                                                         .arg(numTimesCovered));
    }

    fprintf(fp, "\n");
}
