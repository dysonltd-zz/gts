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

#ifndef QTMESSAGEHANDLER_H_
#define QTMESSAGEHANDLER_H_

#include "MessageHandler.h"
#include <QtGui/QMainWindow>

class QtMessageHandler: public MessageHandler
{
public:
    QtMessageHandler( QMainWindow& mainWindow );
    virtual ~QtMessageHandler();

    virtual void Show( QWidget* const  parent,
                       const QString&  title,
                       const QString&  message,
                       const Message::Severity& severity,
                       const QString& details  );

private:
    QtMessageHandler( const QtMessageHandler& );
    QtMessageHandler& operator =( const QtMessageHandler& );

    QMainWindow& m_mainWindow;
};

#endif // QTMESSAGEHANDLER_H_
