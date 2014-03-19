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

#ifndef CAMERACALIBRATIONWIDGET_H
#define CAMERACALIBRATIONWIDGET_H

#include "Tool.h"
#include "HelpViewer.h"
#include "WbConfigTools.h"

#include <memory>

#include <QtGui/QTableWidget>
#include <QtGui/QWidget>

namespace Ui
{
    class CameraCalibrationWidget;
}

class CameraHardware;
class ImageView;
class CaptureLiveBtnController;
class CalibrationImageGridMapper;
class CalibrationImageTableMapper;

class CameraCalibrationWidget : public Tool
{
    Q_OBJECT

public:
    explicit CameraCalibrationWidget( CameraHardware& cameraHardware,
                                        QWidget* const parent = 0 );
    ~CameraCalibrationWidget();

    virtual const QString Name() const { return tr( "Calibrate Camera" ); }
    virtual bool CanClose() const;
    virtual const QString CannotCloseReason() const;

private slots:
    void FromFileClicked();
    void CaptureLiveBtnClicked();
    void CaptureCancelBtnClicked();
    void CalibrateBtnClicked();
    void PrintGridBtnClicked();
    void ImageTableItemChanged(QTableWidgetItem* current, QTableWidgetItem* previous);

private:
    const QString GetSubSchemaDefaultFileName() const;

    void AddImageIfValid( const QString& imageFileName,
                          const WbConfigTools::FileNameMode& mode );

    ImageView* const CreateStreamingView( const QSize& imageSize );

    virtual void ReloadCurrentConfigToolSpecific();

    static const WbSchema CreateSchema();

    bool IsDataValid() const;

    CameraHardware& m_cameraHardware;
    std::auto_ptr< CaptureLiveBtnController > m_captureLiveBtnController;
    Ui::CameraCalibrationWidget* m_ui;
    CalibrationImageGridMapper* m_imageGridMapper;
    CalibrationImageTableMapper* m_imageTableMapper;
};

#endif // CAMERACALIBRATIONWIDGET_H
