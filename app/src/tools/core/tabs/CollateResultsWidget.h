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

#ifndef COLLATERESULTSWIDGET_H
#define COLLATERESULTSWIDGET_H

#include <QStandardItemModel>

#include "Tool.h"
#include "AlgorithmInterface.h"
#include "ImageView.h"

#include <fstream>
#include <vector>

namespace Ui
{
    class CollateResultsWidget;
}

class RunEntry
{

public:
    static const int MAX_LEVEL = 10;
    static const int GetMaxLevel() { return MAX_LEVEL; };

    RunEntry() {};
    ~RunEntry() {};

    float level[MAX_LEVEL];
};

class CollateResultsWidget : public Tool
{
    Q_OBJECT

public:
    explicit CollateResultsWidget(QWidget* parent);
    ~CollateResultsWidget();

    virtual const QString Name() const { return tr("Collate Results"); }
    virtual bool CanClose() const;
    const QString CannotCloseReason() const;

private slots:
    void LoadRunsButtonClicked();
    void AnalyseResultsButtonClicked();
    void SelectAllCheckBoxChecked(int state);

private:
    typedef std::vector<RunEntry> RunMetrics;

    virtual const QString GetSubSchemaDefaultFileName() const;
    const WbSchema CreateSchema();
    bool CreateAnalysisResultDirectory(const WbConfig& config);
    void ShowImage(ImageView* view, const IplImage* image);
    const KeyId GetRoomIdToCollate() const;
    const ExitStatus::Flags CollateCoverageResults(char* floorPlanName,
                                                   char* floorMaskName,
                                                   char* overlayListFileName,
                                                   char* totalCoverageCsvName,
                                                   char* totalCoverageImgName);
    void PrintCsvHeaders(FILE* fp , const int maxLevel);
    void PrintCsvLineForPass(FILE* fp,
                             const int run,
                             IplImage* totalCoverageImg,
                             const int nFloorPixels , const int maxLevel);
    Ui::CollateResultsWidget* m_ui;
    QStandardItemModel* tableModel;
};

#endif // COLLATERESULTSWIDGET_H
