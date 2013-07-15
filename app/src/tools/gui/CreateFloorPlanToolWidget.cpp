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

#include "CreateFloorPlanToolWidget.h"

#include "ui_CreateFloorPlanToolWidget.h"

#include "RoomsCollection.h"
#include "CamerasCollection.h"
#include "CameraPositionsCollection.h"

#include "CalibrationSchema.h"
#include "ExtrinsicCalibrationSchema.h"
#include "CameraPositionSchema.h"
#include "RoomLayoutSchema.h"

#include "FloorPlanSchema.h"

#include "CameraRelationsTableMapper.h"

#include "CaptureLiveDualController.h"

#include "CalibrationAlgorithm.h"

#include "WbConfigTools.h"
#include "WbConfig.h"

#include "GroundPlaneUtility.h"
#include "OpenCvUtility.h"

#include <iostream>
#include <algorithm>

#include "RobotMetrics.h"
#include "CameraCalibration.h"

#include "ImageGrid.h"
#include "ImageView.h"

#include "FileUtilities.h"
#include "FileDialogs.h"

#include "Message.h"

#include "ImageViewer.h"

#include "CameraHardware.h"

#include "WbConfigTools.h"

#include "Logging.h"

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include <QFileDialog>
#include <QtGlobal>


namespace
{
    /**
     * Rotate p according to rotation matrix (from getRotationMatrix2D()) R
     * @param R     Rotation matrix from getRotationMatrix2D()
     * @param p     Point2f to rotate
     * @return      Returns rotated coordinates in a Point2f
     */
    cv::Point2f rotPoint(const cv::Mat &R, const cv::Point2f &p)
    {
        cv::Point2f rp;

        rp.x = (float)(R.at<double>(0,0)*p.x +
                       R.at<double>(0,1)*p.y +
                       R.at<double>(0,2));
        rp.y = (float)(R.at<double>(1,0)*p.x +
                       R.at<double>(1,1)*p.y +
                       R.at<double>(1,2));

        return rp;
    }

    /**
     * Return the size needed to contain bounding box bb when rotated by R
     * @param R     Rotation matrix from getRotationMatrix2D()
     * @param bb    bounding box rectangle to be rotated by R
     * @return      Size of image(width,height) that will compleley contain bb when rotated by R
     */
    cv::Size rotatedImageBB(const cv::Mat &R, const cv::Rect &bb)
    {
        // Rotate the rectangle...
        std::vector<cv::Point2f> rp;

        rp.push_back( rotPoint(R, cv::Point2f(bb.x, bb.y)) );
        rp.push_back( rotPoint(R, cv::Point2f(bb.x + bb.width, bb.y)) );
        rp.push_back( rotPoint(R, cv::Point2f(bb.x + bb.width, bb.y + bb.height)) );
        rp.push_back( rotPoint(R, cv::Point2f(bb.x, bb.y + bb.height)) );

        // Find box...
        float x = rp[0].x;
        float y = rp[0].y;
        float left = x,
              right = x,
              up = y,
              down = y;

        for (int i = 1; i<4; ++i)
        {
            x = rp[i].x;
            y = rp[i].y;

            if (left > x) left = x;
            if (right < x) right = x;
            if (up > y) up = y;
            if (down < y) down = y;
        }

        int w = (int)(right - left + 0.5);
        int h = (int)(down - up + 0.5);

        return cv::Size(w,h);
    }

    /**
     * Rotate region "fromroi" in image "fromI" a total of "angle" degrees and put it in "toI" if toI exists.
     * If toI doesn't exist, create it such that it will hold the entire rotated region. Return toI, rotated imge
     *   This will put the rotated fromroi piece of fromI into the toI image
     *
     * @param fromI     Input image to be rotated
     * @param toI       Output image if provided, (else if &toI = 0, it will create a Mat fill it with the rotated image roi, and return it).
     * @param fromroi   roi region in fromI to be rotated.
     * @param angle     Angle in degrees to rotate
     * @return          Rotated image (you can ignore if you passed in toI
     */
    cv::Mat rotateImage(const cv::Mat &fromI, cv::Mat *toI, const cv::Rect &fromROI, double angle)
    {
        // Make or get the "toI" matrix...
        cv::Point2f cx((float)fromROI.x + (float)fromROI.width/2.0,
                       (float)fromROI.y + (float)fromROI.height/2.0);

        cv::Mat R = getRotationMatrix2D(cx, angle, 1);

        cv::Mat rotI;

        if (toI)
            rotI = *toI;
        else
        {
            cv::Size rs = rotatedImageBB(R, fromROI);

            rotI.create(rs, fromI.type());
        }

        // Adjust for shifts...
        double wdiff = (double)((cx.x - rotI.cols/2.0));
        double hdiff = (double)((cx.y - rotI.rows/2.0));

        R.at<double>(0,2) -= wdiff; // Adjust the rotation point
        R.at<double>(1,2) -= hdiff; // to middle of the dst image

        // Rotate...
        warpAffine(fromI, rotI, R, rotI.size(), cv::INTER_CUBIC,
                                                cv::BORDER_CONSTANT,
                                                cv::Scalar::all(0));

        return(rotI);
    }
}

CreateFloorPlanToolWidget::CreateFloorPlanToolWidget( CameraHardware& cameraHardware,
                                                      QWidget* parent ) :
    Tool                        ( parent, CreateSchema() ),
    m_ui                        ( new Ui::CreateFloorPlanToolWidget ),
    m_captureLiveDualController (),
    m_rotAngle                  ( 0 )
{
    SetupUi();

    m_captureLiveDualController.reset(
        new CaptureLiveDualController( *m_ui->m_captureLiveBtn,
                                       *m_ui->m_captureCancelBtn,
                                       *this,
                                       cameraHardware ) );

	ConnectSignals();
    CreateMappers();
}

void CreateFloorPlanToolWidget::SetupUi()
{
    m_ui->setupUi( this );

#if 0
    ResetUi();
#endif
}

void CreateFloorPlanToolWidget::FillOutCameraCombo( QComboBox& comboBox )
{
    WbConfig config = GetCurrentConfig();

    Collection camPosCollection(CameraPositionsCollection());
    camPosCollection.SetConfig(config);

    comboBox.clear();

    const WbConfig roomLayoutConfig(config.GetParent().GetSubConfig( RoomLayoutSchema::schemaName ) );
    const QStringList camPosIds(roomLayoutConfig
                                 .GetKeyValue(RoomLayoutSchema::cameraPositionIdsKey)
                                 .ToQStringList() );

    for (auto camPosId = camPosIds.begin(); camPosId != camPosIds.end(); ++camPosId)
    {
        comboBox.addItem( WbConfigTools::DisplayNameOf(camPosCollection.ElementById(*camPosId)), QVariant( *camPosId ) );
    }

}

