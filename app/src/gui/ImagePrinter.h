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

#ifndef IMAGEPRINTER_H_
#define IMAGEPRINTER_H_

#include <QtGui/qpainter.h>
#include <QtGui/qprinter.h>
#include <QtGui/qimage.h>
#include <QtCore/qdebug.h>
/** @brief A QObject to handle printing of a QImage
 *
 *  Designed to work directly or with a QPrintPreviewDialog.
 */
class ImagePrinter: public QObject
{
    Q_OBJECT

public:
    ImagePrinter( const QImage& image )
        :
          m_image( image )
    {
    }

public slots:
    /** @brief actually perform the printing.
     *
     *  Takes the page size from the printer and scales the image to the maximum
     *  size to fit the page whilst maintaining its aspect ratio.
     *
     *  @param printer The QPrinter object on which to print.
     */
    void doPrinting( QPrinter* printer )
    {
        if ( printer )
        {
            FixMargins( *printer );

            QPainter painter;
            painter.begin( printer );
            QRectF pageRect = printer->pageRect();
            const QSizeF pageSizeF( pageRect.size() );
            const QSize pageSize( Truncate( pageSizeF ) );
            const QImage scaledImage( m_image.scaled( pageSize,
                    Qt::KeepAspectRatio, Qt::FastTransformation ) );
            const QSizeF offset( ( pageSize - scaledImage.size() ) / 2.0 );

            painter.drawImage( QPoint( offset.width(), offset.height() ),
                scaledImage, scaledImage.rect() );
            painter.end();
        }
    }
private:

    /** @brief Truncate a size to the nearest integer-based equivalent
     *
     *  To prevent image going off edge of page if toSize() function rounds up.
     * @param pageSizeF The QSizeF to truncate
     * @return The QSize equivalent to @a pageSizeF with each component rounded @em down
     * to the nearest integer.
     */
    const QSize Truncate( const QSizeF& pageSizeF )
    {
        const QSize pageSize( static_cast< int >( pageSizeF.width() ), static_cast<int>( pageSizeF.height() ) );
        return pageSize;
    }

    /** @brief Ensure at least 1 mm margin all around due to odd image
     *  issues otherwise
     *
     *  @bug image truncation with zero margins requires this hack
     *
     *  @param printer the QPrinter to modify
     */
    void FixMargins( QPrinter& printer )
    {
        const qreal minPageMarginMm = 1.0;
        qreal left, top, right, bottom;
        printer.getPageMargins( &left, &top, &right, &bottom, QPrinter::Millimeter );

        if ( left < minPageMarginMm ) left = minPageMarginMm;
        if ( top < minPageMarginMm ) top = minPageMarginMm;
        if ( right < minPageMarginMm ) right = minPageMarginMm;
        if ( bottom < minPageMarginMm ) bottom = minPageMarginMm;

        printer.setPageMargins( left, top, right, bottom, QPrinter::Millimeter );
    }

    QImage m_image;
};

#endif

