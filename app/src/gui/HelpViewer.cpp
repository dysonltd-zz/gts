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

#include <QApplication>
#include <QtGlobal>

#define HELP_FILE "gts_userguide.qhc"

HelpViewer::HelpViewer()
{
	m_help_exec = false;
}

HelpViewer::~HelpViewer()
{
}

void HelpViewer::ShowHelpClicked()
{
    Show();
}

void HelpViewer::Show()
{
    if (!m_help_exec)
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

void HelpViewer::OnEndHelp( int, QProcess::ExitStatus )
{
	m_help_exec = false;
}