void CreateFloorPlanToolWidget::ReloadCurrentConfigToolSpecific()
{
    WbConfig config = GetCurrentConfig();

    // Check whether the room has more than one camera position...
    const WbConfig roomLayoutConfig(config.GetParent().GetSubConfig( RoomLayoutSchema::schemaName ) );
    const QStringList cameraPositionIds(roomLayoutConfig
                                        .GetKeyValue(RoomLayoutSchema::cameraPositionIdsKey)
                                        .ToQStringList() );

    FillOutCameraCombo( *m_ui->m_camera1Combo );
    FillOutCameraCombo( *m_ui->m_camera2Combo );

    if (cameraPositionIds.size() > 1)
    {
        m_ui->m_camera1Combo->setEnabled(true);
        m_ui->m_camera2Combo->setEnabled(true);

        m_ui->m_camera1Combo->setCurrentIndex( 0 );
        m_ui->m_camera2Combo->setCurrentIndex( 1 );
    }
    else
    {
        m_ui->m_camera1Combo->setCurrentIndex( -1 );
        m_ui->m_camera2Combo->setCurrentIndex( -1 );
        m_ui->m_mappingsTable->setDisabled(true);
    }
}

void CreateFloorPlanToolWidget::ConnectSignals()
{
    QObject::connect( m_ui->m_camera1Combo,
                      SIGNAL( currentIndexChanged (int) ),
                      this,
                      SLOT( CameraComboChanged() ) );
    QObject::connect( m_ui->m_camera2Combo,
                      SIGNAL( currentIndexChanged (int) ),
                      this,
                      SLOT( CameraComboChanged() ) );

    QObject::connect( m_ui->m_getImageFromFileBtn,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( FromFileBtnClicked() ) );
    QObject::connect( m_ui->m_captureLiveBtn,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( CaptureLiveBtnClicked() ) );

    QObject::connect( m_ui->m_captureCancelBtn,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( CaptureCancelBtnClicked() ) );

    QObject::connect( m_ui->btnRotate,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( BtnRotateClicked() ) );

    QObject::connect( m_ui->btnMatch,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( BtnMatchClicked() ) );
    QObject::connect( m_ui->btnStitch,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( BtnStitchClicked() ) );

    QObject::connect( m_ui->btnSave,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( BtnSaveClicked() ) );

    QObject::connect( m_ui->btnCancel,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( BtnCancelClicked() ) );

    QObject::connect( m_ui->btnCreateFloorPlan,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( BtnCreateFloorPlanClicked() ) );
}

void CreateFloorPlanToolWidget::CreateMappers()
{
    using namespace FloorPlanSchema;

    AddMapper( calGridRowsKey, m_ui->m_gridRowsSpinBox );
    AddMapper( calGridColsKey, m_ui->m_gridColumnsSpinBox );

    m_relationsMapper = new CameraRelationsTableMapper( *m_ui->m_mappingsTable );
    AddMapper( m_relationsMapper );
}

CreateFloorPlanToolWidget::~CreateFloorPlanToolWidget()
{
    delete m_ui;
}

const QString CreateFloorPlanToolWidget::Name() const
{
    return tr( "Create Floor Plan" );
}

QWidget* CreateFloorPlanToolWidget::Widget()
{
    return this;
}

const QString CreateFloorPlanToolWidget::GetSubSchemaDefaultFileName() const
{
    return "floorPlan.xml";
}

ImageView* const CreateFloorPlanToolWidget::GetStreamingView1( const QSize& imageSize )
{
    Q_UNUSED(imageSize);

    return m_ui->m_liveView1;
}

ImageView* const CreateFloorPlanToolWidget::GetStreamingView2( const QSize& imageSize )
{
    Q_UNUSED(imageSize);

    return m_ui->m_liveView2;
}

void CreateFloorPlanToolWidget::ResetUi()
{
    WbConfig config = GetCurrentConfig();

    //Check whether the room has more than one camera position...
    const WbConfig roomLayoutConfig(config.GetParent().GetSubConfig( RoomLayoutSchema::schemaName ) );
    const QStringList cameraPositionIds(roomLayoutConfig
                                         .GetKeyValue(RoomLayoutSchema::cameraPositionIdsKey)
                                         .ToQStringList() );

    m_ui->m_camera1Combo->setEnabled(true);
    m_ui->m_camera2Combo->setEnabled(true);

    if (cameraPositionIds.size() > 1)
    {
        m_ui->m_camera1Combo->setCurrentIndex( 0 );
        m_ui->m_camera2Combo->setCurrentIndex( 0 );
    }
    else
    {
        m_ui->m_camera1Combo->setCurrentIndex( -1 );
        m_ui->m_camera2Combo->setCurrentIndex( -1 );
    }

    m_ui->m_getImageFromFileBtn->setEnabled(false);
    m_ui->m_captureLiveBtn->setEnabled(false);

    m_ui->m_liveView1->Clear();
    m_ui->m_liveView1->update();

    m_ui->m_liveView2->Clear();
    m_ui->m_liveView2->update();

    m_ui->btnRotate->setEnabled(false);
    m_ui->btnMatch->setEnabled(false);
    m_ui->btnStitch->setEnabled(false);
    m_ui->btnSave->setEnabled(false);
}

const QString CreateFloorPlanToolWidget::GetCamera1Id() const
{
    QComboBox* const cameraCombo = m_ui->m_camera1Combo;
    const int newTargetIndex = cameraCombo->currentIndex();
    const QString cameraId( cameraCombo->itemData( newTargetIndex ).toString() );
    return cameraId;
}

const QString CreateFloorPlanToolWidget::GetCamera2Id() const
{
    QComboBox* const cameraCombo = m_ui->m_camera2Combo;
    const int newTargetIndex = cameraCombo->currentIndex();
    const QString cameraId( cameraCombo->itemData( newTargetIndex ).toString() );
    return cameraId;
}

const WbSchema CreateFloorPlanToolWidget::CreateSchema()
{
    using namespace FloorPlanSchema;
    WbSchema floorPlanSchema( CreateWorkbenchSubSchema( schemaName,
                                                        tr( "Floor Plan" ) ) );

    floorPlanSchema.AddKeyGroup( calGridGroup,
                                 WbSchemaElement::Multiplicity::One,
                                 KeyNameList() << calGridRowsKey
                                               << calGridColsKey,
                                 DefaultValueMap().WithDefault( calGridRowsKey,
                                                                KeyValue::from( 4 ) )
                                                  .WithDefault( calGridColsKey,
                                                                KeyValue::from( 6 ) ) );

    floorPlanSchema.AddKeyGroup( mappingGroup,
                                 WbSchemaElement::Multiplicity::One,

                                 KeyNameList() << camera1IdKey
                                               << camera2IdKey
                                               << camera1ImgKey
                                               << camera2ImgKey
                                               << homographyKey );

    floorPlanSchema.AddKeyGroup( transformGroup,
                                 WbSchemaElement::Multiplicity::One,
                                 KeyNameList() << cameraIdKey
                                               << transformKey );

    return floorPlanSchema;
}

