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

#include "NewElementWizard.h"

#include "ui_NewElementWizard.h"

#include "CameraApi.h"
#include "CameraDescription.h"
#include "CameraSelectionFormContents.h"
#include "CameraHardware.h"
#include "WbDefaultKeys.h"

#include <QtGui/QRegExpValidator>
#include <QtCore/QStringBuilder>
#include <QtGui/QApplication>

#include "Debugging.h"

const QString WizardStartPage::nameField( "name" );

WizardStartPage::WizardStartPage( const Collection& collection, const QString& title ) :
    NewElementWizardPage(),
    m_explanationIcon   ( new QLabel() ),
    m_explanationInfo   ( new QLabel() ),
    m_nameEdit          ( new QLineEdit() ),
    m_collection        ( collection )
{
    setTitle( title );

    QLabel* const nameLabel = new QLabel( tr( "&Name" ) );
    QRegExpValidator* const newValidator =
        new QRegExpValidator( QRegExp( "[a-zA-Z0-9_]+" ), m_nameEdit );
    m_nameEdit->setValidator( newValidator );
    nameLabel->setBuddy( m_nameEdit );

    QGridLayout* layout = new QGridLayout;
    m_explanationInfo->setWordWrap( true );
    layout->addWidget( nameLabel,  0, 0 );
    layout->addWidget( m_nameEdit, 0, 1 );

    QHBoxLayout* explanationLayout = new QHBoxLayout;
    explanationLayout->addWidget( m_explanationIcon );
    explanationLayout->addWidget( m_explanationInfo, 10 );
    explanationLayout->setSpacing( 6 );
    layout->addItem( explanationLayout,  1, 1, 1, 2 );
    setLayout(layout);

    registerField( nameField % mandatoryFieldSuffix, m_nameEdit );
}

void WizardStartPage::initializePage()
{
    setField( nameField, QString() );
    m_nameEdit->clear();
}

bool WizardStartPage::isComplete() const
{
    bool complete = QWizardPage::isComplete();
    m_explanationInfo->clear();
    m_explanationIcon->clear();
    if ( !complete )
    {
        m_explanationInfo->setText( tr( "The name must contain only numbers, "
                                        "letters and underscores, and must "
                                        "contain at least one character." ) );
    }
    if ( m_collection.AnyElementHas( WbDefaultKeys::displayNameKey,
                                     KeyValue::from( m_nameEdit->text() ),
                                     Qt::CaseInsensitive ) )
    {
        m_explanationInfo->setText( tr( "The name must not match that of any "
                                        "existing item (regardless of case)." ) );
        complete = false;
    }

    if ( !complete )
    {
        m_explanationIcon->setPixmap(
            QApplication::style()->standardIcon( QStyle::SP_MessageBoxInformation)
                .pixmap( 24, 24, QIcon::Normal, QIcon::On ) );
    }

    return complete;
}


//=================================================================================================================

const QString NewElementWizardPage::mandatoryFieldSuffix( "*" );

//=================================================================================================================

NewElementWizard::NewElementWizard( const Collection& collection,
                                    const QString& elementType,
                                    QWidget* const parent ) :
    QWizard( parent )
{
    const QString title( tr( "New %1" ).arg( elementType ) );
    setWindowTitle( title );
    setWindowFlags( windowFlags() & ~Qt::WindowContextHelpButtonHint );
    setOption( QWizard::NoBackButtonOnStartPage, true );
    setOption( QWizard::HaveNextButtonOnLastPage, false );

    addPage( new WizardStartPage( collection, title ) );
}

RenameElementWizard::RenameElementWizard( const Collection& collection,
                                          const QString& elementName,
                                          QWidget* const parent ) :
    QWizard( parent )
{
    const QString title( tr( "Rename %1" ).arg( elementName ) );
    setWindowTitle( title );
    setWindowFlags( windowFlags() & ~Qt::WindowContextHelpButtonHint );
    setOption( QWizard::NoBackButtonOnStartPage, true );
    setOption( QWizard::HaveNextButtonOnLastPage, false );

    addPage( new WizardStartPage( collection, title ) );
}
