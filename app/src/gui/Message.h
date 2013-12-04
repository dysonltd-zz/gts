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

#ifndef MESSAGE_H_
#define MESSAGE_H_

#include <QWidget>

#include <memory>

class MessageHandler;

class Message
{
public:
    enum Severity
    {
        Severity_Status,
        Severity_NonBlockingInfo,
        Severity_Information,
        Severity_Warning,
        Severity_Critical
    };

    static void SetHandler(std::unique_ptr<MessageHandler> handler);
    static void Show(QWidget* const parent,
                      const QString& title,
                      const QString& message,
                      const Message::Severity& severity,
                      const QString& details = QString());

private:
    static std::unique_ptr<MessageHandler> messageHandler;
};

#endif // MESSAGE_H_
