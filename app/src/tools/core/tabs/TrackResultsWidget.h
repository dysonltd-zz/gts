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

#ifndef TRACKRESULTSWIDGET_H
#define TRACKRESULTSWIDGET_H

#include <QWidget>
#include <QVector>

#include "Tool.h"

class QwtPlotZoomer;
class QwtPlotPanner;

namespace Ui
{
    class TrackResultsWidget;
}

class TrackResultsWidget : public Tool
{
    Q_OBJECT

public:
    explicit TrackResultsWidget( QWidget* parent = 0 );
    ~TrackResultsWidget();

    virtual const QString Name() const { return tr( "Track Results" ); }
    virtual bool CanClose() const;
    virtual const QString CannotCloseReason() const;

private:
    virtual const QString GetSubSchemaDefaultFileName() const;

    const WbSchema CreateSchema();

private:

private slots:
    void LoadDataButtonClicked();

private:
    Ui::TrackResultsWidget* m_ui;

    QVector<QPointF>* m_motionData;
    QVector<QPointF>* m_rotationData;

    QwtPlotZoomer* m_zoomer;
    QwtPlotPanner* m_panner;
};

#endif // TRACKRESULTSWIDGET_H