void CreateFloorPlanToolWidget::CameraComboChanged()

{
    const KeyId camera1Id = KeyId(GetCamera1Id());
    const KeyId camera2Id = KeyId(GetCamera2Id());

    if ( camera1Id != camera2Id )
    {
         m_ui->m_getImageFromFileBtn->setEnabled(true);
         m_ui->m_captureLiveBtn->setEnabled(true);
    }
    else
    {
         m_ui->m_getImageFromFileBtn->setEnabled(false);
         m_ui->m_captureLiveBtn->setEnabled(false);
    }
}

void CreateFloorPlanToolWidget::ShowImage( IplImage* img, ImageView* view )
{
    // Convert image...
    IplImage* imgTmp = cvCreateImage( cvSize( img->width, img->height ), IPL_DEPTH_8U, 3 );
    cvConvertImage( img, imgTmp );

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

bool CreateFloorPlanToolWidget::LoadFile( KeyId cameraPosition, IplImage** camImg, QString fileName, CvPoint2D32f* offset )
{
    bool successful = true;

    // Get configuration information...
    Collection camerasCollection( CamerasCollection() );
    Collection cameraPositionsCollection( CameraPositionsCollection() );

    camerasCollection.SetConfig( GetCurrentConfig() );
    cameraPositionsCollection.SetConfig( GetCurrentConfig() );

    const KeyId camPosId = cameraPosition;

    LOG_INFO(QObject::tr("Camera position id: %1").arg(camPosId));

    const WbConfig camPosConfig = cameraPositionsCollection.ElementById( camPosId );

    if (camPosConfig.IsNull()) successful = false;

    CvMat* cameraMtx = cvCreateMat( 3, 3, CV_32F );
    CvMat* distortionCoeffs = cvCreateMat( 5, 1, CV_32F );
    CvMat* inverseCoeffs = cvCreateMat( 5, 1, CV_32F );

    CvMat* rot = cvCreateMat( 3, 3, CV_32F );
    CvMat* trans = cvCreateMat( 1, 3, CV_32F );

    if (successful)
    {
        const KeyId camId = camPosConfig.GetKeyValue(CameraPositionSchema::cameraIdKey).ToKeyId();

        LOG_INFO(QObject::tr("Camera id: %1").arg(camId));

        WbConfig cameraConfig = camerasCollection.ElementById( camId );

        if (cameraConfig.IsNull()) successful = false;

        if (successful)
        {
            // Intrinsic Parameters...
            const WbConfig cameraIntrisicConfig( cameraConfig.GetSubConfig( CalibrationSchema::schemaName ) );

            if (cameraIntrisicConfig.IsNull()) successful = false;

            if (successful)
            {
                const bool calibrationWasSuccessful = cameraIntrisicConfig
                                    .GetKeyValue( CalibrationSchema::calibrationSuccessfulKey )
                                    .ToBool();

                const bool cameraMtxValid = cameraIntrisicConfig
                                .GetKeyValue( CalibrationSchema::cameraMatrixKey )
                                .ToCvMat( *cameraMtx );
                const bool distortionCoeffsValid = cameraIntrisicConfig
                                .GetKeyValue( CalibrationSchema::distortionCoefficientsKey )
                                .ToCvMat( *distortionCoeffs );
                const bool inverseCoeffsValid = cameraIntrisicConfig
                                .GetKeyValue( CalibrationSchema::invDistortionCoefficientsKey )
                                .ToCvMat( *inverseCoeffs );

                successful = calibrationWasSuccessful &&
                             cameraMtxValid &&
                             distortionCoeffsValid &&
                             inverseCoeffsValid;
            }

            // Extrinsic Parameters...
            const WbConfig cameraExtrisicConfig(camPosConfig.GetSubConfig(ExtrinsicCalibrationSchema::schemaName));

            if (cameraExtrisicConfig.IsNull()) successful = false;

            if (successful)
            {
                const bool rotMatValid = cameraExtrisicConfig
                                .GetKeyValue( ExtrinsicCalibrationSchema::rotationMatrixKey )
                                .ToCvMat( *rot );
                const bool transValid = cameraExtrisicConfig
                                .GetKeyValue( ExtrinsicCalibrationSchema::translationKey )
                                .ToCvMat( *trans );

                successful = rotMatValid &&
                             transValid;
            }
        }
    }

    if (successful)
    {
        // Load file...
        IplImage* imgGrey = cvLoadImage( fileName.toAscii(), CV_LOAD_IMAGE_GRAYSCALE );

        *camImg = GroundPlaneUtility::unwarpGroundPlane( imgGrey,
                                                         cameraMtx,
                                                         distortionCoeffs,
                                                         inverseCoeffs,
                                                         rot,
                                                         trans,
                                                         offset );

        cvReleaseImage( &imgGrey );
    }

    cvReleaseMat( &cameraMtx );
    cvReleaseMat( &distortionCoeffs );
    cvReleaseMat( &inverseCoeffs );

    cvReleaseMat( &rot );
    cvReleaseMat( &trans );

    return successful;
}

void CreateFloorPlanToolWidget::BtnRotateClicked()
{
    m_rotAngle = (m_rotAngle + 90) % 360;

    // Rotate image...

#if LOSSY_ROTATION
    IplImage* imgCpy = cvCloneImage( m_cam2Img );

    CvPoint2D32f centre;
    centre.x = m_cam2Img->width / 2;
    centre.y = m_cam2Img->height / 2;

    CvMat* rotMat = cvCreateMat( 2, 3, CV_32F );
    cv2DRotationMatrix( centre, 90.0, 1.0, rotMat );

    cvWarpAffine( imgCpy, m_cam2Img, rotMat );

    cvReleaseImage( &imgCpy );

    cvReleaseMat( &rotMat );
#else
    cv::Mat imgCpy = rotateImage( m_cam2Img, 0, cv::Rect(0, 0, m_cam2Img->width, m_cam2Img->height), 90.0 );

    cvReleaseImage( &m_cam2Img );

    IplImage ipl_imgCpy  = IplImage(imgCpy);
    m_cam2Img = cvCloneImage( &ipl_imgCpy );
#endif

    ShowImage( m_cam2Img, m_ui->m_liveView2 );

    m_ui->btnMatch->setEnabled( true );
    m_ui->btnStitch->setEnabled( false );
    m_ui->btnSave->setEnabled( false );
}

void CreateFloorPlanToolWidget::BtnMatchClicked()
{
    using namespace FloorPlanSchema;

    std::vector< cv::Point2f > imagePoints1;
    std::vector< cv::Point2f > imagePoints2;

    cv::Size gridSize = cv::Size( GetCurrentConfig().GetKeyValue( calGridColsKey ).ToInt(),
                                  GetCurrentConfig().GetKeyValue( calGridRowsKey ).ToInt() );

    const bool foundCorners1 = cv::findChessboardCorners( cv::Mat( m_cam1Img ),
                                                          gridSize,
                                                          imagePoints1,
                                                          CV_CALIB_CB_ADAPTIVE_THRESH );

    const bool foundCorners2 = cv::findChessboardCorners( cv::Mat( m_cam2Img ),
                                                          gridSize,
                                                          imagePoints2,
                                                          CV_CALIB_CB_ADAPTIVE_THRESH );

    if (foundCorners1 && foundCorners2)
    {
        DisplayMatched( imagePoints1, imagePoints2 );

        m_ui->btnStitch->setEnabled(true);
    }
}

void CreateFloorPlanToolWidget::DisplayMatched( std::vector< cv::Point2f > ip1,
                                                std::vector< cv::Point2f > ip2 )
{
    using namespace FloorPlanSchema;

    cv::Size gridSize = cv::Size( GetCurrentConfig().GetKeyValue( calGridColsKey ).ToInt(),
                                  GetCurrentConfig().GetKeyValue( calGridRowsKey ).ToInt() );

    // Draw matches...
    std::vector<cv::KeyPoint> keypoints1(gridSize.width * gridSize.height);
    std::vector<cv::KeyPoint> keypoints2(gridSize.width * gridSize.height);

#if SHOW_ALL_MATCHES
    std::vector<cv::DMatch> matches(gridSize.width * gridSize.height);
#else
    std::vector<cv::DMatch> matches(1);
#endif

    for ( int p=0; p<gridSize.width*gridSize.height; p++ )
    {
        keypoints1.at(p) = cv::KeyPoint(ip1.at(p), 1.0);
        keypoints2.at(p) = cv::KeyPoint(ip2.at(p), 1.0);

#if SHOW_ALL_MATCHES
        matches.at(p) = cv::DMatch(p, p, 1);
#else
        matches.at(0) = cv::DMatch(0, 0, 1);
#endif
    }

    cv::Mat img_composite;

    cv::Mat frm_1 = cv::Mat( m_cam1Img );
    cv::Mat frm_2 = cv::Mat( m_cam2Img );

    cv::drawMatches( frm_1, keypoints1,
                     frm_2, keypoints2,
                     matches,
                     img_composite,
                     cv::Scalar::all(-1),
                     cv::Scalar::all(-1),
                     std::vector<char>(),
#if SHOW_ALL_MATCHES
                     cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );
#else
                     cv::DrawMatchesFlags::DEFAULT );
#endif

    IplImage img_comp = IplImage(img_composite);
    ImageViewer(&img_comp, this).exec();
}

