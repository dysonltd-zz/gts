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

#include "TargetsWidget.h"

#include "ui_TargetsWidget.h"

#include "TargetSchema.h"
#include "ImagePrintPreviewDlg.h"

#include "WbConfigTools.h"

#include "FileUtilities.h"
#include "FileDialogs.h"

#include "TargetRegistry.h"

#include "Message.h"

TargetsWidget::TargetsWidget( QWidget* parent ) :
    Tool( parent, CreateSchema() ),
    m_ui( new Ui::TargetsWidget )
{
    m_ui->setupUi( this );

	TargetRegistry::FillOutTargetTypeCombo( *m_ui->m_targetTypeComboBox );

    m_ui->m_imageView->SetConversionMethod( ImageView::SmoothConversion );

    using namespace TargetSchema;
    AddMapper( trackImgKey, m_ui->m_trackingTargetFileNameEdit );
    AddMapper( printImgKey, m_ui->m_printableTargetFileNameEdit );

    QObject::connect( m_ui->m_useStandard,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( UseStandardBtnClicked() ) );

    QObject::connect( m_ui->m_targetTypeComboBox,
                      SIGNAL( currentIndexChanged (int) ),
                      this,
                      SLOT( TargetTypeChanged() ) );

    QObject::connect( m_ui->m_browseTrackingBtn,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( BrowseTrackingBtnClicked() ) );
    QObject::connect( m_ui->m_clearTrackingBtn,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( ClearTrackingBtnClicked() ) );

    QObject::connect( m_ui->m_browsePrintableBtn,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( BrowsePrintableBtnClicked() ) );
    QObject::connect( m_ui->m_clearPrintableBtn,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( ClearPrintableBtnClicked() ) );

    QObject::connect( m_ui->m_printTargetBtn,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( PrintTargetBtnClicked() ) );
}

TargetsWidget::~TargetsWidget()
{
    delete m_ui;
}

bool TargetsWidget::IsDataValid() const
{
    if (GetCurrentConfig().IsNull()) return true;

    bool valid = true;

    valid = valid &&
             !(m_ui->m_trackingTargetFileNameEdit->text().isEmpty());
    valid = valid &&
             !(m_ui->m_printableTargetFileNameEdit->text().isEmpty());

	Tool::HighlightLabel(m_ui->m_trackingTargetFileNameLabel, !valid);
	Tool::HighlightLabel(m_ui->m_printableTargetFileNameLabel, !valid);

    return valid;
}

bool TargetsWidget::CanClose() const
{
    if ( !IsDataValid() )
        return false;

    return true;
}

const QString TargetsWidget::CannotCloseReason() const
{
    return tr("Please complete data before leaving tab.");
}

const WbSchema TargetsWidget::CreateSchema()
{
   using namespace TargetSchema;
   WbSchema schema( CreateWorkbenchSubSchema( schemaName, tr( "Params" ) ) );

    schema.AddKeyGroup( TargetSchema::targetGroup,
                        WbSchemaElement::Multiplicity::One,
                        KeyNameList() << TargetSchema::trackImgKey
                                      << TargetSchema::printImgKey );

    return schema;
}

const QString TargetsWidget::GetSubSchemaDefaultFileName() const
{
    return "params.xml";
}

void TargetsWidget::ReloadCurrentConfigToolSpecific()
{
    TargetImageChanged();
}

const QString TargetsWidget::GetSelectedTargetId() const
{
    QComboBox* const targetTypeCombo = m_ui->m_targetTypeComboBox;
    const int newTargetIndex = targetTypeCombo->currentIndex();
    const QString targetId( targetTypeCombo->itemData( newTargetIndex ).toString() );
    return targetId;
}

void TargetsWidget::UseStandardBtnClicked()
{
    if ( m_ui->m_useStandard->isChecked() )
	{
	    m_ui->m_targetTypeComboBox->setEnabled(true);

	    m_ui->m_browseTrackingBtn->setEnabled(false);
	    m_ui->m_clearTrackingBtn->setEnabled(false);

	    m_ui->m_browsePrintableBtn->setEnabled(false);
	    m_ui->m_clearPrintableBtn->setEnabled(false);

	    TargetTypeChanged();
	}
	else
	{
	    m_ui->m_targetTypeComboBox->setEnabled(false);

	    m_ui->m_browseTrackingBtn->setEnabled(true);
	    m_ui->m_clearTrackingBtn->setEnabled(true);

	    m_ui->m_browsePrintableBtn->setEnabled(true);
	    m_ui->m_clearPrintableBtn->setEnabled(true);

        m_ui->m_trackingTargetFileNameEdit->setText("");
        m_ui->m_printableTargetFileNameEdit->setText("");
	}
}

bool TargetsWidget::DirectoryExists( const QString& outputDirectoryName )
{
    QDir outputDirectory( outputDirectoryName );
    bool directoryExists = outputDirectory.exists();

    if ( !directoryExists )
    {
        directoryExists = QDir().mkpath( outputDirectory.absolutePath() );
    }

    return directoryExists;
}

