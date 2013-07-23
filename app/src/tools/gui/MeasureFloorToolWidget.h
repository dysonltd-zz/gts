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

#ifndef MEASUREFLOORTOOLWIDGET_H
#define MEASUREFLOORTOOLWIDGET_H

#include <QWidget>

#include "Tool.h"

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include <QtGui/qimage.h>

class ImageView;

namespace Ui
{
    class MeasureFloorToolWidget;
}

class MeasureFloorToolWidget : public Tool
{
    Q_OBJECT

public:
    explicit MeasureFloorToolWidget( QWidget* parent = 0 );
    virtual ~MeasureFloorToolWidget();

    virtual const QString Name() const;
    virtual QWidget* Widget();

private:
    void SetupUi();
    void ConnectSignals();

    void CreateMappers();
    virtual const QString GetSubSchemaDefaultFileName() const;

    void ReloadCurrentConfigToolSpecific();

    void ShowImage();

    const WbSchema CreateSchema();

    Ui::MeasureFloorToolWidget* m_ui;

private slots:
    void OverlayMaskClicked();
    void ViewClicked(int id, int x, int y);

private:
    ImageView* m_imageView;

    IplImage* floorImg;

    bool m_startPoint;
    bool m_endPoint;

    int m_startPointX;
    int m_startPointY;

    int m_endPointX;
    int m_endPointY;
};

#endif // MEASUREFLOORTOOLWIDGET_H
