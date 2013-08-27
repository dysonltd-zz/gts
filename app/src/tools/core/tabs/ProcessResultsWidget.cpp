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

#include "ProcessResultsWidget.h"

#include "ui_ProcessResultsWidget.h"

#include "CoverageMetrics.h"

#include "Logging.h"

#include <qwt_plot_renderer.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_multi_barchart.h>
#include <qwt_column_symbol.h>
#include <qwt_plot_layout.h>
#include <qwt_legend.h>
#include <qwt_scale_draw.h>

ProcessResultsWidget::ProcessResultsWidget( QWidget* parent ) :
    Tool( parent, CreateSchema() ),
    m_ui( new Ui::ProcessResultsWidget )
{
    m_ui->setupUi( this );

    QObject::connect( m_ui->m_loadDataBtn,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( LoadResultsButtonClicked() ) );
}

ProcessResultsWidget::~ProcessResultsWidget()
{
    delete m_ui;
}

const QString ProcessResultsWidget::GetSubSchemaDefaultFileName() const
{
    return "processResults.xml";
}

bool ProcessResultsWidget::CanClose() const
{
    return true;
}

const QString ProcessResultsWidget::CannotCloseReason() const
{
    return tr("Please complete data before leaving tab.");
}

const WbSchema ProcessResultsWidget::CreateSchema()
{
    WbSchema schema( CreateWorkbenchSubSchema( KeyName( "processResults" ),
                                               tr( "Process Results" ) ) );

    return schema;
}

void ProcessResultsWidget::LoadResultsButtonClicked()
{
    const WbConfig& config = GetCurrentConfig();

    LOG_TRACE("Loading Results...");

    m_ui->m_trackPlot->setCanvasBackground( Qt::white );

    m_ui->m_trackPlot->setAxisTitle( QwtPlot::yLeft, "coverage (%)" );
    m_ui->m_trackPlot->setAxisTitle( QwtPlot::xBottom, "run" );

    QwtPlotMultiBarChart *d_barChartItem = new QwtPlotMultiBarChart();
    d_barChartItem->setLayoutPolicy( QwtPlotMultiBarChart::AutoAdjustSamples );
    d_barChartItem->setSpacing( 20 );
    d_barChartItem->setMargin( 3 );

    d_barChartItem->attach( m_ui->m_trackPlot );

    m_ui->m_trackPlot->insertLegend( new QwtLegend(), QwtPlot::BottomLegend );

    d_barChartItem->setStyle( QwtPlotMultiBarChart::Grouped );

    // Set legends...
    QList<QwtText> titles;

    for ( int i = 0; i < RunEntry::MAX_LEVEL; i++ )
    {
        QString title("Pass %1");
        titles += title.arg( i+1 );
    }

    d_barChartItem->setBarTitles( titles );
    d_barChartItem->setLegendIconSize( QSize( 10, 14 ) );

    for ( int i = 0; i < RunEntry::MAX_LEVEL; i++ )
    {
        QwtColumnSymbol *symbol = new QwtColumnSymbol( QwtColumnSymbol::Box );
        symbol->setLineWidth( 2 );
        symbol->setFrameStyle( QwtColumnSymbol::Raised );
        symbol->setPalette( RunEntry::colours[i] );

        d_barChartItem->setSymbol( i, symbol );
    }

    // Load the coverage metrics...
    CoverageMetrics::RunMetrics metrics;

    QVector< QVector<double> > series;

    const QString totalCoverageCsvName(
        config.GetAbsoluteFileNameFor( "results/total_coverage.csv" ) );

    if (CoverageMetrics::ReadCsv( totalCoverageCsvName.toAscii().data(), metrics ))
    {
        for ( unsigned int i = 0; i < metrics.size(); i++ )
        {
            QVector<double> values;

            for ( int j = 0; j < RunEntry::MAX_LEVEL; j++ )
                values += metrics.at(i).level[j];

            series += values;
        }
    }

    d_barChartItem->setSamples( series );

    d_barChartItem->setOrientation( Qt::Vertical );

    m_ui->m_trackPlot->setAxisAutoScale( QwtPlot::xBottom );
    m_ui->m_trackPlot->setAxisAutoScale( QwtPlot::yLeft );

    QwtScaleDraw *scaleDraw1 = m_ui->m_trackPlot->axisScaleDraw( QwtPlot::xBottom );
    scaleDraw1->enableComponent( QwtScaleDraw::Backbone, false );
    scaleDraw1->enableComponent( QwtScaleDraw::Ticks, false );

    QwtScaleDraw *scaleDraw2 = m_ui->m_trackPlot->axisScaleDraw( QwtPlot::yLeft );
    scaleDraw2->enableComponent( QwtScaleDraw::Backbone, true );
    scaleDraw2->enableComponent( QwtScaleDraw::Ticks, true );

    m_ui->m_trackPlot->plotLayout()->setAlignCanvasToScale( QwtPlot::xBottom, true );
    m_ui->m_trackPlot->plotLayout()->setAlignCanvasToScale( QwtPlot::yLeft, false );

    m_ui->m_trackPlot->plotLayout()->setCanvasMargin( 0 );
    m_ui->m_trackPlot->updateCanvasMargins();

    m_ui->m_trackPlot->replot();
}