void CreateFloorPlanToolWidget::BtnStitchClicked()
{
    using namespace FloorPlanSchema;

    std::vector< cv::Point2f > imagePoints1;
    std::vector< cv::Point2f > imagePoints2;

    cv::Size gridSize = cv::Size( GetCurrentConfig().GetKeyValue( calGridColsKey ).ToInt(),
                                  GetCurrentConfig().GetKeyValue( calGridRowsKey ).ToInt() );

    const bool foundCorners1 = cv::findChessboardCorners( cv::Mat( m_cam1Img ),
                                                          gridSize,
                                                          imagePoints1,
                                                          CV_CALIB_CB_ADAPTIVE_THRESH );

    const bool foundCorners2 = cv::findChessboardCorners( cv::Mat( m_cam2Img ),
                                                          gridSize,
                                                          imagePoints2,
                                                          CV_CALIB_CB_ADAPTIVE_THRESH );

    if (foundCorners1 && foundCorners2)
    {
        // Calculate the homography...
        m_homography = cv::findHomography( cv::Mat(imagePoints2), cv::Mat(imagePoints1), 0 );

        DisplayStitched();

        m_ui->btnSave->setEnabled(true);
    }
    else
    {
        Message::Show( this,
                       tr( "Create Floor Plan" ),
                       tr( "Error - Cannot find chessboard!" ),
                       Message::Severity_Critical );
    }
}

void CreateFloorPlanToolWidget::DisplayStitched()
{
    // Stitch the two images together...
    const IplImage* rootImg = m_cam1Img;

    CvMat* I = cvCreateMat( 3, 3, CV_32F );
    cvSetIdentity(I);

    GroundPlaneUtility::Rect32f cmpBox;
    cmpBox.pos.x = 0;
    cmpBox.pos.y = 0;
    cmpBox.dim.x = (float)m_cam1Img->width;
    cmpBox.dim.y = (float)m_cam1Img->height;

    CvMat matH = m_homography;

    GroundPlaneUtility::compositeImageBoundingBox( &matH, m_cam2Img, &cmpBox );

    // Now we know the dimensions and origin of the composite
    // image so we can allocate it and transform all images.
    int sizex = (int)(cmpBox.dim.x+.5f);
    int sizey = (int)(cmpBox.dim.y+.5f);

    sizex += 4 -( sizex % 4 );

    CvSize cmpSize = cvSize( sizex,sizey );
    IplImage *compImg = cvCreateImage( cmpSize, rootImg->depth, rootImg->nChannels );

    // Align image.
    cvZero( compImg );
    GroundPlaneUtility::alignGroundPlane( &matH, m_cam2Img, compImg );
    IplImage *img1 = cvCloneImage( compImg );

    // Align root.
    cvZero( compImg );
    GroundPlaneUtility::alignGroundPlane( I, m_cam1Img, compImg );
    IplImage *img2 = cvCloneImage( compImg );

    GroundPlaneUtility::createCompositeImage( img1, compImg, compImg );
    GroundPlaneUtility::createCompositeImage( img2, compImg, compImg );

    ImageViewer(compImg, this).exec();

    cvReleaseImage(&img1);
    cvReleaseImage(&img2);
    cvReleaseImage(&compImg);

    cvReleaseMat(&I);
}

