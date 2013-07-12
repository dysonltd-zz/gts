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

#include "VideoPositionBar.h"

#include "ui_VideoPositionBar.h"

#include <QTime>

VideoPositionBar::VideoPositionBar( QWidget* parent )
    : QWidget( parent )
{
    ui.setupUi( this );

    update();
}

VideoPositionBar::~VideoPositionBar()
{
}

void VideoPositionBar::SetPosition( double position )
{
    QTime time = QTime(0,0);
    ui.m_timeLineEdit->setText( time.addMSecs(position).toString("hh:mm:ss:zzz") );
}

double VideoPositionBar::GetRate()
{
    return (double)(ui.m_rateSpinBox->value());
}

void VideoPositionBar::SetRate(const double rate)
{
    ui.m_rateSpinBox->setValue(rate);
}
