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

#ifndef CREATEFLOORPLANTOOLWIDGET_H
#define CREATEFLOORPLANTOOLWIDGET_H

#include "Tool.h"

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include <QImage>
#include <QTableWidget>
#include <QWidget>

class ImageGrid;
class ImageView;
class CameraRelationsTableMapper;
class CaptureLiveDualController;
class CameraHardware;

namespace Ui
{
    class CreateFloorPlanToolWidget;
}

/** \brief A class.
 *
 */
class CreateFloorPlanToolWidget : public Tool
{
    Q_OBJECT

public:
    explicit CreateFloorPlanToolWidget( CameraHardware& cameraHardware,
                                        QWidget* parent = 0 );
    virtual ~CreateFloorPlanToolWidget();

    virtual const QString Name() const { return tr( "Create Floor Plan" ); }
    virtual QWidget* Widget();

private:
    void SetupUi();
    void ConnectSignals();
    void ResetUi();

    void CreateMappers();
    virtual const QString GetSubSchemaDefaultFileName() const;

    const WbSchema CreateSchema();

    const QString GetCamera1Id() const;
    const QString GetCamera2Id() const;

    void FillOutCameraCombo( QComboBox& comboBox );

    void ShowImage( IplImage* img, ImageView* view );

    void CreateFloorPlanSingle();
    void CreateFloorPlanMulti();

    void Stitch(KeyId rootId);

    virtual void ReloadCurrentConfigToolSpecific();

    ImageView* const GetStreamingView1( const QSize& imageSize );
    ImageView* const GetStreamingView2( const QSize& imageSize );

    void DisplayMatched( std::vector< cv::Point2f > ip1,
                         std::vector< cv::Point2f > ip2 );
    void DisplayStitched();

    Ui::CreateFloorPlanToolWidget* m_ui;

private slots:
    void CameraComboChanged();

    void BtnRotateClicked();
    void BtnMatchClicked();
    void BtnStitchClicked();
    void BtnSaveClicked();
    void BtnCancelClicked();

    void BtnCreateFloorPlanClicked();

    void FromFileBtnClicked();
    void CaptureLiveBtnClicked();
    void CaptureCancelBtnClicked();

private:
    KeyId m_camPosId1;
    KeyId m_camPosId2;

    QString m_camera1FileName;
    QString m_camera2FileName;

    IplImage* m_cam1Img;
    IplImage* m_cam2Img;

    cv::Mat m_homography;

    CameraRelationsTableMapper* m_relationsMapper;

    std::auto_ptr< CaptureLiveDualController > m_captureLiveDualController;

    int m_rotAngle;
};

#endif // CREATEFLOORPLANTOOLWIDGET_H