void CreateFloorPlanToolWidget::BtnSaveClicked()
{
    WbConfig config = GetCurrentConfig();

    bool duplicate = false;

    const WbKeyValues::ValueIdPairList cameraMappingIds = config.GetKeyValues( FloorPlanSchema::camera2IdKey );

    for (WbKeyValues::ValueIdPairList::const_iterator it = cameraMappingIds.begin(); it != cameraMappingIds.end(); ++it)
    {
        if (it->value == KeyValue::from( m_camPosId2 ))
        {
            duplicate = true;
        }
    }

    if (!duplicate)
    {
        CvPoint2D32f centre;
        centre.x = m_cam2Img->width / 2;
        centre.y = m_cam2Img->height / 2;

        float t[9];
        CvMat tH = cvMat(3,3,CV_32F,t);

        float r[9];
        CvMat rotH = cvMat(3,3,CV_32F,r);

        cvSetIdentity(&rotH);

        CvMat* rotMat = cvCreateMat(2, 3, CV_32F);

        cv2DRotationMatrix(centre, m_rotAngle, 1.0, rotMat);

        cvmSet(&rotH,0,0, cvmGet(rotMat,0,0));
        cvmSet(&rotH,0,1, cvmGet(rotMat,0,1));
        cvmSet(&rotH,0,2, cvmGet(rotMat,0,2));
        cvmSet(&rotH,1,0, cvmGet(rotMat,1,0));
        cvmSet(&rotH,1,1, cvmGet(rotMat,1,1));
        cvmSet(&rotH,1,2, cvmGet(rotMat,1,2));

        float h[9];
        CvMat xH = cvMat(3,3,CV_32F,h);

        h[0] = m_homography.at<double>(0,0);
        h[1] = m_homography.at<double>(0,1);
        h[2] = m_homography.at<double>(0,2);
        h[3] = m_homography.at<double>(1,0);
        h[4] = m_homography.at<double>(1,1);
        h[5] = m_homography.at<double>(1,2);
        h[6] = m_homography.at<double>(2,0);
        h[7] = m_homography.at<double>(2,1);
        h[8] = m_homography.at<double>(2,2);

	    cvMatMul(&xH, &rotH, &tH);

        const KeyId mappingKeyId = config.AddKeyValue( FloorPlanSchema::homographyKey,
                                                       KeyValue::from( tH ) );

        config.SetKeyValue( FloorPlanSchema::camera1IdKey,
                            KeyValue::from( m_camPosId1 ),
                            mappingKeyId );

        config.SetKeyValue( FloorPlanSchema::camera2IdKey,
                            KeyValue::from( m_camPosId2 ),
                            mappingKeyId );

        WbConfigTools::SetFileName( config,
                                    m_camera1FileName,
                                    FloorPlanSchema::camera1ImgKey,
                                    WbConfigTools::FileNameMode_RelativeInsideWorkbench,
                                    mappingKeyId );

        WbConfigTools::SetFileName( config,
                                    m_camera2FileName,
                                    FloorPlanSchema::camera2ImgKey,
                                    WbConfigTools::FileNameMode_RelativeInsideWorkbench,
                                    mappingKeyId );

        cvReleaseMat( &rotMat );

        ReloadCurrentConfig();

        ResetUi();
    }
    else
	{
        Message::Show( this,
                       tr( "Create Floor Plan" ),
                       tr( "Error - Camera already mapped!" ),
                       Message::Severity_Critical );
	}
}

void CreateFloorPlanToolWidget::BtnCancelClicked()
{
    ResetUi();
}

void CreateFloorPlanToolWidget::BtnCreateFloorPlanClicked()
{
    using namespace FloorPlanSchema;

    WbConfig config = GetCurrentConfig();

    // Remove all existing transforms...
    config.RemoveOldKeys( FloorPlanSchema::cameraIdKey );
    config.RemoveOldKeys( FloorPlanSchema::transformKey );

    const WbConfig roomLayoutConfig(config.GetParent().GetSubConfig( RoomLayoutSchema::schemaName ) );
    const QStringList cameraPositionIds(roomLayoutConfig
                                        .GetKeyValue(RoomLayoutSchema::cameraPositionIdsKey)
                                        .ToQStringList() );

    if (cameraPositionIds.size() > 0)
    {
        if (cameraPositionIds.size() > 1)
        {
            CreateFloorPlanMulti();
        }
        else
        {
            CreateFloorPlanSingle();
        }
    }
}

void CreateFloorPlanToolWidget::CreateFloorPlanSingle()
{
    WbConfig config = GetCurrentConfig();

    Collection m_camPositions = CameraPositionsCollection();
    m_camPositions.SetConfig( config );

    const WbConfig roomLayoutConfig(config.GetParent().GetSubConfig( RoomLayoutSchema::schemaName ) );
    const KeyId camPosId = roomLayoutConfig.GetKeyValue( RoomLayoutSchema::cameraPositionIdsKey ).ToKeyId();

    const WbConfig camPosCfg = m_camPositions.ElementById( camPosId );

    const WbConfig camPosCalCfg = camPosCfg.GetSubConfig( ExtrinsicCalibrationSchema::schemaName );
    const KeyValue imgKey = camPosCalCfg.GetKeyValue( ExtrinsicCalibrationSchema::calibrationImageKey );

    const QString imgFile(camPosCalCfg.GetAbsoluteFileNameFor(imgKey.ToQString()));

    IplImage* imgComposite;

    CvPoint2D32f offset;

    LOG_TRACE(QObject::tr("Creating (single) floor plan for %1...").arg(camPosId));

    if (LoadFile( camPosId, &imgComposite, imgFile, &offset ))
    {
        LOG_INFO(QObject::tr("Loading image %1.").arg(imgFile));

        // Convert image...
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
            config.GetAbsoluteFileNameFor( "floor_plan.png" ) );

        if (!qImg.save( fileName ))
        {
            Message::Show( this,
                           tr( "Create Floor Plan" ),
                           tr( "Error - Cannot write to: %1" )
                               .arg( fileName ),
                           Message::Severity_Critical );
        }

        cvReleaseImage( &imgComposite );

        // show floor plan
        ImageViewer(imgTmp, this).exec();
        cvReleaseImage( &imgTmp );
        Message::Show( this,
                       tr( "Floor Plan Information" ),
                       tr( "Single-position floor plan successfully created.\n\nPlease Create a floor mask if you wish to calculate coverage." ),
                       Message::Severity_Information );
    }
    else
    {
        Message::Show( this,
                       tr( "Create Floor Plan" ),
                       tr( "Error - Cannot load from: %1!" )
                           .arg( imgFile ),
                       Message::Severity_Critical );
    }

    CvMat* identity = cvCreateMat( 3, 3, CV_32F );

    cvSetIdentity( identity );

    // Store transform for root camera...
    const KeyId transformId = config.AddKeyValue( FloorPlanSchema::transformKey,
                                                  KeyValue::from( *identity ) );

    config.SetKeyValue( FloorPlanSchema::cameraIdKey,
                        KeyValue::from(camPosId),
                        transformId );

    LOG_TRACE("Done.");

    cvReleaseMat( &identity );
}

