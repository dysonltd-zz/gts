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

#include "TargetsToolWidget.h"

#include "ui_TargetsToolWidget.h"

#include "TargetSchema.h"
#include "ImagePrintPreviewDlg.h"

#include "WbConfigTools.h"

#include "FileNameUtils.h"
#include "FileDialogs.h"

#include "TargetRegistry.h"

#include "Message.h"

TargetsToolWidget::TargetsToolWidget( QWidget* parent ) :
    Tool( parent, CreateSchema() ),
    m_ui( new Ui::TargetsToolWidget )
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

TargetsToolWidget::~TargetsToolWidget()
{
    delete m_ui;
}

const WbSchema TargetsToolWidget::CreateSchema()
{
   using namespace TargetSchema;
   WbSchema schema( CreateWorkbenchSubSchema( schemaName, tr( "Params" ) ) );

    schema.AddKeyGroup( TargetSchema::targetGroup,
                        WbSchemaElement::Multiplicity::One,
                        KeyNameList() << TargetSchema::trackImgKey
                                      << TargetSchema::printImgKey );

    return schema;
}

const QString TargetsToolWidget::GetSubSchemaDefaultFileName() const
{
    return "params.xml";
}

void TargetsToolWidget::ReloadCurrentConfigToolSpecific()
{
    TargetImageChanged();
}

const QString TargetsToolWidget::GetSelectedTargetId() const
{
    QComboBox* const targetTypeCombo = m_ui->m_targetTypeComboBox;
    const int newTargetIndex = targetTypeCombo->currentIndex();
    const QString targetId( targetTypeCombo->itemData( newTargetIndex ).toString() );
    return targetId;
}

void TargetsToolWidget::UseStandardBtnClicked()
{
    if ( m_ui->m_useStandard->isChecked() )
	{
	    m_ui->m_targetTypeComboBox->setEnabled(true);

	    m_ui->m_browseTrackingBtn->setEnabled(false);
	    m_ui->m_clearTrackingBtn->setEnabled(false);

	    m_ui->m_browsePrintableBtn->setEnabled(false);
	    m_ui->m_clearPrintableBtn->setEnabled(false);
	}
	else
	{
	    m_ui->m_targetTypeComboBox->setEnabled(false);

	    m_ui->m_browseTrackingBtn->setEnabled(true);
	    m_ui->m_clearTrackingBtn->setEnabled(true);

	    m_ui->m_browsePrintableBtn->setEnabled(true);
	    m_ui->m_clearPrintableBtn->setEnabled(true);
	}
}


bool TargetsToolWidget::DirectoryExists( const QString& outputDirectoryName )
{
    QDir outputDirectory( outputDirectoryName );
    bool directoryExists = outputDirectory.exists();

    if ( !directoryExists )
    {
        directoryExists = QDir().mkpath( outputDirectory.absolutePath() );
    }

    return directoryExists;
}

void TargetsToolWidget::TargetTypeChanged()
{
    bool successful = true;

    const TargetRegistry::TargetDetails targetDetails( TargetRegistry::GetTargetById( GetSelectedTargetId() ) );

    const QImage trackImage( targetDetails.imageFileName );
    const QImage printImage( targetDetails.printImageFileName );

	QString trackImageName = targetDetails.imageFileName;
	QString printImageName = targetDetails.printImageFileName;

	trackImageName = trackImageName.remove(':');
	printImageName = printImageName.remove(':');

    if (DirectoryExists( GetCurrentConfig().GetAbsoluteFileNameFor("targetImage/") ))
    {
        const QString trackImageFile(
            GetCurrentConfig().GetAbsoluteFileNameFor( "targetImage/" + trackImageName ) );
        const QString printImageFile(
            GetCurrentConfig().GetAbsoluteFileNameFor( "targetImage/" + printImageName ) );

        successful = successful && trackImage.save( trackImageFile.toAscii().data() );
        successful = successful && printImage.save( printImageFile.toAscii().data() );

	    if ( successful )
	    {
            WbConfigTools::SetFileName( GetCurrentConfig(),
                                        trackImageFile,
                                        TargetSchema::trackImgKey,
                                        WbConfigTools::FileNameMode_RelativeInsideWorkbench );
            WbConfigTools::SetFileName( GetCurrentConfig(),
                                        printImageFile,
                                        TargetSchema::printImgKey,
                                        WbConfigTools::FileNameMode_RelativeInsideWorkbench );

            ReloadCurrentConfig();
	    }
        else
        {
            Message::Show( this,
                           tr( "Targets Tool" ),
                           tr( "Error - Failed to save image(s)!" ),
                           Message::Severity_Critical );
        }
    }
    else
    {
        Message::Show( this,
                       tr( "Targets Tool" ),
                       tr( "Error - Save Workbench First!" ),
                       Message::Severity_Critical );
    }
}

void TargetsToolWidget::BrowseTargetImage( const KeyName& keyName )
{
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

            if ( FileNameUtils::FileIsExternal( imageName, GetCurrentConfig() ) )
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

            WbConfigTools::SetFileName( GetCurrentConfig(),
                                        imageName,
                                        keyName,
                                        mode );

            ReloadCurrentConfig();
        }
    }
}

void TargetsToolWidget::BrowseTrackingBtnClicked()
{
    BrowseTargetImage( TargetSchema::trackImgKey );
}

void TargetsToolWidget::ClearTrackingBtnClicked()
{
    WbConfigTools::SetFileName( GetCurrentConfig(),
                                "",
                                TargetSchema::trackImgKey,
                                WbConfigTools::FileNameMode_Absolute );

    ReloadCurrentConfig();
}

void TargetsToolWidget::BrowsePrintableBtnClicked()
{
    BrowseTargetImage( TargetSchema::printImgKey );
}

void TargetsToolWidget::ClearPrintableBtnClicked()
{
    WbConfigTools::SetFileName( GetCurrentConfig(),
                                "",
                                TargetSchema::printImgKey,
                                WbConfigTools::FileNameMode_Absolute );

    ReloadCurrentConfig();
}

void TargetsToolWidget::TargetImageChanged()
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

void TargetsToolWidget::PrintTargetBtnClicked()
{
    const QString fileName = WbConfigTools::GetFileName( GetCurrentConfig(), TargetSchema::printImgKey );

    if ( !fileName.isEmpty() )
    {
        ImagePrintPreviewDlg printDlg( (QImage( fileName )) );
        printDlg.exec();
    }
}
