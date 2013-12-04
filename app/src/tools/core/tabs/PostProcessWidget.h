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

#ifndef POSTPROCESSWIDGET_H
#define POSTPROCESSWIDGET_H

#include <QWidget>

#include "Tool.h"
#include "AlgorithmInterface.h"
#include "RobotTracker.h"

class TrackModel;
class QItemSelectionModel;

namespace Ui
{
    class PostProcessWidget;
}

class PostProcessWidget : public Tool
{
    Q_OBJECT

public:
    explicit PostProcessWidget(QWidget* const parent = 0);
    ~PostProcessWidget();

    virtual const QString Name() const { return tr("Post Process"); }
    virtual bool CanClose() const;
    virtual const QString CannotCloseReason() const;

private slots:
    void LoadDataButtonClicked();
    void PostProcessButtonClicked();
    void ToggleItemTriggered();

private:
    const KeyId GetRoomIdToCapture() const;
    void ShowNoRoomError();
    virtual const QString GetSubSchemaDefaultFileName() const;
    const WbSchema CreateSchema();
    const ExitStatus::Flags PostProcess(const WbConfig& postProcConfig,
                                         char*           trackerResultsCsvFile,
                                         char*           trackerResultsTxtFile,
                                         char*           trackerResultsImgFile,
                                         char*           coverageFile,
                                         char*           floorPlanFile,
                                         char*           floorMaskFile,
                                         char*           relativeLogFile,
                                         char*           pixelOffsetsFile,
                                         char*           coverageMissedFile,
                                         char*           coverageColourFile,
                                         char*           coverageHistogramFile,
                                         char*           coverageRawFile,
                                         char*           headingFile,
                                         double          incTimeStep);

    void PlotTrackLog(TrackHistory::TrackLog& log,
                       char*                   floorPlanFile,
                       char*                   trackerResultsImgFile);


    Ui::PostProcessWidget* m_ui;
    TrackModel *m_resultsModel;
    QItemSelectionModel *m_selectionModel;
};

#endif // POSTPROCESSWIDGET_H
