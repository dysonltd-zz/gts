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

#include "CreateFloorMaskWidget.h"

#include "ui_CreateFloorMaskWidget.h"

#include "RoomLayoutSchema.h"
#include "FloorPlanSchema.h"
#include "FloorMaskSchema.h"

#include "GroundPlaneUtility.h"
#include "OpenCvUtility.h"

#include "FloorPlanning.h"

#include "ImageView.h"

#include "Message.h"

#include "ImageViewer.h"

#include "Logging.h"

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include <QMessageBox>
#include <QFileDialog>
#include <QDir>

CreateFloorMaskWidget::CreateFloorMaskWidget( QWidget* parent ) :
    Tool ( parent, CreateSchema() ),
    m_ui ( new Ui::CreateFloorMaskWidget )
{
    SetupUi();

    ConnectSignals();

    CreateMappers();
}

void CreateFloorMaskWidget::SetupUi()
{
    m_ui->setupUi( this );
}

void CreateFloorMaskWidget::ConnectSignals()
{
    QObject::connect( m_ui->m_combineParts,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( BtnCombinePartsClicked() ) );

    QObject::connect( m_ui->m_exportPlanBtn,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( BtnExportPlanClicked() ) );
    QObject::connect( m_ui->m_importMaskBtn,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( BtnImportMaskClicked() ) );

    QObject::connect( m_ui->m_createMaskBtn,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( BtnCreateMaskClicked() ) );
}

void CreateFloorMaskWidget::CreateMappers()
{
}

CreateFloorMaskWidget::~CreateFloorMaskWidget()
{
    delete m_ui;
}

QWidget* CreateFloorMaskWidget::Widget()
{
    return this;
}

const QString CreateFloorMaskWidget::GetSubSchemaDefaultFileName() const
{
    return "floorMask.xml";
}

void CreateFloorMaskWidget::ReloadCurrentConfigToolSpecific()
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

const WbSchema CreateFloorMaskWidget::CreateSchema()
{
    using namespace FloorMaskSchema;
    WbSchema floorMaskSchema( CreateWorkbenchSubSchema( schemaName,
                                                        tr( "Floor Mask" ) ) );

    return floorMaskSchema;
}

void CreateFloorMaskWidget::ShowImage(ImageView* view, const IplImage* image)
{
    // Convert image
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

    // Display image
    view->Clear();
    view->SetImage( qImg );
    view->update();

	cvReleaseImage( &imgTmp );
}

void CreateFloorMaskWidget::BtnCombinePartsClicked()
{
    if (m_ui->m_combineParts->isChecked())
    {
        m_ui->m_createMaskBtn->setEnabled(true);
    }
    else
    {
        m_ui->m_createMaskBtn->setEnabled(false);
    }
}

void CreateFloorMaskWidget::BtnExportPlanClicked()
{
    if (m_ui->m_combineParts->isChecked())
    {
        ExportFloorPlanParts( GetCurrentConfig() );
    }
    else
    {
        ExportFloorPlan( GetCurrentConfig() );
    }
}

void CreateFloorMaskWidget::BtnImportMaskClicked()
{
    if (m_ui->m_combineParts->isChecked())
    {
        ImportFloorMaskParts( GetCurrentConfig() );
    }
    else
    {
        ImportFloorMask( GetCurrentConfig() );
    }
}

void CreateFloorMaskWidget::BtnCreateMaskClicked()
{
    using namespace FloorPlanSchema;

    WbConfig config = GetCurrentConfig();

    const WbConfig roomLayoutConfig(config.GetParent().GetSubConfig( RoomLayoutSchema::schemaName ) );
    const QStringList cameraPositionIds(roomLayoutConfig
                                        .GetKeyValue(RoomLayoutSchema::cameraPositionIdsKey)
                                        .ToQStringList() );

    if (cameraPositionIds.size() > 0)
    {
        if (cameraPositionIds.size() > 1)
        {
            CreateFloorMaskMulti();
        }
        else
        {
            CreateFloorMaskSingle();
        }
    }
}

