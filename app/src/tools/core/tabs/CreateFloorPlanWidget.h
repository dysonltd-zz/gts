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

#ifndef CREATEFLOORPLANWIDGET_H
#define CREATEFLOORPLANWIDGET_H

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
    class CreateFloorPlanWidget;
}

/** \brief A class.
 *
 */
class CreateFloorPlanWidget : public Tool
{
    Q_OBJECT

public:
    explicit CreateFloorPlanWidget( CameraHardware& cameraHardware,
                                    QWidget* parent = 0 );
    virtual ~CreateFloorPlanWidget();
    virtual const QString Name() const { return tr( "Create Floor Plan" ); }
    virtual bool CanClose() const;
    virtual const QString CannotCloseReason() const;
    virtual QWidget* Widget();

private slots:
    void CameraComboChanged();
    void RotateBtnClicked();
    void MatchBtnClicked();
    void StitchBtnClicked();
    void SaveBtnClicked();
    void CancelBtnClicked();
    void CreateFloorPlanBtnClicked();
    void FromFileBtnClicked();
    void CaptureLiveBtnClicked();
    void CaptureCancelBtnClicked();

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
    bool IsDataValid() const;

    Ui::CreateFloorPlanWidget* m_ui;
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

#endif // CREATEFLOORPLANWIDGET_H
