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

#ifndef CREATEFLOORMASKWIDGET_H
#define CREATEFLOORMASKWIDGET_H

#include <QWidget>

#include "Tool.h"

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include <QtGui/QImage>

class ImageGrid;
class ImageView;

namespace Ui
{
    class CreateFloorMaskWidget;
}

class CreateFloorMaskWidget : public Tool
{
    Q_OBJECT

public:
    explicit CreateFloorMaskWidget( QWidget* parent = 0 );
    virtual ~CreateFloorMaskWidget();

    virtual const QString Name() const { return tr( "Create Floor Mask" ); }
    virtual QWidget* Widget();

private:
    void SetupUi();
    void ConnectSignals();

    void CreateMappers();
    virtual const QString GetSubSchemaDefaultFileName() const;

    void ReloadCurrentConfigToolSpecific();

    void ShowImage(ImageView* view, const IplImage* image);

    const WbSchema CreateSchema();

    void CreateFloorMaskMulti();
    void CreateFloorMaskSingle();

    void Stitch(KeyId rootId);

    WbConfig GetFloorPlanConfig();

    Ui::CreateFloorMaskWidget* m_ui;

private slots:
    void BtnCombinePartsClicked();

    void BtnExportPlanClicked();
    void BtnImportMaskClicked();

    void BtnCreateMaskClicked();

private:
    bool ExportFloorPlan( const WbConfig& config );
    bool ImportFloorMask( const WbConfig& config );

    bool ExportFloorPlanParts( const WbConfig& config );
    bool ImportFloorMaskParts( const WbConfig& config );
};

#endif // CREATEFLOORMASKWIDGET_H
