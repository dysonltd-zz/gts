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

#include "ImagePrinter.h"

ImagePrinter::ImagePrinter(const QImage &image)
    : m_image(image)
{
}

void ImagePrinter::doPrinting(QPrinter* printer)
{
    if (printer)
    {
        FixMargins(*printer);

        QPainter painter;
        painter.begin(printer);
        QRectF pageRect = printer->pageRect();
        const QSizeF pageSizeF(pageRect.size());
        const QSize pageSize(Truncate(pageSizeF));
        const QImage scaledImage(m_image.scaled(pageSize,
                                                  Qt::KeepAspectRatio,
                                                  Qt::FastTransformation));

        const QSizeF offset((pageSize - scaledImage.size())/ 2.0);

        painter.drawImage(QPoint(offset.width(), offset.height()),
                           scaledImage,
                           scaledImage.rect());
        painter.end();
    }
}

const QSize ImagePrinter::Truncate(const QSizeF& pageSizeF)
{
    const QSize pageSize(static_cast< int >(pageSizeF.width()),
                         static_cast< int >(pageSizeF.height()));
    return pageSize;
}

void ImagePrinter::FixMargins(QPrinter& printer)
{
    const qreal minPageMarginMm = 1.0;
    qreal left, top, right, bottom;
    printer.getPageMargins(&left, &top, &right, &bottom, QPrinter::Millimeter);

    if (left < minPageMarginMm) left = minPageMarginMm;
    if (top < minPageMarginMm) top = minPageMarginMm;
    if (right < minPageMarginMm) right = minPageMarginMm;
    if (bottom < minPageMarginMm) bottom = minPageMarginMm;

    printer.setPageMargins(left, top, right, bottom, QPrinter::Millimeter);
}
