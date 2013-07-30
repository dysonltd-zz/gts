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

#include "MeasureFloorToolWidget.h"

#include "ui_MeasureFloorToolWidget.h"

#include "FloorMaskSchema.h"

#include "ImageGrid.h"
#include "ImageView.h"

#include "OpenCvTools.h"

#include <opencv/highgui.h>

#include <QMessageBox>
#include <QFileDialog>

MeasureFloorToolWidget::MeasureFloorToolWidget( QWidget* parent ) :
    Tool         ( parent, CreateSchema() ),
    m_ui         ( new Ui::MeasureFloorToolWidget ),
    m_startPoint ( false ),
    m_endPoint   ( false )
{
    SetupUi();

    ConnectSignals();

    CreateMappers();
}

void MeasureFloorToolWidget::SetupUi()
{
    m_ui->setupUi( this );

    m_imageView = m_ui->m_imageGrid->AddBlankImage( m_ui->m_imageGrid->size() );
}

void MeasureFloorToolWidget::ConnectSignals()
{
    QObject::connect( m_imageView,
                      SIGNAL( onLeftClick(int, int, int) ),
                      this,
                      SLOT( ViewClicked(int, int, int) ) );

    QObject::connect( m_ui->m_overlayMask,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( OverlayMaskClicked() ) );
}

void MeasureFloorToolWidget::CreateMappers()
{
}

MeasureFloorToolWidget::~MeasureFloorToolWidget()
{
    delete m_ui;
}

const QString MeasureFloorToolWidget::Name() const
{
    return tr( "Measure Floor" );
}

QWidget* MeasureFloorToolWidget::Widget()
{
    return this;
}

const QString MeasureFloorToolWidget::GetSubSchemaDefaultFileName() const
{
    return "measureFloor.xml";
}

void MeasureFloorToolWidget::ReloadCurrentConfigToolSpecific()
{
    const QString planName(
        GetCurrentConfig().GetAbsoluteFileNameFor( "floor_plan.png" ) );
    const QString maskName(
        GetCurrentConfig().GetAbsoluteFileNameFor( "floor_mask.png" ) );

    m_ui->m_overlayMask->setEnabled(false);

    m_ui->m_planSizeEdit->setText("");
    m_ui->m_maskSizeEdit->setText("");

    m_ui->m_startPointEdit->setText("");
    m_ui->m_endPointEdit->setText("");
    m_ui->m_distanceEdit->setText("");

    m_startPoint = false;
    m_endPoint = false;

    if ( QFile::exists( planName ) )
    {
        IplImage* floorPlanImg = cvLoadImage( planName.toAscii() );

        if (floorPlanImg)
        {
            const int nTotalPixels = floorPlanImg->width * floorPlanImg->height;
            m_ui->m_planSizeEdit->setText(QObject::tr("%1 pixels").arg(nTotalPixels));

            cvReleaseImage( &floorPlanImg );
        }
    }

    if ( QFile::exists( maskName ) )
    {
        IplImage* floorMaskImg = OpenCvTools::LoadSingleChannelImage( maskName.toAscii().data() );

        if (floorMaskImg)
        {
            // Number of non-zero pixels in floor mask is total number of
            // pixels of interest - the size of the area we are looking at.

            const int nMaskPixels = cvCountNonZero( floorMaskImg );
            m_ui->m_maskSizeEdit->setText(QObject::tr("%1 pixels").arg(nMaskPixels));

            cvReleaseImage( &floorMaskImg );

            m_ui->m_overlayMask->setEnabled(true);
        }
    }

    ShowImage();
}

void MeasureFloorToolWidget::OverlayMaskClicked()
{
    ShowImage();
}

const WbSchema MeasureFloorToolWidget::CreateSchema()
{
    WbSchema schema( CreateWorkbenchSubSchema( KeyName( "measureFloor" ),
                                               tr( "Measure Floor" ) ) );

    return schema;
}

