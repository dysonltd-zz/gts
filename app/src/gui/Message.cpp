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

#include "Message.h"
#include "MessageHandler.h"

#include <cassert>

std::unique_ptr< MessageHandler > Message::messageHandler;

void Message::SetHandler( std::unique_ptr< MessageHandler > handler )
{
    assert( !messageHandler.get() );
    messageHandler = std::move(handler);
}

void Message::Show( QWidget* const parent,
                    const QString& title,
                    const QString& message,
                    const Message::Severity& severity,
                    const QString& details )
{
    assert( messageHandler.get() );
    messageHandler->Show( parent, title, message, severity, details );
}

