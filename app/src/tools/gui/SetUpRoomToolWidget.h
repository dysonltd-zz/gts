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

#ifndef SETUPROOMTOOLWIDGET_H
#define SETUPROOMTOOLWIDGET_H

#include "CollectionToolWidget.h"
#include <QtGui/qwidget.h>
#include <memory>
#include <vector>

class RoomTableMapper;
namespace Ui
{
    class SetUpRoomToolWidget;
}

class SetUpRoomToolWidget : public Tool
{
    Q_OBJECT

public:
    explicit SetUpRoomToolWidget( QWidget* parent );
    ~SetUpRoomToolWidget();

    virtual const QString Name() const;
    virtual QWidget* Widget();

private slots:
    void AddClicked();
    void RemoveClicked();

private:
    virtual const QString GetSubSchemaDefaultFileName() const;

    static const WbSchema CreateSchema();
    std::auto_ptr< Ui::SetUpRoomToolWidget > m_ui;
    RoomTableMapper* m_roomTableMapper;
};

#endif // SETUPROOMTOOLWIDGET_H
