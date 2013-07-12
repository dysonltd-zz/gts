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

#ifndef VIDEOPOSITIONBAR_H
#define VIDEOPOSITIONBAR_H

#include <QtGui/QWidget>

#include "ui_VideoPositionBar.h"

class VideoPositionBar : public QWidget
{
    Q_OBJECT

public:
    VideoPositionBar( QWidget* parent = 0 );
    ~VideoPositionBar();

    void SetPosition( double position );
    double GetRate();
    void SetRate(const double rate);

private:
    Ui::VideoPositionBar ui;
};

#endif // VIDEOPOSITIONBAR_H
