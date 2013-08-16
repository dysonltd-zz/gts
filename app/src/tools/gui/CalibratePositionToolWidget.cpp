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

#include "CalibratePositionToolWidget.h"

#include "ui_CalibratePositionToolWidget.h"

#include "CaptureLiveBtnController.h"
#include "CamerasCollection.h"
#include "CameraPositionSchema.h"
#include "CameraHardware.h"
#include "CalibrationImageViewMapper.h"
#include "ExtrinsicCalibrationSchema.h"
#include "ExtrinsicCalibrationAlgorithm.h"
#include "Message.h"
#include "ButtonLabelMapper.h"
#include "PrintCalibrationGrid.h"
#include "WbConfigTools.h"
#include "CalibratePositionResultsMapper.h"
#include "UnknownLengthProgressDlg.h"

#include "FileUtilities.h"
#include "FileDialogs.h"

#if defined(__MINGW32__) || defined(__GNUC__)
#include <Callback.h>
#else
#include <functional>
#endif

CalibratePositionToolWidget::CalibratePositionToolWidget( CameraHardware& cameraHardware,
                                                          QWidget* parent ) :
    Tool( parent, CreateSchema() ),
    m_ui( new Ui::CalibratePositionToolWidget ),
    m_captureLiveBtnController()
{
    m_ui->setupUi( this );

    m_imageView = m_ui->m_imageGrid->AddBlankImage( m_ui->m_imageGrid->size() );

    m_captureLiveBtnController.reset(
        new CaptureLiveBtnController( *m_ui->m_captureLiveBtn,
                                      *m_ui->m_captureCancelBtn, *this, cameraHardware ) );

    using namespace ExtrinsicCalibrationSchema;
    AddMapper( gridRowsKey, m_ui->m_gridRowsSpinBox );
    AddMapper( gridColumnsKey, m_ui->m_gridColumnsSpinBox );
    AddMapper( gridSquareSizeInCmKey, m_ui->m_gridSquareSizeSpinBox );

    m_viewMapper = new CalibrationImageViewMapper( *m_imageView );

    AddMapper( m_viewMapper );

    AddMapper( new ButtonLabelMapper( *m_ui->m_printGridBtn,
                                      tr( "Print Grid" ),
                                      tr( "(%1x%2)" ),
                                      ExtrinsicCalibrationSchema::gridRowsKey,
                                      ExtrinsicCalibrationSchema::gridColumnsKey ) );

    AddMapper( new CalibratePositionResultsMapper( *m_ui->m_resultsTextBrowser ) );

    QObject::connect( m_ui->m_getImageFromFileBtn,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( FromFileBtnClicked() ) );
    QObject::connect( m_ui->m_captureLiveBtn,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( CaptureLiveBtnClicked() ) );
    QObject::connect( m_ui->m_calibrateBtn,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( CalibrateBtnClicked() ) );
    QObject::connect( m_ui->m_printGridBtn,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( PrintGridBtnClicked() ) );
    QObject::connect( m_ui->m_showUnwarped,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( ShowUnwarpedBtnClicked() ) );
    QObject::connect( m_ui->m_captureCancelBtn,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( CaptureCancelBtnClicked() ) );
}

CalibratePositionToolWidget::~CalibratePositionToolWidget()
{
    delete m_ui;
}

void CalibratePositionToolWidget::ShowNoCameraError()
{
    Message::Show( this,
                   tr( "Calibrate Position" ),
                   tr( "Error - There is no camera selected!" ),
                   Message::Severity_Critical );
}

const KeyId CalibratePositionToolWidget::GetCameraIdToCapture() const
{
    const WbConfig calibratePositionConfig( GetCurrentConfig() );
    const WbConfig aConfig( calibratePositionConfig.GetParent() );
    const KeyId cameraIdToCapture( aConfig.GetKeyValue( CameraPositionSchema::cameraIdKey ).ToKeyId() );

    return cameraIdToCapture;
}

ImageView* const CalibratePositionToolWidget::GetStreamingView( const QSize& imageSize )
{
    Q_UNUSED(imageSize);

    return m_imageView;
}

void CalibratePositionToolWidget::CalibrateBtnClicked()
{
    ExtrinsicCalibrationAlgorithm alg;
    UnknownLengthProgressDlg* const progressDialog = new UnknownLengthProgressDlg( this );
    progressDialog->Start( tr( "Calibrating..." ), tr( "" ) );
    const bool calibrationSuccessful = alg.Run( GetCurrentConfig() );

    if ( calibrationSuccessful )
    {
        progressDialog->Complete( tr( "Position Calibration Successful" ),
                                  tr( "The position has been calibrated." ) );
    }
    else
    {
        progressDialog->ForceClose();

        Message::Show( 0,
                       tr( "Position Calibration Tool" ),
                       tr( "See the log for details!" ),
                       Message::Severity_Critical );
    }
}

