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

#include "ImageViewer.h"
#include "ui_ImageViewer.h"

#include <QtGui/QImage>

ImageViewer::ImageViewer(const QImage& image, QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::ImageViewer)
{
    m_ui->setupUi(this);

    ShowImage(image);

    QObject::connect( m_ui->m_okBtn,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( accept() ) );
}

ImageViewer::ImageViewer(const IplImage* image, QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::ImageViewer)
{
    m_ui->setupUi(this);

    ShowImage(image);

    QObject::connect( m_ui->m_okBtn,
                      SIGNAL( clicked() ),
                      this,
                      SLOT( accept() ) );
}

ImageViewer::~ImageViewer()
{
    delete m_ui;
}

void ImageViewer::ShowImage(const QImage& image)
{
    m_ui->m_view->Clear();
    m_ui->m_view->SetImage( image );
    m_ui->m_view->update();
}

void ImageViewer::ShowImage(const IplImage* image)
{
    // Convert image
    IplImage* imgTmp = cvCreateImage( cvSize( image->width, image->height ), IPL_DEPTH_8U, 3 );
    cvConvertImage( image, imgTmp );

    const QSize imgSize( imgTmp->width, imgTmp->height );
    QImage qImg = QImage( imgSize, QImage::Format_RGB888 );

    CvMat mtxWrapper;
    cvInitMatHeader( &mtxWrapper,
                     imgTmp->height,
                     imgTmp->width,
                     CV_8UC3,
                     qImg.bits() );

    cvConvertImage( imgTmp, &mtxWrapper, 0 );

    // Display image
	ShowImage(qImg);

	cvReleaseImage( &imgTmp );
}
