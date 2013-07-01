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

#ifndef IMAGEGRID_H
#define IMAGEGRID_H

#include <QtGui/QFrame>
#include <QtGui/qscrollarea.h>
#include <QtCore/QString>
#include <QtGui/QImage>
#include "ImageView.h"
#include <vector>
#include <memory>
#include <QtGui/QGridLayout>

namespace Ui
{
    class ImageGrid;
}

/** @brief A class to show a grid of images, laid out in order to make each
 *  image as large as possible.
 *
 */
class ImageGrid : public QScrollArea
{
    Q_OBJECT

public:
    explicit ImageGrid( QWidget* parent = 0 );
    ~ImageGrid();

    typedef std::unique_ptr<ImageView> ImageViewPtr;
    ImageView* const AddImage( const QString& imageFileName, const QString& caption = QString(), int id = 0 );
    ImageView* const AddImage( const QImage& image,          const QString& caption = QString(), int id = 0 );
    ImageView *const AddBlankImage( const QSize& imageSize, int id = 0 );

    void Clear();

public slots:
     void updateImage( int id, const QImage& image );

protected:
    virtual void resizeEvent( QResizeEvent* );
    virtual void wheelEvent( QWheelEvent* event );

private:
    void InsertImageInGrid( const int imageIndex );
    void ReflowImages();
    void CalculateNumRowsColumns();

    double GetImageAreaWith( const size_t rows, const size_t columns ) const;

    Ui::ImageGrid*             m_ui;         ///< The UI class created by Qt designer.
    std::vector<ImageViewPtr>  m_imageViews; ///< The image views to display.
    size_t                     m_rows;       ///< The current number of rows.
    size_t                     m_columns;    ///< The current number of columns.
    QFrame*                    m_frame;
    QGridLayout*               m_gridLayout;
    double                     m_zoom;
};

#endif // IMAGEGRID_H