void CalibratePositionToolWidget::SetCalibrationImage( const QString& imageName,
                                                       const WbConfigTools::FileNameMode& mode )
{
    if ( !imageName.isEmpty() )
    {
        WbConfigTools::SetFileName( GetCurrentConfig(),
                                    imageName,
                                    ExtrinsicCalibrationSchema::calibrationImageKey,
                                    mode );

        ReloadCurrentConfig();
    }
}

void CalibratePositionToolWidget::FromFileBtnClicked()
{
    // Make sure folder is there before adding file...
    const QString fileDirPath( GetCurrentConfig().GetAbsoluteFileNameFor( "calibrationImage/" ) );
    const bool mkPathSuccessful = QDir().mkpath( fileDirPath );

    if (!mkPathSuccessful)
    {
        Message::Show( this,
                       tr( "Position Calibration Tool" ),
                       tr( "Error - Folder is missing!"),
                       Message::Severity_Critical );
        return;
    }

    // Display file selection dialog...
    FileDialogs::ExtendedFileDialog fileDialog( this,
                                                tr( "Select Image File" ),
                                                GetCurrentConfig().GetAbsoluteFileInfo().absolutePath(),
                                                "Images( *.png *.jpg *.bmp *.ppm );;All Files( * )",
                                                true );
    const int result = fileDialog.exec();
    if ( result == QFileDialog::Accepted )
    {
        QString calImageName( fileDialog.selectedFiles().front() );

        if ( !calImageName.isEmpty() )
        {
            WbConfigTools::FileNameMode mode = WbConfigTools::FileNameMode_RelativeInsideWorkbench;

            if ( FileUtilities::FileIsExternal( calImageName, GetCurrentConfig() ) )
            {
                if ( fileDialog.CopyFileSelected() )
                {
                    const QString dstFile = QFileInfo( calImageName ).fileName();

                    const QString newImageName(
                        GetCurrentConfig().GetAbsoluteFileNameFor( "calibrationImage/" + dstFile ) );

                    QFile::copy( calImageName, newImageName );

                    calImageName = newImageName;
                }
                else
                {
                    if ( fileDialog.RelativeSelected() )
                    {
                        mode = WbConfigTools::FileNameMode_Relative;
                    }
                    else
                    {
                        mode = WbConfigTools::FileNameMode_Absolute;
                    }
                }
            }

            SetCalibrationImage( calImageName, mode );
        }
    }
}

void CalibratePositionToolWidget::CaptureLiveBtnClicked()
{
    Collection camerasCollection( CamerasCollection() );
    camerasCollection.SetConfig( GetCurrentConfig() );

    const KeyId cameraIdToCapture(GetCameraIdToCapture());
    if (cameraIdToCapture.isEmpty()) { ShowNoCameraError(); return; }

    const WbConfig cameraConfig( camerasCollection.ElementById( cameraIdToCapture ) );

     const QString newFileNameFormat(
         GetCurrentConfig().GetAbsoluteFileNameFor( "calibrationImage/Calib%1.png" ) );

     const QString capturedImage(
         m_captureLiveBtnController->CaptureLiveBtnClicked(
                     cameraConfig,
                     newFileNameFormat,
#if defined(__MINGW32__) || defined(__GNUC__)
                     MakeCallback( this,
                                   &CalibratePositionToolWidget::GetStreamingView ) ) );
#else
                     [this](const QSize& imageSize) -> ImageView*
                     {
                        return GetStreamingView(imageSize);
                     } ) );
#endif

     SetCalibrationImage( capturedImage, WbConfigTools::FileNameMode_RelativeInsideWorkbench );
}

void CalibratePositionToolWidget::CaptureCancelBtnClicked()
{
    m_ui->m_captureCancelBtn->setEnabled(false);

    m_ui->m_captureLiveBtn->setEnabled(true);
    m_ui->m_getImageFromFileBtn->setEnabled(true);

    m_captureLiveBtnController->CaptureCancelBtnClicked();
}

const HelpBookmark CalibratePositionToolWidget::GetHelpText() const
{
    return HELP_SETTING_UP_THE_CAMERA_POSITIONS;
}

const QString CalibratePositionToolWidget::GetSubSchemaDefaultFileName() const
{
    return "cameraPositionCalibration.xml";
}

void CalibratePositionToolWidget::PrintGridBtnClicked()
{
    PrintCalibrationGrid( m_ui->m_gridRowsSpinBox->value()+1,
                          m_ui->m_gridColumnsSpinBox->value()+1 );
}

void CalibratePositionToolWidget::ShowUnwarpedBtnClicked()
{
    m_viewMapper->ToggleWarping( m_ui->m_showUnwarped->isChecked() );
}

