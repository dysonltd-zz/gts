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

#include "TrackResultsWidget.h"

#include "ui_TrackResultsWidget.h"

#include "TrackHistory.h"

#include "Angles.h"
#include "MathsConstants.h"

#include "Logging.h"

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_symbol.h>
#include <qwt_legend.h>

TrackResultsWidget::TrackResultsWidget( QWidget* parent ) :
    Tool( parent, CreateSchema() ),
    m_ui( new Ui::TrackResultsWidget )
{
    m_ui->setupUi( this );

    QObject::connect( m_ui->m_loadDataBtn,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( LoadDataButtonClicked() ) );
}

TrackResultsWidget::~TrackResultsWidget()
{
    delete m_ui;
}

const QString TrackResultsWidget::GetSubSchemaDefaultFileName() const
{
    return "postProcess.xml";
}

bool TrackResultsWidget::CanClose() const
{
    return true;
}

const QString TrackResultsWidget::CannotCloseReason() const
{
    return tr("Please complete data before leaving tab.");
}

const WbSchema TrackResultsWidget::CreateSchema()
{
    WbSchema schema( CreateWorkbenchSubSchema( KeyName( "trackResults" ), tr( "Track Results" ) ) );

    return schema;
}

void TrackResultsWidget::LoadDataButtonClicked()
{
    TrackHistory::TrackLog log;

    const WbConfig& config = GetCurrentConfig();

    LOG_TRACE("Track Results Load");

    if (m_motionData) delete m_motionData;
    if (m_rotationData) delete m_rotationData;

    m_ui->m_trackPlot->setCanvasBackground( Qt::white );
    m_ui->m_trackPlot->setAxisAutoScale( QwtPlot::xBottom, true );
    m_ui->m_trackPlot->setAxisAutoScale( QwtPlot::yLeft, true );
    m_ui->m_trackPlot->setAxisAutoScale( QwtPlot::yRight, true );

    m_ui->m_trackPlot->insertLegend( new QwtLegend(), QwtPlot::BottomLegend );

    QwtPlotGrid *grid = new QwtPlotGrid();
    grid->attach( m_ui->m_trackPlot );

    m_ui->m_trackPlot->enableAxis( QwtPlot::yLeft );
    m_ui->m_trackPlot->enableAxis( QwtPlot::yRight );
    m_ui->m_trackPlot->setAxisTitle( QwtPlot::yLeft, "cm" );
    m_ui->m_trackPlot->setAxisTitle( QwtPlot::yRight, "rad" );

    QwtPlotCurve *motionCurve = new QwtPlotCurve();
    motionCurve->setTitle( "Distance" );
    motionCurve->setPen( Qt::blue, 1 ),
    motionCurve->setRenderHint( QwtPlotItem::RenderAntialiased, true );
    motionCurve->setYAxis( QwtPlot::yLeft );

    QwtPlotCurve *rotationCurve = new QwtPlotCurve();
    rotationCurve->setTitle( "Rotation" );
    rotationCurve->setPen( Qt::red, 1 ),
    rotationCurve->setRenderHint( QwtPlotItem::RenderAntialiased, true );
    rotationCurve->setYAxis( QwtPlot::yRight );

    const WbConfig runConfig( config.GetParent() );

     const QString fileName(
         runConfig.GetAbsoluteFileNameFor( "results/track_result_out.txt" ) );

    m_motionData = new QVector<QPointF>;
    m_rotationData = new QVector<QPointF>;

    if (TrackHistory::ReadHistoryLog( fileName.toAscii().data(), log ))
    {
        if (log.size() > 1)
        {
            double posX = log[0].GetPosition().x;
            double posY = log[0].GetPosition().y;
            double ang = log[0].GetOrientation();

            for ( unsigned int i=1; i<log.size(); ++i )
            {
                double posXi = log[i].GetPosition().x;
                double posYi = log[i].GetPosition().y;
                double angi = log[i].GetOrientation();

                double diffPos = sqrt(((posX - posXi)*(posX - posXi)) +
                                      ((posY - posYi)*(posY - posYi)));

                double diffAng = Angles::DiffAngle(ang, angi);

                m_motionData->push_back(QPointF(i,diffPos));
                m_rotationData->push_back(QPointF(i,diffAng));

                posX = posXi;
                posY = posYi;
                ang = angi;
            }
        }
    }

    motionCurve->setSamples( *m_motionData );
    rotationCurve->setSamples( *m_rotationData );

    motionCurve->attach( m_ui->m_trackPlot );
    rotationCurve->attach( m_ui->m_trackPlot );

    m_ui->m_trackPlot->updateAxes();

    m_ui->m_trackPlot->replot();
}
