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

#include "TrackView.h"

#include "TrackModel.h"

#include "RobotMetrics.h"
#include "RobotTracker.h"

#include "ScanUtility.h"
#include "GroundPlaneUtility.h"

#include "WbConfig.h"
#include "WbConfigTools.h"
#include "WbDefaultKeys.h"

#include "Collection.h"
#include "CamerasCollection.h"
#include "RoomsCollection.h"
#include "RunsCollection.h"
#include "RobotsCollection.h"
#include "CameraPositionsCollection.h"

#include "RoomLayoutSchema.h"
#include "CameraPositionSchema.h"
#include "RobotMetricsSchema.h"
#include "CalibrationSchema.h"
#include "ExtrinsicCalibrationSchema.h"
#include "RunSchema.h"
#include "TrackRobotSchema.h"
#include "VideoCaptureSchema.h"
#include "FloorPlanSchema.h"
#include "PostProcessSchema.h"

#include <opencv/highgui.h>

#include <QGridLayout>
#include <QtGlobal>

TrackView::TrackView( QWidget *parent )
{
    Q_UNUSED(parent);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(&m_imageView);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);
}

IplImage* TrackView::loadFloorPlan( const WbConfig& runConfig )
{
    Collection m_rooms = RoomsCollection();
    m_rooms.SetConfig( runConfig );

    const KeyValue roomId = runConfig.GetKeyValue( RunSchema::roomIdKey );
    const WbConfig roomConfig = m_rooms.ElementById( roomId.ToKeyId() );

    const WbConfig roomLayoutConfig(
                roomConfig.GetSubConfig( RoomLayoutSchema::schemaName ) );

    const QString floorPlanName( roomLayoutConfig.GetAbsoluteFileNameFor( "floor_plan.png" ) );

    m_baseImg = cvLoadImage(floorPlanName.toAscii().data(), CV_LOAD_IMAGE_GRAYSCALE);

    return m_baseImg;
}

bool TrackView::loadMetrics( const WbConfig& runConfig )
{
    WbConfig trackConfig = runConfig.GetSubConfig( TrackRobotSchema::schemaName );

    Collection m_rooms = RoomsCollection();
    m_rooms.SetConfig( runConfig );

    const KeyValue roomId = runConfig.GetKeyValue( RunSchema::roomIdKey );
    const WbConfig roomConfig = m_rooms.ElementById( roomId.ToKeyId() );

    std::vector<WbConfig> camPosConfigs = GetCameraPositionsConfigs( roomConfig );
    const WbConfig firstCamPosConfigForThisRun( camPosConfigs.at(0) );

    WbConfig firstCamPosCalibrationCfg;
    firstCamPosCalibrationCfg = firstCamPosConfigForThisRun.GetSubConfig( ExtrinsicCalibrationSchema::schemaName );
    const KeyValue squareSizeCmKey = firstCamPosCalibrationCfg.GetKeyValue( ExtrinsicCalibrationSchema::gridSquareSizeInCmKey );
    const KeyValue squareSizePxKey = firstCamPosCalibrationCfg.GetKeyValue( ExtrinsicCalibrationSchema::gridSquareSizeInPxKey );

    const KeyValue resolutionKey = trackConfig.GetKeyValue( TrackRobotSchema::GlobalTrackingParams::resolution );

    m_metricsScaleFactor = 1.0;

    return true;
}

void TrackView::setModel(QAbstractItemModel *model)
{
    m_model = model;

    updateView();
}

void TrackView::rowsRemoved ( )
{
    updateView();
}

void TrackView::dataChanged ( const QModelIndex& topLeft,
                              const QModelIndex& bottomRight )
{
    Q_UNUSED(topLeft);
    Q_UNUSED(bottomRight);

    updateView();
}

