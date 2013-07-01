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

#ifndef CREATEFLOORMASKTOOLWIDGET_H
#define CREATEFLOORMASKTOOLWIDGET_H

#include <QWidget>

#include "Tool.h"

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include <QtGui/qimage.h>

class ImageGrid;
class ImageView;

namespace Ui
{
    class CreateFloorMaskToolWidget;
}

class CreateFloorMaskToolWidget : public Tool
{
    Q_OBJECT

public:
    explicit CreateFloorMaskToolWidget( QWidget* parent = 0 );
    virtual ~CreateFloorMaskToolWidget();

    virtual const QString Name() const;
    virtual QWidget* Widget();

private:
    void SetupUi();
    void ConnectSignals();

    void CreateMappers();
    virtual const QString GetSubSchemaDefaultFileName() const;

    void ReloadCurrentConfigToolSpecific();

    void ShowImage(ImageView* view, const IplImage* image);

    const WbSchema CreateSchema();

    Ui::CreateFloorMaskToolWidget* m_ui;

private slots:
    void BtnExportPlanClicked();
    void BtnImportMaskClicked();

private:
    bool ExportFloorPlan( const WbConfig& config );
    bool ImportFloorMask( const WbConfig& config );
};

#endif // CREATEFLOORMASKTOOLWIDGET_H
