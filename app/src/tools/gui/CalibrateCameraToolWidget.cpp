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

#include "CalibrateCameraToolWidget.h"

#include "ui_CalibrateCameraToolWidget.h"

#include "CalibrationImageGridMapper.h"
#include "CalibrationImageTableMapper.h"
#include "VideoSource.h"
#include "CameraHardware.h"
#include "Message.h"
#include "CalibrationSchema.h"
#include "CalibrationAlgorithm.h"
#include "CaptureLiveBtnController.h"
#include "WbKeyValues.h"
#include "ImagePrintPreviewDlg.h"
#include "ChessboardImage.h"
#include "ButtonLabelMapper.h"
#include "PrintCalibrationGrid.h"
#include "UnknownLengthProgressDlg.h"
#include "CalibrateCameraResultsMapper.h"

#include <QtGui/QCheckBox>
#include <QtGui/QFileDialog>
#include <QtGui/QRegExpValidator>

#include "FileUtilities.h"
#include "FileDialogs.h"

#include "WbConfigTools.h"

#if defined(__MINGW32__) || defined(__GNUC__)
    #include <Callback.h>
#else
    #include <functional>
#endif

/** @todo add newly-captured files with relative file paths
 */
CalibrateCameraToolWidget::CalibrateCameraToolWidget( CameraHardware& cameraHardware,
                                                      QWidget* const parent ) :
    Tool          ( parent, CreateSchema() ),
    m_cameraHardware( cameraHardware ),
    m_captureLiveBtnController(),
    m_ui          ( new Ui::CalibrateCameraToolWidget ),
    m_imageGridMapper( 0 ),
    m_imageTableMapper( 0 )
{
    m_ui->setupUi( this );

    m_captureLiveBtnController.reset(
        new CaptureLiveBtnController( *m_ui->m_captureLiveBtn,
                                      *m_ui->m_captureCancelBtn, *this, m_cameraHardware ) );

    QHeaderView* horizHeader = m_ui->m_imagesTableWidget->horizontalHeader();
    horizHeader->setResizeMode( 0, QHeaderView::Stretch );
    horizHeader->setResizeMode( 1, QHeaderView::ResizeToContents );

    AddMapper( CalibrationSchema::gridSquareSizeInCmKey, m_ui->m_gridSquareSizeSpinBox );
    AddMapper( CalibrationSchema::gridRowsKey,       m_ui->m_gridRowsSpinBox );
    AddMapper( CalibrationSchema::gridColumnsKey,    m_ui->m_gridColumnsSpinBox );
    AddMapper( CalibrationSchema::noTangentialDistortionKey,
               m_ui->m_zeroTangentialCheckBox );
    AddMapper( CalibrationSchema::fixPrincipalPointKey,    m_ui->m_fixPrincipalPtCheckBox );
    AddMapper( CalibrationSchema::flipImagesKey,           m_ui->m_flipCheckBox );
    AddMapper( CalibrationSchema::fixedAspectRatioKey,     m_ui->m_aspectRatioLineEdit );
    AddMapper( CalibrationSchema::shouldFixAspectRatioKey, m_ui->m_fixAspectRatioGroup );

    AddMapper( new ButtonLabelMapper( *m_ui->m_printGridBtn,
                                      tr( "Print Grid" ),
                                      tr( " (%1x%2)" ),
                                      CalibrationSchema::gridRowsKey,
                                      CalibrationSchema::gridColumnsKey ) );

    m_imageTableMapper = new CalibrationImageTableMapper( *m_ui->m_imagesTableWidget );
    AddMapper( m_imageTableMapper );

    m_imageGridMapper = new CalibrationImageGridMapper( *m_ui->m_imageGrid );
    AddMapper( m_imageGridMapper );

    AddMapper( new CalibrateCameraResultsMapper( *m_ui->m_resultsTextBrowser ) );

    QDoubleValidator* validator = new QDoubleValidator;
    m_ui->m_aspectRatioLineEdit->setValidator( validator );

    m_ui->m_optionsTabs->setCurrentIndex( 0 );

    QObject::connect( m_ui->m_fromFileBtn,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( FromFileClicked() ) );
    QObject::connect( m_ui->m_captureLiveBtn,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( CaptureLiveBtnClicked() ) );
    QObject::connect( m_ui->m_captureCancelBtn,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( CaptureCancelBtnClicked() ) );
    QObject::connect( m_ui->m_calibrateBtn,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( CalibrateBtnClicked() ) );
    QObject::connect( m_ui->m_printGridBtn,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( PrintGridBtnClicked() ) );
    QObject::connect( m_ui->m_imagesTableWidget,
                      SIGNAL( currentItemChanged ( QTableWidgetItem*,
                                                   QTableWidgetItem* ) ),
                      this,
                      SLOT( ImageTableItemChanged( QTableWidgetItem*,
                                                   QTableWidgetItem* ) ) );
}

