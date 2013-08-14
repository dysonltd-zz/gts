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

#ifndef CalibratePositionTOOLWIDGET_H
#define CalibratePosITIONTOOLWIDGET_H

#include "Tool.h"
#include "HelpViewer.h"

#include "WbConfigTools.h"

#include <memory>

#include <QtCore/QSize>
#include <QWidget>

namespace Ui
{
    class CalibratePositionToolWidget;
}

class CalibrationImageViewMapper;
class CaptureLiveBtnController;
class CameraHardware;
class ImageView;

class CalibratePositionToolWidget : public Tool
{
    Q_OBJECT

public:
    explicit CalibratePositionToolWidget( CameraHardware& cameraHardware,
                                          QWidget* parent = 0 );
    ~CalibratePositionToolWidget();

    virtual const QString Name() const { return tr( "Calibrate Position" ); }
    virtual const HelpBookmark GetHelpText() const;

    virtual bool CanClose() const;
    virtual const QString CannotCloseReason() const;

private slots:
    void CaptureLiveBtnClicked();
    void CaptureCancelBtnClicked();
    void CalibrateBtnClicked();
    void PrintGridBtnClicked();
    void FromFileBtnClicked();
    void ShowUnwarpedBtnClicked();

private:
    ImageView* const GetStreamingView( const QSize& imageSize );
    virtual const QString GetSubSchemaDefaultFileName() const;
    virtual void ReloadCurrentConfigToolSpecific();
    void SetCalibrationImage( const QString& imageName,
                              const WbConfigTools::FileNameMode& mode );

    static const WbSchema CreateSchema();

    bool IsDataValid() const;

    Ui::CalibratePositionToolWidget* m_ui;

    CalibrationImageViewMapper* m_viewMapper;

    ImageView* m_imageView;

    std::auto_ptr< CaptureLiveBtnController > m_captureLiveBtnController;
};

#endif // CALIBRATEPOSITIONTOOLWIDGET_H