void CreateFloorPlanToolWidget::CreateFloorPlanMulti()
{
    LOG_TRACE("Creating (multi) floor plan...");

    LOG_TRACE("Checking mapping...");

    // Check camera mapping(s)...
    if (CheckMappingIsComplete())
    {
        LOG_TRACE("Finding root...");

        // Find the base camera(s)...
        std::vector<KeyId> rootCamera = FindRoot();

        if (rootCamera.size() == 1)
        {
            LOG_INFO(QObject::tr("Checking root mapping for %1.").arg(rootCamera.front()));

            // Find a path from camera to the base...
            if (CheckRootMapping(rootCamera.front()))
            {
                LOG_INFO(QObject::tr("Compositing images with %1.").arg(rootCamera.front()));

                Stitch(rootCamera.front());
            }
            else
            {
                Message::Show( this,
                               tr( "Create Floor Plan" ),
                               tr( "Error - Mapping is incomplete!" ),
                               Message::Severity_Critical );
            }
        }
        else
        {
            Message::Show( this,
                           tr( "Create Floor Plan" ),
                           tr( "Error - Need one root camera!" ),
                           Message::Severity_Critical );
        }
    }
    else
    {
        Message::Show( this,
                       tr( "Create Floor Plan" ),
                       tr( "Error - Must map every camera!" ),
                       Message::Severity_Critical );
    }

    LOG_TRACE("Done.");
}

bool CreateFloorPlanToolWidget::CheckMappingIsComplete()
{
    bool allMapped = true;

    WbConfig config = GetCurrentConfig();

    // for each camera,
    //    for each mapping,
    //       if camera1 = camera OR camera2 = camera
    //          found = true;
    //          break;
    //    if !found
    //       break
    // return found

    const WbConfig roomLayoutConfig(config.GetParent().GetSubConfig( RoomLayoutSchema::schemaName ) );
    const QStringList cameraPositionIds(roomLayoutConfig
                                        .GetKeyValue(RoomLayoutSchema::cameraPositionIdsKey)
                                        .ToQStringList() );

    const WbKeyValues::ValueIdPairList cameraMappingIds = config.GetKeyValues( FloorPlanSchema::homographyKey );

    for ( int n = 0; n < cameraPositionIds.size(); ++n )
    {
        const KeyId camPosId = cameraPositionIds.at( n );

        bool found = false;

        LOG_INFO(QObject::tr("Checking mapping for %1.").arg(camPosId));

        for (WbKeyValues::ValueIdPairList::const_iterator it = cameraMappingIds.begin(); it != cameraMappingIds.end(); ++it)
        {
            const KeyId camera1Id( config.GetKeyValue( FloorPlanSchema::camera1IdKey, it->id ).ToKeyId() );
            const KeyId camera2Id( config.GetKeyValue( FloorPlanSchema::camera2IdKey, it->id ).ToKeyId() );

            if ((camPosId == camera1Id) || (camPosId == camera2Id))
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            allMapped = false;
            break;
        }
    }

    return allMapped;
}

std::vector<KeyId> CreateFloorPlanToolWidget::FindRoot()
{
    std::vector<KeyId> rootCamera;

    WbConfig config = GetCurrentConfig();

    // for each camera,
    //    for each mapping,
    //       if camera2 == camera
    //          root = false
    //    if root
    //       add to set

    const WbConfig roomLayoutConfig(config.GetParent().GetSubConfig( RoomLayoutSchema::schemaName ) );
    const QStringList cameraPositionIds(roomLayoutConfig
                                        .GetKeyValue(RoomLayoutSchema::cameraPositionIdsKey)
                                        .ToQStringList() );

    const WbKeyValues::ValueIdPairList cameraMappingIds = config.GetKeyValues( FloorPlanSchema::homographyKey );

    for ( int n = 0; n < cameraPositionIds.size(); ++n )
    {
        const KeyId camPosId = cameraPositionIds.at( n );

        bool root = true;

        for (WbKeyValues::ValueIdPairList::const_iterator it = cameraMappingIds.begin(); it != cameraMappingIds.end(); ++it)
        {
            const KeyId camera1Id( config.GetKeyValue( FloorPlanSchema::camera1IdKey, it->id ).ToKeyId() );
            const KeyId camera2Id( config.GetKeyValue( FloorPlanSchema::camera2IdKey, it->id ).ToKeyId() );

            if (camPosId == camera2Id)
            {
                root = false;
                break;
            }
        }

        if (root)
        {
            rootCamera.push_back(camPosId);
        }
    }

    return rootCamera;
}

std::vector<KeyId> CreateFloorPlanToolWidget::FindChain(KeyId camId, KeyId rootId, std::vector<KeyId> mappingChain)
{
    WbConfig config = GetCurrentConfig();

    const WbKeyValues::ValueIdPairList cameraMappingIds = config.GetKeyValues( FloorPlanSchema::homographyKey );

    for (WbKeyValues::ValueIdPairList::const_iterator it = cameraMappingIds.begin(); it != cameraMappingIds.end(); ++it)
    {
        const KeyId camera1Id( config.GetKeyValue( FloorPlanSchema::camera1IdKey, it->id ).ToKeyId() );
        const KeyId camera2Id( config.GetKeyValue( FloorPlanSchema::camera2IdKey, it->id ).ToKeyId() );

        LOG_INFO(QObject::tr("Camera1 id = %1.").arg(camera1Id));
        LOG_INFO(QObject::tr("Camera2 id = %1.").arg(camera2Id));

        if (camId == camera2Id)
        {
            if (std::find(mappingChain.begin(), mappingChain.end(), camera1Id) == mappingChain.end())
            {
                mappingChain.push_back(camera1Id);

                if (camera1Id != rootId)
                {
                    LOG_INFO(QObject::tr("Find chain for %1 - %2.").arg(camera1Id)
                                                                   .arg(rootId));

                    mappingChain = FindChain(camera1Id, rootId, mappingChain);
                }

                if (mappingChain.back() == rootId)
                {
                    LOG_INFO("Found.");

                    break;
                }
                else
                {
                    mappingChain.pop_back();
                }
            }
        }
    }

    return mappingChain;
}

