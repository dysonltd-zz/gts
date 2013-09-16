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

#include "WorkbenchTreeWidget.h"

#include "OStreamConfigFileWriter.h"
#include "WbConfig.h"

#include <QtGui/QToolTip>
#include <QtGui/QHelpEvent>

#include <sstream>
#include <iostream>

WorkbenchTreeWidget::WorkbenchTreeWidget( QWidget* const parent )
:
QTreeWidget( parent )
{
}

WorkbenchTreeWidget::~WorkbenchTreeWidget()
{
}

bool WorkbenchTreeWidget::viewportEvent( QEvent* event )
{
#ifndef NDEBUG
    if ( event->type() == QEvent::ToolTip )
    {
        QHelpEvent* helpEvent = static_cast< QHelpEvent* >( event );
        QTreeWidgetItem* itemPointedAt = itemAt( helpEvent->pos() );
        if ( itemPointedAt )
        {
            WbConfig itemConfig( WbConfig::FromTreeItem( *itemPointedAt ) );

            std::ostringstream oss;
            OStreamConfigFileWriter writer( oss, OStreamConfigFileWriter::NonRecursive );
            itemConfig.WriteUsing( writer );

            QToolTip::showText( helpEvent->globalPos(),
                                QString::fromStdString( oss.str() ) );
        }
        else
        {
            QToolTip::hideText();
            event->ignore();
        }
        return true;
    }
#endif
    return QTreeWidget::viewportEvent( event );
}