void MeasureFloorToolWidget::ShowImage()
{
    CvScalar colours[4] = { CV_RGB( 0,   0,   255 ),
                            CV_RGB( 0,   255, 0   ),
                            CV_RGB( 255, 255, 0   ),
                            CV_RGB( 0,   255, 255 ) };

    const QString planName(
        GetCurrentConfig().GetAbsoluteFileNameFor( "floor_plan.png" ) );
    const QString maskName(
        GetCurrentConfig().GetAbsoluteFileNameFor( "floor_mask.png" ) );

    IplImage* floorPlanImg = cvLoadImage( planName.toAscii() );

    if (floorPlanImg)
    {
        if (m_ui->m_overlayMask->isChecked())
        {
            IplImage* floorMaskImg = OpenCvTools::LoadSingleChannelImage( maskName.toAscii().data() );

           // Overlay floor mask onto floor image...
            OpenCvTools::DrawColouredOverlay( floorPlanImg,
                                              floorMaskImg,
                                              CV_RGB(100,0,0),
                                              std::bind2nd(std::equal_to<int>(), 255) );

            cvReleaseImage( &floorMaskImg );
        }

        if (m_startPoint)
        {
            CvPoint posStart = cvPoint( m_startPointX,
                                        m_startPointY );

	        cvCircle( floorPlanImg, posStart, 3, colours[2], 1, CV_AA );

            if (m_endPoint)
            {
                CvPoint posEnd = cvPoint( m_endPointX,
                                          m_endPointY );

	            cvCircle( floorPlanImg, posEnd, 3, colours[2], 1, CV_AA );

	            cvLine( floorPlanImg, posStart, posEnd, colours[0], 1, CV_AA );
            }
        }

        // Convert image...
        IplImage* imgTmp = cvCreateImage( cvSize( floorPlanImg->width,
                                                  floorPlanImg->height ), IPL_DEPTH_8U, 3 );
        cvConvertImage( floorPlanImg, imgTmp );

        const QSize imgSize( imgTmp->width, imgTmp->height );
        QImage qImg = QImage( imgSize, QImage::Format_RGB888 );

        CvMat mtxWrapper;
        cvInitMatHeader( &mtxWrapper,
                         imgTmp->height,
                         imgTmp->width,
                         CV_8UC3,
                         qImg.bits() );

        cvConvertImage( imgTmp, &mtxWrapper, 0 );

        // Display image...
        m_imageView->Clear();
        m_imageView->SetImage( qImg );
        m_imageView->update();

	    cvReleaseImage( &floorPlanImg );
	    cvReleaseImage( &imgTmp );
    }
}

void MeasureFloorToolWidget::ViewClicked( int id, int x, int y )
{
    Q_UNUSED(id);

    if (!m_startPoint)
    {
        m_startPointX = x;
        m_startPointY = y;
        m_startPoint = true;

        m_ui->m_startPointEdit->setText(QObject::tr("%1,%2").arg(m_startPointX)
                                                            .arg(m_startPointY));
    }
    else
    {
        if (!m_endPoint)
        {
            m_endPointX = x;
            m_endPointY = y;
            m_endPoint = true;
        }
        else
        {
            m_startPointX = m_endPointX;
            m_startPointY = m_endPointY;

            m_endPointX = x;
            m_endPointY = y;
        }

        m_ui->m_startPointEdit->setText(QObject::tr("%1,%2").arg(m_startPointX)
                                                            .arg(m_startPointY));

        m_ui->m_endPointEdit->setText(QObject::tr("%1,%2").arg(m_endPointX)
                                                          .arg(m_endPointY));

        double distX = m_startPointX-m_endPointX;
        double distY = m_startPointY-m_endPointY;

        m_ui->m_distanceEdit->setText(QObject::tr("%1 cm").arg(sqrt(distX*distX +
                                                                    distY*distY)));
    }

    ShowImage();
}
