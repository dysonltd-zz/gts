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
    m_helpLaunched = false;
}

HelpViewer::~HelpViewer()
{
}

void HelpViewer::OnEndHelp(int, QProcess::ExitStatus)
{
    m_helpLaunched = false;
}

void HelpViewer::Show()
{
    // if help not already loaded
    if (!m_helpLaunched)
    {
        m_helpProcess = new QProcess;

        QStringList args;
        args << QLatin1String("-collectionFile")
        #ifdef __GNUC__
             << (qApp->applicationDirPath() + "/" + HELP_FILE);
        #else
             << (qApp->applicationDirPath() + "/../doc/" + HELP_FILE);
        #endif

        QObject::connect(m_helpProcess,
                         SIGNAL(finished(int, QProcess::ExitStatus)),
                         this,
                         SLOT(OnEndHelp(int, QProcess::ExitStatus)));

        m_helpProcess->start(QLatin1String("assistant"), args);
        m_helpProcess->waitForStarted();
        if(!m_helpProcess->waitForStarted())
        {
            return;
        }

        m_helpLaunched = true;
    }
}

void HelpViewer::Close()
{
    if (m_helpLaunched)
    {
        m_helpProcess->terminate();
        m_helpProcess->waitForFinished();
        m_helpLaunched = false;
    }
}

void HelpViewer::ShowHelp()
{
    Show();
}