void TargetsWidget::TargetTypeChanged()
{
    bool successful = true;

    const TargetRegistry::TargetDetails targetDetails( TargetRegistry::GetTargetById( GetSelectedTargetId() ) );

    const QImage trackImage( targetDetails.imageFileName );
    const QImage printImage( targetDetails.printImageFileName );

	QString trackImageName = targetDetails.imageFileName;
	QString printImageName = targetDetails.printImageFileName;

	trackImageName = trackImageName.remove(':');
	printImageName = printImageName.remove(':');

    if (DirectoryExists( GetCurrentConfig().GetAbsoluteFileNameFor("targetImages/") ))
    {
        const QString trackImageFile(
            GetCurrentConfig().GetAbsoluteFileNameFor( "targetImages/" + trackImageName ) );
        const QString printImageFile(
            GetCurrentConfig().GetAbsoluteFileNameFor( "targetImages/" + printImageName ) );

        successful = successful && trackImage.save( trackImageFile.toAscii().data() );
        successful = successful && printImage.save( printImageFile.toAscii().data() );

	    if ( successful )
	    {
            const QString relTrackImageFile =
                WbConfigTools::ConvertFileName( GetCurrentConfig(),
                                                trackImageFile,
                                                WbConfigTools::FileNameMode_RelativeInsideWorkbench,
                                                true );
            const QString relPrintImageFile =
                WbConfigTools::ConvertFileName( GetCurrentConfig(),
                                                printImageFile,
                                                WbConfigTools::FileNameMode_RelativeInsideWorkbench,
                                                true );

            m_ui->m_trackingTargetFileNameEdit->setText(relTrackImageFile);
            m_ui->m_printableTargetFileNameEdit->setText(relPrintImageFile);
	    }
        else
        {
            Message::Show( this,
                           tr( "Target Configuration Tool" ),
                           tr( "Error - Failed to save)!" ),
                           Message::Severity_Critical );
        }
    }
    else
    {
        Message::Show( this,
                       tr( "Target Configuration Tool" ),
                       tr( "Error - Save Workbench!" ),
                       Message::Severity_Critical );
    }
}

QString TargetsWidget::BrowseTargetImage()
{
    // Make sure folder is there before adding file...
    const QString fileDirPath( GetCurrentConfig().GetAbsoluteFileNameFor( "targetImage/" ) );
    const bool mkPathSuccessful = QDir().mkpath( fileDirPath );

    if (!mkPathSuccessful)
    {
        Message::Show( this,
                       tr( "Target Configuration Tool" ),
                       tr( "Error - Folder is missing!"),
                       Message::Severity_Critical );
        return QString();
    }

    QString relImageFile;

    // Display file selection dialog...
    FileDialogs::ExtendedFileDialog fileDialog( this,
                                                tr( "Select Target File" ),
                                                GetCurrentConfig().GetAbsoluteFileInfo().absolutePath(),
                                                "Images( *.png *.jpg *.bmp *.ppm );;All Files( * )",
                                                true );
    const int result = fileDialog.exec();
    if ( result == QFileDialog::Accepted )
    {
        QString imageName( fileDialog.selectedFiles().front() );

        if ( !imageName.isEmpty() )
        {
            WbConfigTools::FileNameMode mode = WbConfigTools::FileNameMode_RelativeInsideWorkbench;

            if ( FileUtilities::FileIsExternal( imageName, GetCurrentConfig() ) )
            {
                if ( fileDialog.CopyFileSelected() )
                {
                    const QString dstFile = QFileInfo( imageName ).fileName();

                    const QString newImageName(
                        GetCurrentConfig().GetAbsoluteFileNameFor( "targetImage/" + dstFile ) );

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

            relImageFile =
                WbConfigTools::ConvertFileName( GetCurrentConfig(),
                                                imageName,
                                                mode,
                                                true );
        }
    }

    return relImageFile;
}

void TargetsWidget::BrowseTrackingBtnClicked()
{
    const QString relImageFile = BrowseTargetImage();

    m_ui->m_trackingTargetFileNameEdit->setText(relImageFile);
}

void TargetsWidget::ClearTrackingBtnClicked()
{
    m_ui->m_trackingTargetFileNameEdit->setText("");
}

void TargetsWidget::BrowsePrintableBtnClicked()
{
    const QString relImageFile = BrowseTargetImage();

    m_ui->m_printableTargetFileNameEdit->setText(relImageFile);
}

void TargetsWidget::ClearPrintableBtnClicked()
{
    m_ui->m_printableTargetFileNameEdit->setText("");
}

void TargetsWidget::TargetImageChanged()
{
    const QString fileName = WbConfigTools::GetFileName( GetCurrentConfig(), TargetSchema::trackImgKey );

    if ( fileName.isEmpty() )
    {
        m_ui->m_imageView->Clear();
    }
    else
    {
        m_ui->m_imageView->SetImage( fileName );
    }

    m_ui->m_imageView->update();
}

void TargetsWidget::PrintTargetBtnClicked()
{
    const QString fileName = WbConfigTools::GetFileName( GetCurrentConfig(), TargetSchema::printImgKey );

    if ( !fileName.isEmpty() )
    {
        ImagePrintPreviewDlg printDlg( (QImage( fileName )) );
        printDlg.exec();
    }
}
