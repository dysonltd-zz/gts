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

#include "QtMessageHandler.h"

#include <QtCore/qstringbuilder.h>
#include <QtGui/QMessageBox>
#include <QtGui/QStatusBar>

#include <cassert>

#include <QtGlobal>

QtMessageHandler::QtMessageHandler( QMainWindow& mainWindow )
:
m_mainWindow( mainWindow )
{
}

QtMessageHandler::~QtMessageHandler()
{
}

void QtMessageHandler::Show( QWidget* const parent, const QString& title,
                             const QString& message, const Message::Severity& severity,
                             const QString& details )
{
    QWidget* dialogBoxParent = parent;
    if ( dialogBoxParent == 0 )
    {
        dialogBoxParent = &m_mainWindow;
    }

    QMessageBox::Icon msgBoxIcon = QMessageBox::NoIcon;

    Q_UNUSED(msgBoxIcon);

    switch ( severity )
    {
        case Message::Severity_Status:
        {
            QStatusBar* const statusBar = m_mainWindow.statusBar();
            assert( statusBar );
            statusBar->showMessage( title % QObject::tr( ": " ) % message );
            break;
        }
        case Message::Severity_Information:
        case Message::Severity_NonBlockingInfo:
        {
            msgBoxIcon = QMessageBox::Information;
            break;
        }
        case Message::Severity_Warning:
        {
            msgBoxIcon = QMessageBox::Warning;
            break;
        }
        case Message::Severity_Critical:
        {
            msgBoxIcon = QMessageBox::Critical;
            break;
        }
        default:
        {
            assert( !"Unhandled message severity" );
            break;
        }
    }

    if ( severity != Message::Severity_Status )
    {
        QMessageBox* msgBox = new QMessageBox( QMessageBox::Information,
                                               title,
                                               message,
                                               QMessageBox::Ok,
                                               dialogBoxParent );

        msgBox->setWindowModality( Qt::ApplicationModal );
        msgBox->setDetailedText( details );

        if ( severity == Message::Severity_NonBlockingInfo )
        {
            msgBox->show();
        }
        else
        {
            msgBox->exec();
        }
    }
}

