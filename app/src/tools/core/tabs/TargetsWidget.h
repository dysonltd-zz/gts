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

#ifndef TARGETSWIDGET_H
#define TARGETSWIDGET_H

#include <QWidget>

#include "Tool.h"
#include "KeyName.h"

namespace Ui
{
    class TargetsWidget;
}

class TargetsWidget : public Tool
{
    Q_OBJECT

public:
    explicit TargetsWidget(QWidget* parent = 0);
    virtual ~TargetsWidget();

    virtual const QString Name() const { return tr("Targets"); }
    virtual bool CanClose() const;
    const QString CannotCloseReason() const;

private slots:
    void UseStandardBtnClicked();

    void TargetTypeChanged();

    void BrowseTrackingBtnClicked();
    void ClearTrackingBtnClicked();

    void BrowsePrintableBtnClicked();
    void ClearPrintableBtnClicked();

    void PrintTargetBtnClicked();

private:
    virtual const QString GetSubSchemaDefaultFileName() const;

    void ReloadCurrentConfigToolSpecific();
    void TargetImageChanged();

    static const WbSchema CreateSchema();

    bool IsDataValid() const;

    const QString GetSelectedTargetId() const;

    QString BrowseTargetImage();

    bool DirectoryExists(const QString& outputDirectoryName);

    Ui::TargetsWidget* m_ui;
};

#endif // TARGETSWIDGET_H