bool CreateFloorPlanToolWidget::CheckRootMapping(KeyId rootId)
{
    bool allMapped = true;

    WbConfig config = GetCurrentConfig();

    // for each camera in root,
    //   for each camera
    //      if camera /= root
    //         if !FindChain (camera, root)
    //            ... = false;
    //            break;
    //   if found
    //      theRoot = root
    //      break
    //   else...

    const WbConfig roomLayoutConfig(config.GetParent().GetSubConfig( RoomLayoutSchema::schemaName ) );
    const QStringList cameraPositionIds(roomLayoutConfig
                                        .GetKeyValue(RoomLayoutSchema::cameraPositionIdsKey)
                                        .ToQStringList() );

    const WbKeyValues::ValueIdPairList cameraMappingIds = config.GetKeyValues( FloorPlanSchema::homographyKey );
    for ( int n = 0; n < cameraPositionIds.size(); ++n )
    {
        const KeyId camPosId = cameraPositionIds.at( n );

        if (camPosId != rootId)
        {
            LOG_INFO(QObject::tr("Find chain for %1 - %2.").arg(camPosId)
                                                           .arg(rootId));

            std::vector<KeyId> chain = FindChain(camPosId, rootId, std::vector<KeyId>());

            if (chain.size() == 0)
            {
                LOG_INFO("Not found.");

                allMapped = false;
                break;
            }
        }
    }

    return allMapped;
}

void CreateFloorPlanToolWidget::ComputeTransform(KeyId refId, std::vector<KeyId> chain, CvMat* transform)
{
    WbConfig config = GetCurrentConfig();

    const WbKeyValues::ValueIdPairList cameraMappingIds = config.GetKeyValues( FloorPlanSchema::homographyKey );

    for (std::vector<KeyId>::iterator elt = chain.begin(); elt != chain.end(); ++elt)
    {
        for (WbKeyValues::ValueIdPairList::const_iterator it = cameraMappingIds.begin(); it != cameraMappingIds.end(); ++it)
        {
            const KeyId camera1Id( config.GetKeyValue( FloorPlanSchema::camera1IdKey, it->id ).ToKeyId() );
            const KeyId camera2Id( config.GetKeyValue( FloorPlanSchema::camera2IdKey, it->id ).ToKeyId() );

            if ((camera1Id == *elt) && (camera2Id == refId))
            {
                CvMat* homography = cvCreateMat( 3, 3, CV_32F );

                const bool homographyValid = config
                                .GetKeyValue( FloorPlanSchema::homographyKey, it->id )
                                .ToCvMat( *homography );
                Q_UNUSED(homographyValid);

                CvMat* tmp = cvCreateMat( 3, 3, CV_32F );
                cvMatMul( transform, homography, tmp );

                cvmSet(transform,0,0, cvmGet(tmp,0,0));
                cvmSet(transform,0,1, cvmGet(tmp,0,1));
                cvmSet(transform,0,2, cvmGet(tmp,0,2));
                cvmSet(transform,1,0, cvmGet(tmp,1,0));
                cvmSet(transform,1,1, cvmGet(tmp,1,1));
                cvmSet(transform,1,2, cvmGet(tmp,1,2));
                cvmSet(transform,2,0, cvmGet(tmp,2,0));
                cvmSet(transform,2,1, cvmGet(tmp,2,1));
                cvmSet(transform,2,2, cvmGet(tmp,2,2));

                cvReleaseMat(&tmp);
                cvReleaseMat(&homography);

                refId = *elt;
            }
        }
    }
}

void CreateFloorPlanToolWidget::Stitch(KeyId camRoot)
{
    WbConfig config = GetCurrentConfig();

    // Create composite...
    IplImage *imgComposite;

    CvMat* transform = cvCreateMat( 3, 3, CV_32F );
    CvMat* identity = cvCreateMat( 3, 3, CV_32F );

    // Load root image...
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

    const QString rootFileName(config.GetAbsoluteFileNameFor(rootImage.ToQString()));

    LOG_INFO(QObject::tr("Root image: %1").arg(rootFileName));

    IplImage* rootImg;
    CvPoint2D32f offset;
    LoadFile( camRoot, &rootImg, rootFileName, &offset );

    cvSetIdentity(identity);

    // Store transform for root camera...
    const KeyId transformId = config.AddKeyValue( FloorPlanSchema::transformKey,
                                                  KeyValue::from( *identity ) );

    config.SetKeyValue( FloorPlanSchema::cameraIdKey,
                        KeyValue::from(camRoot),
                        transformId );

    GroundPlaneUtility::Rect32f cmpBox;
    cmpBox.pos.x = 0;
    cmpBox.pos.y = 0;
    cmpBox.dim.x = (float)rootImg->width;
    cmpBox.dim.y = (float)rootImg->height;

    LOG_INFO(QObject::tr("Composite bounding box is %1 %2 %3 %4.").arg(cmpBox.dim.x)
                                                                  .arg(cmpBox.dim.y)
                                                                  .arg(cmpBox.pos.x)
                                                                  .arg(cmpBox.pos.y));

    // Process remaining (non-root) cameras...
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

            std::vector<KeyId> chain = FindChain(camPosId, camRoot, std::vector<KeyId>());

            cvSetIdentity(transform);
            ComputeTransform( camPosId, chain, transform );

            // Store transform for camera...
            const KeyId transformId = config.AddKeyValue( FloorPlanSchema::transformKey,
                                                          KeyValue::from( *transform ) );

            config.SetKeyValue( FloorPlanSchema::cameraIdKey,
                                KeyValue::from(camPosId),
                                transformId );

            // Load camera image...
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

            const QString camFileName(config.GetAbsoluteFileNameFor(camImageFile.ToQString()));

            LOG_INFO(QObject::tr("Camera image: %1").arg(camFileName));

            IplImage* camImg;
            CvPoint2D32f offset;
            LoadFile( camPosId, &camImg, camFileName, &offset );

            // Stitch the images together...
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

    // Process remaining (non-root) cameras...
    for ( int i = 0; i < cameraPositionIds.size(); ++i )
    {
        const KeyId camPosId = cameraPositionIds.at( i );

        if (camPosId != camRoot)
        {
            std::vector<KeyId> chain = FindChain(camPosId, camRoot, std::vector<KeyId>());

            cvSetIdentity(transform);
            ComputeTransform( camPosId, chain, transform );

            // Load camera image...
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

            const QString camFileName(config.GetAbsoluteFileNameFor(camImageFile.ToQString()));

            LOG_INFO(QObject::tr("Camera filename: %1").arg(camFileName));

            IplImage* camImg;
            CvPoint2D32f offset;
            LoadFile( camPosId, &camImg, camFileName, &offset );

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

    // Convert image...
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
         config.GetAbsoluteFileNameFor( "floor_plan.png" ) );

    if (!qImg.save( fileName ))
    {
        Message::Show( this,
                       tr( "Create Floor Plan" ),
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
                   tr( "Floor Plan Information" ),
                   tr( "Multi-position floor plan successfully created.\n\nPlease Create a floor mask if you wish to calculate coverage." ),
                   Message::Severity_Information );
}

void CreateFloorPlanToolWidget::CaptureLiveBtnClicked()
{
    WbConfig config = GetCurrentConfig();

    m_camPosId1 = KeyId(GetCamera1Id());
    m_camPosId2 = KeyId(GetCamera2Id());

    m_ui->m_camera1Combo->setEnabled(false);
    m_ui->m_camera2Combo->setEnabled(false);

    m_ui->m_captureCancelBtn->setEnabled(true);

    m_ui->m_getImageFromFileBtn->setEnabled(false);

    m_ui->btnCancel->setEnabled(false);

    Collection camerasCollection( CamerasCollection() );
    Collection cameraPositionsCollection( CameraPositionsCollection() );

    camerasCollection.SetConfig( config );
    cameraPositionsCollection.SetConfig( config );


    const WbConfig camPosConfig1 = cameraPositionsCollection.ElementById( m_camPosId1 );
    const WbConfig camPosConfig2 = cameraPositionsCollection.ElementById( m_camPosId2 );

    const KeyId cameraId1( camPosConfig1.GetKeyValue(
                CameraPositionSchema::cameraIdKey )
                        .ToQString() );

    const KeyId cameraId2( camPosConfig2.GetKeyValue(
                CameraPositionSchema::cameraIdKey )
                        .ToQString() );

    const WbConfig cameraConfig1( camerasCollection.ElementById( cameraId1 ) );
    const WbConfig cameraConfig2( camerasCollection.ElementById( cameraId2 ) );

    const QString newFileNameFormat(
        config.GetAbsoluteFileNameFor( "calibrationImage/Calib%1.png" ) );

    bool capturedImages(
        m_captureLiveDualController->CaptureLiveBtnClicked(
                    cameraConfig1,
                    cameraConfig2,
                    newFileNameFormat,
#if defined(__MINGW32__) || defined(__GNUC__)
                    MakeCallback( this,
                                  &CreateFloorPlanToolWidget::GetStreamingView1),
                    MakeCallback( this,
                                  &CreateFloorPlanToolWidget::GetStreamingView2) ) );
#else
                    [this](const QSize& imageSize) -> ImageView*
                    {
                       return GetStreamingView1(imageSize);
                    },
                    [this](const QSize& imageSize) -> ImageView*
                    {
                       return GetStreamingView2(imageSize);
                    } ) );
#endif

     if (capturedImages)
     {
         CvPoint2D32f offset;

         m_camera1FileName = m_captureLiveDualController->CapturedFileName1();
         m_camera2FileName = m_captureLiveDualController->CapturedFileName2();

         if (LoadFile( m_camPosId1, &m_cam1Img, m_camera1FileName, &offset ))
         {
             ShowImage(m_cam1Img, m_ui->m_liveView1);
         }

         if (LoadFile( m_camPosId2, &m_cam2Img, m_camera2FileName, &offset ))
         {
             ShowImage(m_cam2Img, m_ui->m_liveView2);
         }

         m_ui->m_captureCancelBtn->setEnabled(false);

         m_ui->btnRotate->setEnabled(true);
         m_ui->btnMatch->setEnabled(true);

         m_ui->btnCancel->setEnabled(true);

         m_rotAngle = 0;
     }
}

