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

#ifndef TRACKVIEW_H
#define TRACKVIEW_H

#include <QScrollArea>
#include <QAbstractItemModel>

#include "ImageView.h"

#include <cv.h>

class QLabel;
class WbConfig;

class TrackView : public QScrollArea
{
    Q_OBJECT

public:
    TrackView( QWidget *parent = 0 );

    IplImage* loadFloorPlan( const WbConfig& runConfig );
    bool loadMetrics( const WbConfig& runConfig );

    void setModel(QAbstractItemModel *model);

    QAbstractItemModel *model() const { return m_model; }

public slots:
    void currentChanged (const QModelIndex& current,
                         const QModelIndex& previous);

    void rowsRemoved ();

    void dataChanged ( const QModelIndex& topLeft,
                       const QModelIndex& bottomRight );

private:
    void updateView( int idx );

    IplImage*           m_baseImg;

    double              m_metricsScaleFactor;

    QAbstractItemModel* m_model;

    ImageView           m_imageView;
};

#endif // TRACKVIEW_H
