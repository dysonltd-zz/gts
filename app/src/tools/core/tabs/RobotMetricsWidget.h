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

#ifndef ROBOTMETRICSWIDGET_H
#define ROBOTMETRICSWIDGET_H

#include <QWidget>
#include <QLabel>

#include "Tool.h"

namespace Ui
{
    class RobotMetricsWidget;
}

class RobotMetricsWidget : public Tool
{
    Q_OBJECT

public:
    explicit RobotMetricsWidget(QWidget* parent = 0);
    virtual ~RobotMetricsWidget();

    virtual const QString Name() const { return tr("Robot Metrics"); }
    virtual bool CanClose() const;
    const QString CannotCloseReason() const;

private:
    virtual const QString GetSubSchemaDefaultFileName() const;

    void ReloadCurrentConfigToolSpecific();

    static const WbSchema CreateSchema();

    bool IsDataValid() const;

    Ui::RobotMetricsWidget* m_ui;
};

#endif // ROBOTMETRICSWIDGET_H