bool CreateFloorMaskWidget::ExportFloorPlan( const WbConfig& config )
{
    const QString floorPlanSrcFileName( config.GetAbsoluteFileNameFor( "floor_plan.png" ) );

    if ( !QFile::exists( floorPlanSrcFileName ) )
    {
        QMessageBox::critical( 0,
                               QObject::tr( "Error" ),
                               QObject::tr( "Floor plan image not found." ) );
        return false;
    }

    QString floorPlanDstFileName( QFileDialog::getSaveFileName(  0,
                                                                 QObject::tr( "Choose where to save the floor plan" ),
                                                                 config.GetAbsoluteFileInfo().absolutePath(),
                                                                 tr("Images (*.png)") ) );

    QFileInfo file(floorPlanDstFileName);

    if ( !floorPlanDstFileName.isEmpty() )
    {
        if(file.fileName() != "" && file.suffix().isEmpty())
        {
            floorPlanDstFileName += ".png";
        }

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

bool CreateFloorMaskWidget::ImportFloorMask( const WbConfig& config )
{

    const QString floorMaskSrcFileName(
                QFileDialog::getOpenFileName( 0,
                                              QObject::tr( "Choose where to find the floor mask" ),
                                              config.GetAbsoluteFileInfo().absolutePath() )
                );

    if ( !floorMaskSrcFileName.isEmpty() )
    {
        const QString floorMaskDstFileName( config.GetAbsoluteFileNameFor( "floor_mask.png" ) );

        if ( QFile::exists(floorMaskDstFileName) )
        {
            QFile::remove(floorMaskDstFileName);
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

bool CreateFloorMaskWidget::ExportFloorPlanParts( const WbConfig& config )
{
    bool successful = true;

    QDir directory;

    // Process configured cameras
    const WbConfig roomLayoutConfig(config.GetParent().GetSubConfig( RoomLayoutSchema::schemaName ) );
    const QStringList cameraPositionIds(roomLayoutConfig
                                        .GetKeyValue(RoomLayoutSchema::cameraPositionIdsKey)
                                        .ToQStringList() );

    QString path = QFileDialog::getExistingDirectory (this, tr("Select Destination Directory"), directory.path());

    if ( !path.isNull() )
    {
        directory.setPath(path);

        for ( int i = 0; i < cameraPositionIds.size(); ++i )
        {
            const KeyId camPosId = cameraPositionIds.at( i );


            const QString floorPlanSrcFileName(
                config.GetAbsoluteFileNameFor( "plan_" + camPosId + ".png" ) );

            const QString floorPlanDstFileName( directory.absoluteFilePath( "plan_" + camPosId + ".png" ) );

            if ( QFile::exists(floorPlanDstFileName) )
            {
                QFile::remove(floorPlanDstFileName);
            }

            successful = QFile::copy( floorPlanSrcFileName, floorPlanDstFileName );

            if ( !successful )
            {
                QMessageBox::critical( 0,
                                       QObject::tr( "Error" ),
                                       QObject::tr( "Failed to copy file '%1' to '%2'" )
                                                .arg( floorPlanSrcFileName )
                                                .arg( floorPlanDstFileName ) );
                break;
            }
        }
    }

    return successful;
}

bool CreateFloorMaskWidget::ImportFloorMaskParts( const WbConfig& config )
{
    bool successful = true;

    QDir directory;

    // Process configured cameras
    const WbConfig roomLayoutConfig(config.GetParent().GetSubConfig( RoomLayoutSchema::schemaName ) );
    const QStringList cameraPositionIds(roomLayoutConfig
                                        .GetKeyValue(RoomLayoutSchema::cameraPositionIdsKey)
                                        .ToQStringList() );

    QString path = QFileDialog::getExistingDirectory (this, tr("Select Source Directory"), directory.path());

    if ( !path.isNull() )
    {
        directory.setPath(path);

        for ( int i = 0; i < cameraPositionIds.size(); ++i )
        {
            const KeyId camPosId = cameraPositionIds.at( i );

            const QString floorMaskDstFileName(
                config.GetAbsoluteFileNameFor( "mask_" + camPosId + ".png" ) );

            const QString floorMaskSrcFileName( directory.absoluteFilePath( "mask_" + camPosId + ".png" ) );

            if ( QFile::exists(floorMaskDstFileName) )
            {
                QFile::remove(floorMaskDstFileName);
            }

            successful = QFile::copy( floorMaskSrcFileName, floorMaskDstFileName );

            if ( !successful )
            {
                QMessageBox::critical( 0,
                                        QObject::tr( "Error" ),
                                        QObject::tr( "Failed to copy file '%1' to '%2'" )
                                                .arg( floorMaskSrcFileName )
                                                .arg( floorMaskDstFileName ) );
                break;
            }
        }
    }

    return successful;
}

WbConfig CreateFloorMaskWidget::GetFloorPlanConfig()
{
    return GetCurrentConfig().GetParent().GetSubConfig( FloorPlanSchema::schemaName );
}

void CreateFloorMaskWidget::CreateFloorMaskSingle()
{
    WbConfig config = GetCurrentConfig();

    const WbConfig roomLayoutConfig(config.GetParent().GetSubConfig( RoomLayoutSchema::schemaName ) );
    const KeyId camPosId = roomLayoutConfig.GetKeyValue( RoomLayoutSchema::cameraPositionIdsKey ).ToKeyId();

    const QString imgFile(config.GetAbsoluteFileNameFor("mask_" + camPosId + ".png"));

    IplImage* imgComposite;

    CvPoint2D32f offset;

    LOG_TRACE(QObject::tr("Creating (single) floor mask for %1").arg(camPosId));

    if (FloorPlanning::LoadFile( GetFloorPlanConfig(), camPosId, &imgComposite, imgFile, &offset, false ))
    {
        LOG_INFO(QObject::tr("Loading image %1.").arg(imgFile));

        // Convert image
        IplImage* imgTmp = cvCreateImage( cvSize( imgComposite->width,
                                                  imgComposite->height ), IPL_DEPTH_8U, 3 );
        cvConvertImage( imgComposite, imgTmp );

        const QSize imgSize( imgTmp->width, imgTmp->height );
        QImage qImg = QImage( imgSize, QImage::Format_RGB888 );

        CvMat mtxWrapper;
        cvInitMatHeader( &mtxWrapper,
                         imgTmp->height,
                         imgTmp->width,
                         CV_8UC3,
                         qImg.bits() );

        cvConvertImage( imgTmp, &mtxWrapper, 0 );

        const QString fileName(
            config.GetAbsoluteFileNameFor( "floor_mask.png" ) );

        if (!qImg.save( fileName ))
        {
            Message::Show( this,
                           tr( "Create Floor Mask" ),
                           tr( "Error - Cannot write to: %1" )
                               .arg( fileName ),
                           Message::Severity_Critical );
        }

        cvReleaseImage( &imgComposite );

        // show floor plan
        ImageViewer(imgTmp, this).exec();
        cvReleaseImage( &imgTmp );
        Message::Show( this,
                       tr( "Floor Mask Information" ),
                       tr( "Single-position floor mask successfully created." ),
                       Message::Severity_Information );
    }
    else
    {
        Message::Show( this,
                       tr( "Create Floor Mask" ),
                       tr( "Error - Cannot load from: %1!" )
                           .arg( imgFile ),
                       Message::Severity_Critical );
    }

    CvMat* identity = cvCreateMat( 3, 3, CV_32F );

    cvSetIdentity( identity );

    LOG_TRACE("Done.");

    cvReleaseMat( &identity );
}

void CreateFloorMaskWidget::CreateFloorMaskMulti()
{
    LOG_TRACE("Creating (multi) floor mask");

    LOG_TRACE("Checking mapping");

    // Check camera mapping(s)
    if (FloorPlanning::CheckMappingIsComplete(GetFloorPlanConfig()))
    {
        LOG_TRACE("Finding root");

        // Find the base camera(s)
        std::vector<KeyId> rootCamera = FloorPlanning::FindRoot(GetFloorPlanConfig());

        if (rootCamera.size() == 1)
        {
            LOG_INFO(QObject::tr("Checking root mapping for %1.").arg(rootCamera.front()));

            // Find a path from camera to the base
            if (FloorPlanning::CheckRootMapping(GetFloorPlanConfig(), rootCamera.front()))
            {
                LOG_INFO(QObject::tr("Compositing images with %1.").arg(rootCamera.front()));

                Stitch(rootCamera.front());
            }
            else
            {
                Message::Show( this,
                               tr( "Create Floor Mask" ),
                               tr( "Error - Mapping is incomplete!" ),
                               Message::Severity_Critical );
            }
        }
        else
        {
            Message::Show( this,
                           tr( "Create Floor Mask" ),
                           tr( "Error - Need one root camera!" ),
                           Message::Severity_Critical );
        }
    }
    else
    {
        Message::Show( this,
                       tr( "Create Floor Mask" ),
                       tr( "Error - Must map every camera!" ),
                       Message::Severity_Critical );
    }

    LOG_TRACE("Done.");
}

void CreateFloorMaskWidget::Stitch(KeyId camRoot)
{
    WbConfig config = GetFloorPlanConfig();

    // Create composite
    IplImage *imgComposite;

    CvMat* transform = cvCreateMat( 3, 3, CV_32F );
    CvMat* identity = cvCreateMat( 3, 3, CV_32F );

    // Load root image
    KeyValue rootImage;

    const WbKeyValues::ValueIdPairList cameraMappingIds = config.GetKeyValues( FloorPlanSchema::homographyKey );

    for (WbKeyValues::ValueIdPairList::const_iterator it = cameraMappingIds.begin(); it != cameraMappingIds.end(); ++it)
    {
        const KeyId cameraId( config.GetKeyValue( FloorPlanSchema::camera1IdKey, it->id ).ToKeyId() );

        if (cameraId == camRoot)
        {
            rootImage = config.GetKeyValue( FloorPlanSchema::camera1ImgKey, it->id );
            break;
        }
    }

    const QString rootFileName(config.GetAbsoluteFileNameFor("mask_" + camRoot + ".png"));

    LOG_INFO(QObject::tr("Root image: %1").arg(rootFileName));

    IplImage* rootImg;
    CvPoint2D32f offset;
    FloorPlanning::LoadFile( GetFloorPlanConfig(), camRoot, &rootImg, rootFileName, &offset, false );

    cvSetIdentity(identity);

    GroundPlaneUtility::Rect32f cmpBox;
    cmpBox.pos.x = 0;
    cmpBox.pos.y = 0;
    cmpBox.dim.x = (float)rootImg->width;
    cmpBox.dim.y = (float)rootImg->height;

    LOG_INFO(QObject::tr("Composite bounding box is %1 %2 %3 %4.").arg(cmpBox.dim.x)
                                                                  .arg(cmpBox.dim.y)
                                                                  .arg(cmpBox.pos.x)
                                                                  .arg(cmpBox.pos.y));

    // Process remaining (non-root) cameras
    const WbConfig roomLayoutConfig(config.GetParent().GetSubConfig( RoomLayoutSchema::schemaName ) );
    const QStringList cameraPositionIds(roomLayoutConfig
                                        .GetKeyValue(RoomLayoutSchema::cameraPositionIdsKey)
                                        .ToQStringList() );

    for ( int i = 0; i < cameraPositionIds.size(); ++i )
    {
        const KeyId camPosId = cameraPositionIds.at( i );

        if (camPosId != camRoot)
        {
            LOG_INFO(QObject::tr("Find chain for %1 - %2.").arg(camPosId)
                                                           .arg(camRoot));

            std::vector<KeyId> chain = FloorPlanning::FindChain(GetFloorPlanConfig(), camPosId, camRoot, std::vector<KeyId>());

            cvSetIdentity(transform);
            FloorPlanning::ComputeTransform( GetFloorPlanConfig(), camPosId, chain, transform );

            // Load camera image
            KeyValue camImageFile;

            for (WbKeyValues::ValueIdPairList::const_iterator it = cameraMappingIds.begin(); it != cameraMappingIds.end(); ++it)
            {
                const KeyId cameraId( config.GetKeyValue( FloorPlanSchema::camera2IdKey, it->id ).ToKeyId() );

                if (cameraId == camPosId)
                {
                    camImageFile = config.GetKeyValue( FloorPlanSchema::camera2ImgKey, it->id );
                    break;
                }
            }

            const QString camFileName(config.GetAbsoluteFileNameFor("mask_" + camPosId + ".png"));

            LOG_INFO(QObject::tr("Camera image: %1").arg(camFileName));

            IplImage* camImg;
            CvPoint2D32f offset;
            FloorPlanning::LoadFile( GetFloorPlanConfig(), camPosId, &camImg, camFileName, &offset, false );

            // Stitch the images together
            GroundPlaneUtility::compositeImageBoundingBox( transform, camImg, &cmpBox );

            LOG_INFO(QObject::tr("Composite bounding box is %1 %2 %3 %4.").arg(cmpBox.dim.x)
                                                                          .arg(cmpBox.dim.y)
                                                                          .arg(cmpBox.pos.x)
                                                                          .arg(cmpBox.pos.y));
        }
    }

    // Now we know dimensions and origin of the composite
    // image so we can allocate it and transform all images.
    int sizex = (int)(cmpBox.dim.x+.5f);
    int sizey = (int)(cmpBox.dim.y+.5f);

    sizex += 4 -( sizex % 4 );

    CvSize cmpSize = cvSize( sizex,sizey );
    imgComposite = cvCreateImage( cmpSize, rootImg->depth, rootImg->nChannels );

    // Process remaining (non-root) cameras
    for ( int i = 0; i < cameraPositionIds.size(); ++i )
    {
        const KeyId camPosId = cameraPositionIds.at( i );

        if (camPosId != camRoot)
        {
            std::vector<KeyId> chain = FloorPlanning::FindChain(GetFloorPlanConfig(), camPosId, camRoot, std::vector<KeyId>());

            cvSetIdentity(transform);
            FloorPlanning::ComputeTransform( GetFloorPlanConfig(), camPosId, chain, transform );

            // Load camera image
            KeyValue camImageFile;

            for (WbKeyValues::ValueIdPairList::const_iterator it = cameraMappingIds.begin(); it != cameraMappingIds.end(); ++it)
            {
                const KeyId cameraId( config.GetKeyValue( FloorPlanSchema::camera2IdKey, it->id ).ToKeyId() );

                if (camPosId == cameraId)
                {
                    camImageFile = config.GetKeyValue( FloorPlanSchema::camera2ImgKey, it->id );
                    break;
                }
            }

            const QString camFileName(config.GetAbsoluteFileNameFor("mask_" + camPosId + ".png"));

            LOG_INFO(QObject::tr("Camera filename: %1").arg(camFileName));

            IplImage* camImg;
            CvPoint2D32f offset;
            FloorPlanning::LoadFile( GetFloorPlanConfig(), camPosId, &camImg, camFileName, &offset, false );

            // Align the image.
            cvZero( imgComposite );
            GroundPlaneUtility::alignGroundPlane( transform, camImg, imgComposite );
            IplImage *img1 = cvCloneImage( imgComposite );

            // Align root image.
            cvZero( imgComposite );
            GroundPlaneUtility::alignGroundPlane( identity, rootImg, imgComposite );
            IplImage *img2 = cvCloneImage( imgComposite );

            GroundPlaneUtility::createCompositeImage( img1, imgComposite, imgComposite );
            GroundPlaneUtility::createCompositeImage( img2, imgComposite, imgComposite );

            cvReleaseImage( &img1 );
            cvReleaseImage( &img2 );
        }
    }

    // Convert image
    IplImage* imgTmp = cvCreateImage( cvSize( imgComposite->width,
                                              imgComposite->height ), IPL_DEPTH_8U, 3 );
    cvConvertImage( imgComposite, imgTmp );

    const QSize imgSize( imgTmp->width, imgTmp->height );
    QImage qImg = QImage( imgSize, QImage::Format_RGB888 );

    CvMat mtxWrapper;
    cvInitMatHeader( &mtxWrapper,
                     imgTmp->height,
                     imgTmp->width,
                     CV_8UC3,
                     qImg.bits() );

    cvConvertImage( imgTmp, &mtxWrapper, 0 );

     const QString fileName(
         config.GetAbsoluteFileNameFor( "floor_mask.png" ) );

    if (!qImg.save( fileName ))
    {
        Message::Show( this,
                       tr( "Create Floor Mask" ),
                       tr( "Error - Cannot write to: %1!" )
                           .arg( fileName ),
                       Message::Severity_Critical );
    }

    cvReleaseMat( &transform );
    cvReleaseMat( &identity );

    cvReleaseImage( &imgComposite );
    ImageViewer(imgTmp, this).exec();
    cvReleaseImage( &imgTmp );
    Message::Show( this,
                   tr( "Floor Mask Information" ),
                   tr( "Multi-position floor mask successfully created." ),
                   Message::Severity_Information );
}
