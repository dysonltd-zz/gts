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

#ifndef PROCESSRESULTSWIDGET_H
#define PROCESSRESULTSWIDGET_H

#include "Tool.h"

namespace Ui
{
    class ProcessResultsWidget;
}

class ProcessResultsWidget : public Tool
{
    Q_OBJECT

public:
    explicit ProcessResultsWidget( QWidget* parent );
    ~ProcessResultsWidget();

    virtual const QString Name() const { return tr( "Process Results" ); }
    virtual bool CanClose() const;
    const QString CannotCloseReason() const;

private:
    virtual const QString GetSubSchemaDefaultFileName() const;

    const WbSchema CreateSchema();

private slots:
    void LoadResultsButtonClicked();

private:
    Ui::ProcessResultsWidget* m_ui;
};

#endif // PROCESSRESULTSWIDGET_H
