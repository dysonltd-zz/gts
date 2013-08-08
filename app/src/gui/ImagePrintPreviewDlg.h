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

#ifndef IMAGEPRINTPREVIEWDLG_H_
#define IMAGEPRINTPREVIEWDLG_H_

#include <QtGui/QPrintPreviewDialog>
#include <QtGui/QImage>

#include "ImagePrinter.h"

class ImagePrintPreviewDlg
{
public:
    ImagePrintPreviewDlg( const QImage& image ) :
        m_image( image )
    {
    }

    int exec()
    {
        QPrinter printer;

        const qreal defaultPageMarginMm = 10.0;
        printer.setPageMargins( defaultPageMarginMm, defaultPageMarginMm,
                                defaultPageMarginMm, defaultPageMarginMm,
                                QPrinter::Millimeter );

        if ( m_image.width() > m_image.height() )
        {
            printer.setOrientation( QPrinter::Landscape );
        }

        ImagePrinter p( m_image );
        QPrintPreviewDialog ppDlg( &printer );

        QObject::connect( &ppDlg,
                          SIGNAL( paintRequested(QPrinter*) ),
                          &p,
                          SLOT ( doPrinting(QPrinter*) ) );

        return ppDlg.exec();
    }

private:
    QImage m_image;
};

#endif

