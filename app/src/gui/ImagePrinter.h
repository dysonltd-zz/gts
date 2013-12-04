/*
 * Copyright (C)2007-2013 Dyson Technology Ltd, all rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option)any later version.
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

#include <QtGui/QPainter>
#include <QtGui/QPrinter>
#include <QtGui/QImage>
#include <QtCore/QDebug>

/**
  @brief A QObject to handle printing of a QImage.
  Designed to work directly or with a QPrintPreviewDialog.
 **/
class ImagePrinter: public QObject
{
    Q_OBJECT

public slots:
    /**
      @brief actually perform the printing.
      Takes the page size from the printer and scales the image to the maximum
      size to fit the page whilst maintaining its aspect ratio.
      @param printer The QPrinter object on which to print.
     **/
    void doPrinting(QPrinter* printer);

public:
    ImagePrinter(const QImage& image);

private:
    /**
      @brief Truncate a size to the nearest integer-based equivalent
      To prevent image going off edge of page if toSize()function rounds up.
      @param pageSizeF The QSizeF to truncate
      @return The QSize equivalent to @a pageSizeF with each component rounded @em down
       to the nearest integer.
     **/
    const QSize Truncate(const QSizeF& pageSizeF);

    /**
      @brief Ensure at least 1 mm margin all around due to odd image
      issues otherwise
      @param printer the QPrinter to modify
      @bug image truncation with zero margins requires this hack
     **/
    void FixMargins(QPrinter& printer);

    QImage m_image;
};

#endif

