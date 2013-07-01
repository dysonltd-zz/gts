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

#include "HelpViewer.h"

#include "ui_HelpViewer.h"

#include <QApplication>

#include <QtGlobal>

#define HELP_FILE "gts_userguide.qhc"

HelpViewer::HelpViewer( QWidget *parent ) :
    QWidget           ( parent ),
    m_ui              ( new Ui::HelpViewer ),
    m_showHelpBtn     (),
    m_pendingBookmark ( HELP_INVALID )
{
    m_ui->setupUi(this);

    m_showHelpBtn = new QToolButton( this );
    m_showHelpBtn->setIcon( QIcon( ":/help.png" ) );
    m_showHelpBtn->setPopupMode(QToolButton::InstantPopup);

    QObject::connect( m_showHelpBtn,
	                  SIGNAL( clicked() ),
                      this,
			          SLOT( ShowHelpClicked() ) );

#if 0
    m_process_help = new QProcess(this);
#endif

	m_help_exec = false;

#if 0
	QObject::connect( m_process_help,
	                  SIGNAL( started() ),
                      this,
			          SLOT( OnStartedHelp() ) );
	QObject::connect( m_process_help,
	                  SIGNAL( finished( int, QProcess::ExitStatus ) ),
			          this,
			          SLOT( OnEndHelp( int, QProcess::ExitStatus ) ) );
#endif
}

HelpViewer::~HelpViewer()
{
    delete m_ui;
}

QToolButton* const HelpViewer::GetShowHelpBtn() const
{
    return m_showHelpBtn;
}

void HelpViewer::SetUiWidget( QWidget* const widget )
{
    delete m_ui->containerWidget->layout();
    QHBoxLayout* layout = new QHBoxLayout;
    layout->addWidget( widget );
    layout->setContentsMargins( 0, 0, 0, 0 );
    m_ui->containerWidget->setLayout( layout );
}

void HelpViewer::ShowHelpClicked()
{
    Show();
}

void HelpViewer::showPage( const QString &page )
{
    Q_UNUSED(page);

#if 0
    QString path = QApplication::applicationDirPath() + "/help/" + page;

    if (!assistant)
        assistant = new QAssistantClient("");

    assistant->showPage(path);
#endif
}

void HelpViewer::preparePage( HelpBookmark bookmark )
{
    m_pendingBookmark = bookmark;
}

void HelpViewer::requestPage( HelpBookmark bookmark )
{
    if (m_help_exec)
    {
        showPage( bookmark );
    }
    else
    {
        m_pendingBookmark = bookmark;

        Show();
    }
}

void HelpViewer::showPage( HelpBookmark bookmark )
{
	QByteArray ba;
	ba.append("activateIdentifier " + helpBookmark[bookmark]);
	ba.append('\0');
	ba.append('\n');

	m_process_help->write(ba);
}

void HelpViewer::Show()
{
    if (m_help_exec)
    {
        if (m_pendingBookmark != HELP_INVALID)
        {
            showPage( m_pendingBookmark );

	        m_pendingBookmark = HELP_INVALID;
        }
    }
    else
    {
        m_process_help = new QProcess;
        QStringList args;

        args << QLatin1String("-collectionFile")
        #ifdef __GNUC__
             << ( qApp->applicationDirPath() + "/" + HELP_FILE )
        #else
             << ( qApp->applicationDirPath() + "/../doc/" + HELP_FILE )
        #endif

             << QLatin1String("-enableRemoteControl");
        QObject::connect( m_process_help,
                         SIGNAL( started() ),
                         this,
                         SLOT( OnStartedHelp() ) );
        QObject::connect( m_process_help,
                         SIGNAL( finished( int, QProcess::ExitStatus ) ),
                         this,
                         SLOT( OnEndHelp( int, QProcess::ExitStatus ) ) );

        m_process_help->start(QLatin1String("assistant"), args);
        m_process_help->waitForStarted();
        if( !m_process_help->waitForStarted() )
        {
            return;
        }

        m_help_exec = true;
    }
}

void HelpViewer::Close()
{
    if (m_help_exec)
    {
        m_process_help->terminate();
		m_process_help->waitForFinished();
		m_help_exec = false;
    }
}

void HelpViewer::OnHelpChange()
{
#if 0
	QByteArray ba;
	ba.append("setSource qthelp://Sample_Help/doc/doc2.html");
	ba.append('\0');

	m_process_help->write(ba);
#endif
}

void HelpViewer::OnStartedHelp()
{
    if (m_pendingBookmark != HELP_INVALID)
    {
        showPage( m_pendingBookmark );

	    m_pendingBookmark = HELP_INVALID;
    }
}

void HelpViewer::OnEndHelp( int, QProcess::ExitStatus )
{
	m_help_exec = false;
}
