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

#include "SetUpRoomWidget.h"

#include "ui_SetUpRoomWidget.h"

#include "ConfigKeyMapper.h"
#include "WbConfigTools.h"
#include "WbDefaultKeys.h"
#include "RoomTableMapper.h"
#include "RoomLayoutSchema.h"

#include <QtGui/QTableWidget>

SetUpRoomWidget::SetUpRoomWidget(QWidget* parent) :
    Tool(parent, CreateSchema()),
    m_ui(new Ui::SetUpRoomWidget),
    m_roomTableMapper(0)
{
    m_ui->setupUi(this);
    m_roomTableMapper = new RoomTableMapper(*m_ui->m_roomLayoutTable);
    AddMapper(m_roomTableMapper);

    QObject::connect(m_ui->m_addBtn,
                      SIGNAL(clicked()),
                      this,
                      SLOT(AddClicked()));
    QObject::connect(m_ui->m_removeBtn,
                      SIGNAL(clicked()),
                      this,
                      SLOT(RemoveClicked()));
}

SetUpRoomWidget::~SetUpRoomWidget()
{
}

const QString SetUpRoomWidget::Name() const
{
    return tr("Room Layout");
}

QWidget* SetUpRoomWidget::Widget()
{
    return this;
}

const QString SetUpRoomWidget::GetSubSchemaDefaultFileName() const
{
    return "roomLayout.xml";
}

const WbSchema SetUpRoomWidget::CreateSchema()
{
    using namespace RoomLayoutSchema;
    WbSchema roomLayoutSchema(CreateWorkbenchSubSchema(schemaName,
                                                         tr("Room Layout")));
    roomLayoutSchema.AddSingleValueKey(cameraPositionIdsKey,
                                        WbSchemaElement::Multiplicity::One);
    return roomLayoutSchema;
}

void SetUpRoomWidget::AddClicked()
{
    m_roomTableMapper->AddRow();
}

void SetUpRoomWidget::RemoveClicked()
{
    m_roomTableMapper->RemoveCurrentRow();
}