void TrackView::selectionChanged ( const QItemSelection& selected,
                                   const QItemSelection& deselected )
{
    QModelIndexList selectedList = selected.indexes();

    foreach (QModelIndex idx, selectedList)
    {
        m_model->setData(idx, QVariant(true), TrackModel::IS_SELECTED);
    }

    QModelIndexList deselectedList = deselected.indexes();

    foreach (QModelIndex idx, deselectedList)
    {
        m_model->setData(idx, QVariant(false), TrackModel::IS_SELECTED);
    }

    updateView();
}

void TrackView::updateView()
{
    IplImage* compImg = 0;
    IplImage* compImgCol = 0;

    CvScalar colours[4] = { CV_RGB( 0,   0,   255 ),
                            CV_RGB( 0,   255, 0   ),
                            CV_RGB( 255, 255, 0   ),
                            CV_RGB( 0,   255, 255 ) };

    compImg = cvCloneImage( m_baseImg );

    //// Plot logs
    compImgCol = cvCreateImage( cvSize( compImg->width, compImg->height ), compImg->depth, 3 );
    cvConvertImage( compImg, compImgCol ); // colour copy of composite image

    float timeThresh = 0.5f;

    TrackHistory::TrackLog avg;

    for (int row = 0; row < m_model->rowCount(); ++row)
    {
        if (m_model->data(m_model->index(row, 0), TrackModel::IS_DELETED).toBool())
        {
            avg.push_back( TrackEntry( cvPoint2D32f( m_model->data(m_model->index(row, TrackModel::COLUMN_POSX)).toFloat(),
                                                     m_model->data(m_model->index(row, TrackModel::COLUMN_POSY)).toFloat() * -1.0),
                                                     m_model->data(m_model->index(row, TrackModel::COLUMN_HEADING)).toFloat(),
                                                     m_model->data(m_model->index(row, TrackModel::COLUMN_ERROR)).toFloat(),
                                                     m_model->data(m_model->index(row, TrackModel::COLUMN_TIME)).toDouble(),
                                                     m_model->data(m_model->index(row, TrackModel::COLUMN_WGM)).toFloat() ) );
        }
    }

    ScanUtility::PlotLog( avg,
                          compImgCol,
                          colours[0],
                          cvRect( 0, 0, 0, 0 ),
                          0,
                          1,
                          timeThresh );

    for (int row = 0; row < m_model->rowCount(); ++row)
    {
        if (m_model->data(m_model->index(row, 0), TrackModel::IS_SELECTED).toBool())
        {
            TrackEntry pt( cvPoint2D32f( m_model->data(m_model->index(row, TrackModel::COLUMN_POSX)).toFloat(),
                                         m_model->data(m_model->index(row, TrackModel::COLUMN_POSY)).toFloat() * -1.0),
                                         m_model->data(m_model->index(row, TrackModel::COLUMN_HEADING)).toFloat(),
                                         m_model->data(m_model->index(row, TrackModel::COLUMN_ERROR)).toFloat(),
                                         m_model->data(m_model->index(row, TrackModel::COLUMN_TIME)).toDouble(),
                                         m_model->data(m_model->index(row, TrackModel::COLUMN_WGM)).toFloat() );
            ScanUtility::PlotPoint( pt,
                                    compImgCol,
                                    colours[2],
                                    cvRect( 0, 0, 0, 0 ),
                                    0,
                                    1 );
        }
    }

    QImage qimage;

    const int DONT_FLIP = 0;
    int flipFlag = DONT_FLIP;

    IplImage* img = cvCreateImage( cvSize( compImgCol->width,
                                           compImgCol->height ), IPL_DEPTH_8U, 3 );
    cvConvertImage( compImgCol, img );

    // Convert from IplImage to QImage
    const QSize imgSize( img->width,img->height );
    qimage = QImage( imgSize, QImage::Format_RGB888 );


    CvMat mtxWrapper;
    cvInitMatHeader( &mtxWrapper,
                     img->height,
                     img->width,
                     CV_8UC3,
                     qimage.bits() );

    cvConvertImage( img, &mtxWrapper, flipFlag );
    cvReleaseImage( &img );

    m_imageView.SetImage( qimage );
    m_imageView.update();
}
