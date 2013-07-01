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

#include "CreateFloorMaskToolWidget.h"

#include "ui_CreateFloorMaskToolWidget.h"

#include "FloorMaskSchema.h"

#include "ImageView.h"

#include <QMessageBox>
#include <QFileDialog>

CreateFloorMaskToolWidget::CreateFloorMaskToolWidget( QWidget* parent ) :
    Tool     ( parent, CreateSchema() ),
    m_ui     ( new Ui::CreateFloorMaskToolWidget )
{
    SetupUi();

    ConnectSignals();

    CreateMappers();
}

void CreateFloorMaskToolWidget::SetupUi()
{
    m_ui->setupUi( this );
}

void CreateFloorMaskToolWidget::ConnectSignals()
{
    QObject::connect( m_ui->m_exportPlanBtn,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( BtnExportPlanClicked() ) );
    QObject::connect( m_ui->m_importMaskBtn,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( BtnImportMaskClicked() ) );
}

void CreateFloorMaskToolWidget::CreateMappers()
{
}

CreateFloorMaskToolWidget::~CreateFloorMaskToolWidget()
{
    delete m_ui;
}

const QString CreateFloorMaskToolWidget::Name() const
{
    return tr( "Create Floor Mask" );
}

QWidget* CreateFloorMaskToolWidget::Widget()
{
    return this;
}

const QString CreateFloorMaskToolWidget::GetSubSchemaDefaultFileName() const
{
    return "floorMask.xml";
}

void CreateFloorMaskToolWidget::ReloadCurrentConfigToolSpecific()
{
    const QString planName(
        GetCurrentConfig().GetAbsoluteFileNameFor( "floor_plan.png" ) );

    if ( QFile::exists( planName ) )
    {
        m_ui->m_exportPlanBtn->setEnabled(true);
        m_ui->m_importMaskBtn->setEnabled(true);

        IplImage* img = cvLoadImage(planName.toAscii(), CV_LOAD_IMAGE_GRAYSCALE);

        if (img)
        {
            ShowImage(m_ui->m_planView, img);
            cvReleaseImage( &img );
        }
    }

    const QString maskName(
        GetCurrentConfig().GetAbsoluteFileNameFor( "floor_mask.png" ) );

    if ( QFile::exists( maskName ) )
    {
        IplImage* img = cvLoadImage(maskName.toAscii(), CV_LOAD_IMAGE_GRAYSCALE);

        if (img)
        {
            ShowImage(m_ui->m_maskView, img);
            cvReleaseImage( &img );

        }
    }
}

const WbSchema CreateFloorMaskToolWidget::CreateSchema()
{
    using namespace FloorMaskSchema;
    WbSchema floorMaskSchema( CreateWorkbenchSubSchema( schemaName,
                                                        tr( "Floor Mask" ) ) );

    return floorMaskSchema;
}

void CreateFloorMaskToolWidget::ShowImage(ImageView* view, const IplImage* image)
{
    // Convert image...
    IplImage* imgTmp = cvCreateImage( cvSize( image->width, image->height ), IPL_DEPTH_8U, 3 );
    cvConvertImage( image, imgTmp );

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
    view->Clear();
    view->SetImage( qImg );
    view->update();

	cvReleaseImage( &imgTmp );
}

void CreateFloorMaskToolWidget::BtnExportPlanClicked()
{
    ExportFloorPlan( GetCurrentConfig() );
}

void CreateFloorMaskToolWidget::BtnImportMaskClicked()
{
    ImportFloorMask( GetCurrentConfig() );
}

bool CreateFloorMaskToolWidget::ExportFloorPlan( const WbConfig& config )
{
    const QString floorPlanSrcFileName(
        config.GetAbsoluteFileNameFor( "floor_plan.png" ) );

    if ( !QFile::exists( floorPlanSrcFileName ) )
    {
        QMessageBox::critical( 0,
                               QObject::tr( "Error" ),
                               QObject::tr( "Floor plan image not found." ) );
        return false;
    }

    const QString floorPlanDstFileName( QFileDialog::getSaveFileName( 0,
                                                                     QObject::tr( "Choose where to save the floor plan" ),
                                                                     QDir::homePath() ) );

    if ( !floorPlanDstFileName.isEmpty() )
    {
        if ( QFile::exists( floorPlanDstFileName ) )
        {
            QFile::remove( floorPlanDstFileName );
        }

        bool copySuccessFul = QFile::copy( floorPlanSrcFileName, floorPlanDstFileName );

        if ( !copySuccessFul )
        {
            QMessageBox::critical( 0,
                                   QObject::tr( "Error" ),
                                   QObject::tr( "Failed to copy file '%1' to '%2'" )
                                            .arg( floorPlanSrcFileName )
                                            .arg( floorPlanDstFileName ) );
        }

        return copySuccessFul;
    }
    else
    {
        return true;
    }
}

bool CreateFloorMaskToolWidget::ImportFloorMask( const WbConfig& config )
{
    const QString floorMaskSrcFileName( QFileDialog::getOpenFileName( 0,
                                                                      QObject::tr( "Choose where to find the floor mask" ),
                                                                      QDir::homePath() ) );

    if ( !floorMaskSrcFileName.isEmpty() )
    {
        const QString floorMaskDstFileName(
            config.GetAbsoluteFileNameFor( "floor_mask.png" ) );

        if ( QFile::exists( floorMaskDstFileName ) )
        {
            QFile::remove( floorMaskDstFileName );
        }

        bool copySuccessFul = QFile::copy( floorMaskSrcFileName, floorMaskDstFileName );

        if ( !copySuccessFul )
        {
            QMessageBox::critical( 0,
                                   QObject::tr( "Error" ),
                                   QObject::tr( "Failed to copy file '%1' to '%2'" )
                                            .arg( floorMaskSrcFileName )
                                            .arg( floorMaskDstFileName ) );
        }

        ReloadCurrentConfig();

        return copySuccessFul;
    }
    else
    {
        return false;
    }
}
