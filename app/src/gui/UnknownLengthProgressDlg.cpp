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

#include "UnknownLengthProgressDlg.h"
#include <QtGui/QPushButton>

UnknownLengthProgressDlg::UnknownLengthProgressDlg( QWidget* const parent )
:
    QWidget( parent ),
    m_bar( new QProgressBar( this ) ),
    m_layout( new QVBoxLayout( this ) ),
    m_label( new QLabel( this ) ),
    m_allowClose( false )
{
    setWindowModality( Qt::ApplicationModal );
    setLayout( m_layout );
    m_bar->setTextVisible( false );
    m_bar->setRange( 0, 0 );
    setWindowModality( Qt::ApplicationModal );
    setWindowFlags( Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint );
    layout()->addWidget( m_label );
    layout()->addWidget( m_bar );
    setAttribute( Qt::WA_DeleteOnClose );
}

void UnknownLengthProgressDlg::Start( const QString& title, const QString& message )
{
    setWindowTitle( title );
    SetLabelText( message );
    show();
    AdjustGeometry();
}

void UnknownLengthProgressDlg::Complete( const QString& title, const QString& message )
{
    setWindowTitle( title );
    SetLabelText( message );
    const int maxVal = 1;
    m_bar->setRange( 0, maxVal );
    m_bar->setValue( maxVal );
    QPushButton* const okBtn = new QPushButton( "OK", this );
    m_allowClose = true;

    QObject::connect( okBtn,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( close() ) );

    m_layout->addWidget( okBtn, 0, Qt::AlignHCenter );
    okBtn->setFocus();
    okBtn->setDefault( true );
    AdjustGeometry();
}

void UnknownLengthProgressDlg::ForceClose()
{
    m_allowClose = true;
    close();
}

void UnknownLengthProgressDlg::closeEvent( QCloseEvent* event )
{
    if ( event && !m_allowClose )
    {
        event->ignore();
    }
}

void UnknownLengthProgressDlg::SetLabelText( const QString& message )
{
    m_label->setText( message );
    m_label->setVisible( !message.isEmpty() );
}

void UnknownLengthProgressDlg::AdjustGeometry()
{
    adjustSize();
    if ( parentWidget() )
    {
        const int x = parentWidget()->width()/2 - width()/2;
        const int y = parentWidget()->height()/2 - height()/2;
        const QPoint pos( parentWidget()->mapToGlobal( QPoint( x, y ) ) );
        const QRect  rect( pos, size() );
        setGeometry( rect );
    }
}
