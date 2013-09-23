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

#include "ImageGrid.h"

#include "ui_ImageGrid.h"

#include <iostream>
#include <cmath>
#include <cassert>
#include <algorithm>

#include <QtGui/QWheelEvent>

#include "Debugging.h"

ImageGrid::ImageGrid( QWidget* parent ) :
    QScrollArea( parent ),
    m_ui( new Ui::ImageGrid ),
    m_zoom( 1.0 )
{
    m_ui->setupUi( this );
    setContentsMargins( 0, 0, 0, 0 );
    m_frame = new QFrame( this );
    setWidget( m_frame );
    m_gridLayout = new QGridLayout( m_frame );
    m_frame->setLayout( m_gridLayout );
}

ImageGrid::~ImageGrid()
{
    delete m_ui;
}

/** @brief Add a specified ImageView to this grid.
 *
 *  @param imageIndex The index in @a m_imageViews of the view to add.
 */
void ImageGrid::InsertImageInGrid( const int imageIndex )
{
    if ( m_rows*m_columns > 0 )
    {
        size_t numImages = m_gridLayout->count();
        size_t row       = numImages/m_columns;
        size_t column    = numImages%m_columns;

        if ( m_gridLayout->itemAtPosition( row, column ) )
        {
            m_gridLayout->removeItem( m_gridLayout->itemAtPosition( row, column ) );
        }

        m_gridLayout->addWidget(m_imageViews[imageIndex].get(), row, column, 1, 1);
    }
}

/** @brief Determine the best number of rows and columns to maximise displayable area of images
 *
 *  Tries to pick values which will make the ratio Actual Size : Image Full Size as large as
 *  possible.
 *
 */
void ImageGrid::CalculateNumRowsColumns()
{
    const size_t numImages = m_imageViews.size();

    float imageAspectRatio = m_imageViews[0]->GetImageAspectRatio(); /// @bug Currently all based on image[0]
    float gridAspectRatio  = (float)m_frame->width()/(float)m_frame->height();

    float approxRowsSquared = ( imageAspectRatio/gridAspectRatio )*numImages;

    m_rows = (size_t)( std::ceil( std::sqrt( approxRowsSquared ) ) );

    if ( m_rows == 0 )
    {
        m_columns = 0;
    }
    else
    {
        m_columns = numImages/m_rows;
        if ( m_columns == 0 )
        {
            m_columns = 1;
        }

        if ( m_columns*m_rows < numImages )
        {
            const int possibleColumns = std::ceil( (float)numImages/(float)m_rows );
            const int possibleRows    = std::ceil( (float)numImages/(float)m_columns );
            if ( GetImageAreaWith( possibleRows, m_columns ) >
                 GetImageAreaWith( m_rows, possibleColumns ) )
            {
                m_rows = possibleRows;
            }
            else
            {
                m_columns = possibleColumns;
            }
        }
        ASSERT_GREATER_THAN_OR_EQUAL( m_rows*m_columns, numImages );
    }
}

/** @brief Calculate the area devoted to an image if we use the specified number of rows and
 *  columns.
 *
 *  @param rows The number of rows.
 *  @param columns The number of columns.
 *
 *  @return The ideal (i.e. non-rounded) image area in pixels squared.
 */
double ImageGrid::GetImageAreaWith( const size_t rows, const size_t columns ) const
{
    assert( !m_imageViews.empty() );

    const QSize imageSize = m_imageViews.at( 0 )->GetImageSize();

    const double imageCellWidth  = (double)width()/columns;
    const double imageCellHeight = (double)height()/rows;

    const double scaleToFitWidth  = imageCellWidth/imageSize.width();
    const double scaleToFitHeight = imageCellHeight/imageSize.height();

    const double scale = std::min( scaleToFitHeight, scaleToFitWidth );

    return scale*imageSize.width()*imageSize.height();
}

/** @brief Recalculate the layout size (rows x columns) and organise the images appropriately.
 *
 */
void ImageGrid::ReflowImages()
{
    m_frame->setFixedSize( maximumViewportSize()*m_zoom );

    if ( m_imageViews.size() > 0 )
    {
        for ( size_t i = 0; i < m_imageViews.size(); ++i )
        {
            m_gridLayout->removeWidget(m_imageViews[i].get());
        }

        CalculateNumRowsColumns();

        for ( size_t i = 0; i < m_imageViews.size(); ++i )
        {
            InsertImageInGrid( i );
        }
    }
    else
    {
        m_rows = 0;
        m_columns = 0;
    }
}

/** @brief Add an image to the grid by file name.
 *
 *  @param imageFileName The fileName of the image to add.
 *  @param caption The caption to use.
 *
 *  @return The ImageView which was added.  The ImageGrid retains ownership.
 */
ImageView* const ImageGrid::AddImage( const QString& imageFileName, const QString& caption, int id )
{
    const QImage image( imageFileName );
    return AddImage( image, caption, id );
}

/** @brief Add an image to the grid.
 *
 *  @param image The image to add.
 *  @param caption The caption to use.
 *
 *  @return The ImageView which was added.  The ImageGrid retains ownership.
 */
ImageView* const ImageGrid::AddImage( const QImage& image, const QString& caption, int id )
{
    m_zoom = 1.0;
    const size_t imageNum = m_imageViews.size();
    ImageView* newImageView = new ImageView( this, id );
    newImageView->setObjectName( QString( "imageView%1" ).arg( imageNum ) );
    newImageView->setAutoFillBackground( false );
    newImageView->SetImage( image );

    static const bool PRESERVE_ASPECT_RATIO = true;
    newImageView->SetPreserveAspectRatio( PRESERVE_ASPECT_RATIO );
    newImageView->SetCaption( caption );

    m_imageViews.push_back(ImageViewPtr(newImageView));
    ReflowImages();
    return newImageView;
}

/** @brief Handle Qt GUI resize.
 *
 *  Event parameter is unused.
 */
void ImageGrid::resizeEvent( QResizeEvent* )
{
    ReflowImages();
}

void ImageGrid::Clear()
{
    m_zoom = 1.0;
    m_imageViews.clear();
    ReflowImages();
}

ImageView* const ImageGrid::AddBlankImage( const QSize& imageSize, int id )
{
    QImage newBlankImage( imageSize, QImage::Format_RGB888 );
    newBlankImage.fill( 0xFFFFFF );

    ImageView* const addedImageView = AddImage( newBlankImage, QString(), id );
    return addedImageView;
}

/** @brief Perform zoom on Qt GUI mouse wheel event + Control Key.
 *
 *  @param event The information about the amount of scrolling to perform & modifier keys.
 */
void ImageGrid::wheelEvent( QWheelEvent* event )
{
    if ( event->modifiers() == Qt::CTRL )
    {
        const double zoomScale = 0.001;
        m_zoom += zoomScale*event->delta();
        m_zoom = std::max( 1.0, m_zoom );
        ReflowImages();

        for ( size_t i = 0; i < m_imageViews.size(); ++i )
        {
            m_imageViews[i].get()->setZoom( m_zoom );
        }

        event->accept();
    }
    else
    {
        QScrollArea::wheelEvent( event );
    }
}

void ImageGrid::updateImage( int id, const QImage& image, double fps )
{
    m_imageViews[id].get()->SetImage( image );
    m_imageViews[id].get()->SetCaption( QString( "FPS: %1" ).arg( fps, 5, 'f', 2 ) );
    m_imageViews[id].get()->update();
}