void CalibratePositionToolWidget::ReloadCurrentConfigToolSpecific()
{
    m_ui->m_calibrateBtn
        ->setEnabled( !GetCurrentConfig()
                      .GetKeyValue( ExtrinsicCalibrationSchema::calibrationImageKey )
                      .IsNull() );
}

const WbSchema CalibratePositionToolWidget::CreateSchema()
{
    using namespace ExtrinsicCalibrationSchema;
    WbSchema schema( CreateWorkbenchSubSchema( schemaName,
                                               tr( "Calibration" ) ) );

    schema.AddKeyGroup( gridGroup,
                        WbSchemaElement::Multiplicity::One,
                        KeyNameList() << gridSquareSizeInCmKey
                                      << gridRowsKey
                                      << gridColumnsKey,
                        DefaultValueMap().WithDefault( gridSquareSizeInCmKey,
                                                       KeyValue::from( 0.0 ) )
                                         .WithDefault( gridRowsKey,
                                                       KeyValue::from( 0 ) )
                                         .WithDefault( gridColumnsKey,
                                                       KeyValue::from( 0 ) ) );

    schema.AddSingleValueKey( calibrationImageKey, WbSchemaElement::Multiplicity::One );

    schema.AddKeyGroup( resultsGroup,
                        WbSchemaElement::Multiplicity::One,
                        KeyNameList() << calibrationSuccessfulKey
                                      << calibrationDateKey
                                      << calibrationTimeKey
                                      << rotationMatrixKey
                                      << translationKey
                                      << reprojectionErrorKey
                                      << gridSquareSizeInPxKey );
    return schema;
}

bool CalibratePositionToolWidget::IsDataValid() const
{
    if (GetCurrentConfig().IsNull()) return true;

    bool valid = true;

    valid = valid && !(GetCameraIdToCapture().isEmpty());

    valid = valid &&
             !(m_ui->m_gridSquareSizeSpinBox->value() == 0.0);
    valid = valid &&
             !(m_ui->m_gridRowsSpinBox->value() == 0);
    valid = valid &&
             !(m_ui->m_gridColumnsSpinBox->value() == 0);

    return valid;
}

bool CalibratePositionToolWidget::CanClose() const
{
    return IsDataValid() && !m_captureLiveBtnController->CurrentlyStreamingLiveSource();
}

const QString CalibratePositionToolWidget::CannotCloseReason() const
{
    return tr("Please complete data or capture before leaving tab.");
}

#if 0
void CalibratePositionToolWidget::BtnShowWarpGradientClicked()
{
    QImage m_image;

    const QString calibrationImageRelativeFileName(
        config.GetKeyValue( ExtrinsicCalibrationSchema::calibrationImageKey )
        .ToQString() );
    const QString calibrationImageAbsoluteFileName(
        config.GetAbsoluteFileNameFor( calibrationImageRelativeFileName ) );

    m_image = QImage(calibrationImageAbsoluteFileName).convertToFormat(QImage::Format_RGB888);

    cv::Mat imageMat(m_image.height(), m_image.width(), CV_8UC3, m_image.bits());

    // Compute undistortion maps for the ground plane
    // (these will be used to undistort entire sequence)
    *viewWarp = computeGroundPlaneWarpBatch( viewGrey,
                                             GetIntrinsicParams(),
                                             GetDistortionParams(),
                                             GetUndistortionParams(),
                                             GetRotationParams(),
                                             GetTranslationParams(),
                                             &m_mapx,
                                             &m_mapy,
                                             &m_offset );

    // Compute the warp gradients
    CvMat* m_mapDx = GradientMagCv32fc1( m_mapx );
    CvMat* m_mapDy = GradientMagCv32fc1( m_mapy );
    CvMat* m_mapGM = cvCloneMat( m_mapDy );

    cvAdd( m_mapDx, m_mapDy, m_mapGM );

    IplImage* magImg = VisualiseCvMatrix32fc1( m_mapGM );
    IplImage* tmp = LogNormaliseCvImage( magImg );
    cvReleaseImage(&magImg);
    magImg = tmp;
    tmp = 0;

    IplImage* img = cvLoadImage(fileName.toAscii(), CV_LOAD_IMAGE_GRAYSCALE);

    if (magImg)
    {
        ImageViewer(magImg, this).exec();

        cvReleaseImage( &magImg );
    }
    else
    {
        Message::Show( this,
                       tr( "Calibrate Position Tool" ),
                       tr( "Error - No floor plan found!"),
                       Message::Severity_Critical );
    }

    cvReleaseMat( &m_mapDx );
    cvReleaseMat( &m_mapDy );
    cvReleaseMat( &m_mapGM );
}
#endif