CalibrateCameraToolWidget::~CalibrateCameraToolWidget()
{
    delete m_ui;
}

const HelpBookmark CalibrateCameraToolWidget::GetHelpText() const
{
    return HELP_CALIBRATING_THE_CAMERA;
}

const QString CalibrateCameraToolWidget::GetSubSchemaDefaultFileName() const
{
    return "calibration.xml";
}

void CalibrateCameraToolWidget::ImageTableItemChanged(QTableWidgetItem* current,
                                                      QTableWidgetItem* previous)
{
    Q_UNUSED(previous);

    assert( m_imageGridMapper );
    if ( m_imageGridMapper && current )
    {
        QTableWidgetItem* currentRowNameItem =
            m_ui->m_imagesTableWidget->item(current->row(),
                                            CalibrationImageTableMapper::nameColumn );
        m_imageGridMapper->SetCurrentImage(
            currentRowNameItem->data(CalibrationImageTableMapper::idRoleOnName)
                                     .toString());
        ReloadCurrentConfig( m_imageTableMapper ); //must exclude table here to ensure it still has a "current row" for delete
    }
}

void CalibrateCameraToolWidget::FromFileClicked()
{
    // Make sure folder is there before adding file...
    const QString fileDirPath( GetCurrentConfig().GetAbsoluteFileNameFor( "calibrationImages/" ) );
    const bool mkPathSuccessful = QDir().mkpath( fileDirPath );

    if (!mkPathSuccessful)
    {
        Message::Show( this,
                       tr( "Camera Calibration Tool" ),
                       tr( "Error - Folder is missing!"),
                       Message::Severity_Critical );
        return;
    }

    // Display file selection dialog...
    FileDialogs::ExtendedFileDialog fileDialog( this,
                                                tr( "Select Image(s) to Add" ),
                                                GetCurrentConfig().GetAbsoluteFileInfo().absolutePath(),
                                                "Images( *.png *.jpg *.bmp *.ppm );;All Files( * )",
                                                false );
    const int result = fileDialog.exec();
    if ( result == QFileDialog::Accepted )
    {
        QStringList filesToOpen( fileDialog.selectedFiles() );

        foreach (QString imageName, filesToOpen)
        {
            WbConfigTools::FileNameMode mode = WbConfigTools::FileNameMode_RelativeInsideWorkbench;

            if ( FileUtilities::FileIsExternal( imageName, GetCurrentConfig() ) )
            {
                if ( fileDialog.CopyFileSelected() )
                {
                    const QString dstFile = QFileInfo( imageName ).fileName();

                    const QString newImageName(
                        GetCurrentConfig().GetAbsoluteFileNameFor( "calibrationImages/" + dstFile ) );

                    QFile::copy( imageName, newImageName );

                    imageName = newImageName;
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

            AddImageIfValid( imageName, mode );
        }
    }
}

void CalibrateCameraToolWidget::CalibrateBtnClicked()
{
    CalibrationAlgorithm alg;
    UnknownLengthProgressDlg* const progressDialog = new UnknownLengthProgressDlg( this );
    progressDialog->Start( tr( "Calibrating..." ), tr( "" ) );
    const bool calibrationSuccessful = alg.Run( GetCurrentConfig() );
    ReloadCurrentConfig();

    if ( calibrationSuccessful )
    {
        progressDialog->Complete( tr( "Camera Calibration Successful" ),
                                  tr( "The camera has been calibrated.\n"
                                      "Average reprojection error is: %1." )
                                      .arg( GetCurrentConfig()
                                            .GetKeyValue( CalibrationSchema::avgReprojectionErrorKey )
                                            .ToDouble() ) );
    }
    else
    {
        progressDialog->ForceClose();

        Message::Show( 0,
                       tr( "Camera Calibration Tool" ),
                       tr( "See the log for details!" ),
                       Message::Severity_Critical );
    }
}

void CalibrateCameraToolWidget::PrintGridBtnClicked()
{
    PrintCalibrationGrid( m_ui->m_gridRowsSpinBox->value()+1,
                          m_ui->m_gridColumnsSpinBox->value()+1 );
}

ImageView* const CalibrateCameraToolWidget::CreateStreamingView( const QSize& imageSize )
{
    m_ui->m_imageGrid->Clear();

    return m_ui->m_imageGrid->AddBlankImage( imageSize );
}

void CalibrateCameraToolWidget::ReloadCurrentConfigToolSpecific()
{
    const WbKeyValues::ValueIdPairList calibImages(
                GetCurrentConfig().GetKeyValues( CalibrationSchema::imageFileKey ) );

    m_ui->m_calibrateBtn->setEnabled( calibImages.size() >= 2 );
}

void CalibrateCameraToolWidget::AddImageIfValid( const QString& imageFileName,
                                                 const WbConfigTools::FileNameMode& mode )
{
    if ( !imageFileName.isEmpty() )
    {
        WbConfigTools::AddFileName( GetCurrentConfig(),
                                    imageFileName,
                                    CalibrationSchema::imageFileKey,
                                    mode );
        ReloadCurrentConfig();
    }
}

void CalibrateCameraToolWidget::CaptureLiveBtnClicked()
{
    WbConfig config( GetCurrentConfig() );
    const WbConfig cameraConfig( config.FindAncestor( KeyName( "camera" ) ) );

    const QString newFileNameFormat( config.GetAbsoluteFileNameFor( "calibrationImages/Calib%1.png" ) );

    const QString capturedImageFileName =
            m_captureLiveBtnController->CaptureLiveBtnClicked(cameraConfig,
                                                              newFileNameFormat,
#if defined(__MINGW32__) || defined(__GNUC__)
                                                              MakeCallback( this,
                                                                            &CalibrateCameraToolWidget::CreateStreamingView ) );
#else
															  [this](const QSize& imageSize) -> ImageView*
                                                              {
                                                                  return CreateStreamingView(imageSize);
                                                              } );
#endif

    AddImageIfValid( capturedImageFileName, WbConfigTools::FileNameMode_RelativeInsideWorkbench );
}

void CalibrateCameraToolWidget::CaptureCancelBtnClicked()
{
    m_ui->m_captureCancelBtn->setEnabled(false);
    m_ui->m_captureLiveBtn->setEnabled(true);
    m_captureLiveBtnController->CaptureCancelBtnClicked();
}

bool CalibrateCameraToolWidget::IsDataValid() const
{
    if (GetCurrentConfig().IsNull()) return true;

    bool valid = true;

    valid = valid &&
             !(m_ui->m_gridSquareSizeSpinBox->value() == 0.0);
    valid = valid &&
             !(m_ui->m_gridRowsSpinBox->value() == 0);
    valid = valid &&
             !(m_ui->m_gridColumnsSpinBox->value() == 0);

    return valid;
}

bool CalibrateCameraToolWidget::CanClose() const
{
    return IsDataValid() && !m_captureLiveBtnController->CurrentlyStreamingLiveSource();
}

const QString CalibrateCameraToolWidget::CannotCloseReason() const
{
    return tr("Please complete data or capture before leaving tab.");
}

const WbSchema CalibrateCameraToolWidget::CreateSchema()
{
    using namespace CalibrationSchema;
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

    schema.AddKeyGroup( imageGroup,
                        WbSchemaElement::Multiplicity::One,
                        KeyNameList() << imageFileKey
                                      << imageErrorKey
                                      << imageReprojectedPointsKey );

    schema.AddKeyGroup( advancedGroup,
                        WbSchemaElement::Multiplicity::One,
                        KeyNameList() << noTangentialDistortionKey
                                      << fixPrincipalPointKey
                                      << flipImagesKey
                                      << shouldFixAspectRatioKey
                                      << fixedAspectRatioKey,
                        DefaultValueMap().WithDefault( noTangentialDistortionKey,
                                                       KeyValue::from( true ) )
                                         .WithDefault( fixPrincipalPointKey,
                                                       KeyValue::from( false ) )
                                         .WithDefault( flipImagesKey,
                                                       KeyValue::from( false ) )
                                         .WithDefault( shouldFixAspectRatioKey,
                                                       KeyValue::from( false ) )
                                         .WithDefault( fixedAspectRatioKey,
                                                       KeyValue::from( 1.0 ) ));

    schema.AddKeyGroup( resultsGroup,
                        WbSchemaElement::Multiplicity::One,
                        KeyNameList() << calibrationSuccessfulKey
                                      << calibrationDateKey
                                      << calibrationTimeKey
                                      << rowsUsedForCalibrationKey
                                      << columnsUsedForCalibrationKey
                                      << imageHeightKey
                                      << imageWidthKey
                                      << cameraMatrixKey
                                      << distortionCoefficientsKey
                                      << invDistortionCoefficientsKey
                                      << avgReprojectionErrorKey );

    return schema;
}