void CreateFloorPlanToolWidget::CaptureCancelBtnClicked()
{
    m_ui->m_captureCancelBtn->setEnabled(false);

    m_ui->m_captureLiveBtn->setEnabled(true);
    m_ui->m_getImageFromFileBtn->setEnabled(true);

    m_ui->btnCancel->setEnabled(true);

    m_captureLiveDualController->CaptureCancelBtnClicked();
}

void CreateFloorPlanToolWidget::FromFileBtnClicked()
{
    WbConfig config = GetCurrentConfig();

    m_camPosId1 = KeyId(GetCamera1Id());
    m_camPosId2 = KeyId(GetCamera2Id());

    m_ui->m_camera1Combo->setEnabled(false);
    m_ui->m_camera2Combo->setEnabled(false);

    m_ui->m_captureLiveBtn->setEnabled(false);

    if ( !m_camPosId1.isEmpty() &&
         !m_camPosId2.isEmpty() )
    {
        FileDialogs::ExtendedFileDialog fileDialog( this,
                                                    tr( "Select Image File" ),
                                                    config.GetAbsoluteFileInfo().absolutePath(),
                                                    "Images( *.png *.jpg *.bmp *.ppm );;All Files( * )",
                                                    true );
        const int result1 = fileDialog.exec();
        if ( result1 == QFileDialog::Accepted )
        {
            m_camera1FileName = fileDialog.selectedFiles().front();

            if ( !m_camera1FileName.isEmpty() )
            {
                if ( FileUtilities::FileIsExternal( m_camera1FileName, config ) )
                {
                    if ( fileDialog.CopyFileSelected() )
                    {
                        const QString dstFile = QFileInfo( m_camera1FileName ).fileName();

                        const QString newImageName(
                            config.GetAbsoluteFileNameFor( "calibrationImage/" + dstFile ) );

                        QFile::copy( m_camera1FileName, newImageName );

                        m_camera1FileName = newImageName;
                    }
                }
            }
        }

        const int result2 = fileDialog.exec();
        if ( result2 == QFileDialog::Accepted )
        {
            m_camera2FileName = fileDialog.selectedFiles().front();

            if ( !m_camera2FileName.isEmpty() )
            {
                if ( FileUtilities::FileIsExternal( m_camera2FileName, config ) )
                {
                    if ( fileDialog.CopyFileSelected() )
                    {
                        const QString dstFile = QFileInfo( m_camera2FileName ).fileName();

                        const QString newImageName(
                            config.GetAbsoluteFileNameFor( "calibrationImage/" + dstFile ) );

                        QFile::copy( m_camera2FileName, newImageName );

                        m_camera2FileName = newImageName;
                    }
                }
            }
        }

        CvPoint2D32f offset;

        if ( !m_camera1FileName.isEmpty() &&
             !m_camera2FileName.isEmpty() )
        {
            if (LoadFile( m_camPosId1, &m_cam1Img, m_camera1FileName, &offset ))
            {
                ShowImage(m_cam1Img, m_ui->m_liveView1);
            }

            if (LoadFile( m_camPosId2, &m_cam2Img, m_camera2FileName, &offset ))
            {
                ShowImage(m_cam2Img, m_ui->m_liveView2);
            }

            m_ui->btnRotate->setEnabled(true);
            m_ui->btnMatch->setEnabled(true);

            m_rotAngle = 0;
        }
    }
}

