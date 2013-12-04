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

#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QtGui/QDialog>
#include <QtGui/QTextEdit>

#include <opencv/cv.h>
#include <opencv/highgui.h>

namespace Ui
{
    class ImageViewer;
}

/**
  @brief Display a QImage or IplImage in a QDialog box
 */
class ImageViewer : public QDialog
{
    Q_OBJECT
    
public:
    /**
     * @brief Initialises and displays dialog with a QImage
     * @param image QImage to display
     * @param parent The parent widget that is launching this dialog
     */
    explicit ImageViewer(const QImage& image, QWidget *parent = 0);

    /**
     * @brief Initialises and displays dialog with an old OpenCV style IplImage
     * @param image IplImage (OpenCV) to display
     * @param parent The parent widget that is launching this dialog
     */
    explicit ImageViewer(const IplImage* image, QWidget *parent = 0);

    /**
     * @brief Initialises and displays dialog with C++ OpenCV Mat
     * @param image CV::Mate (OpenCV) to display
     * @param parent The parent widget that is launching this dialog
     */
    explicit ImageViewer(const cv::Mat *image, QWidget *parent = 0);
    virtual ~ImageViewer();
    
private:
    /**
      @brief Replace current QImage if one exists and display in dialog
      @param image QImage
     */
    void ShowImage(const QImage& image);

    /**
      @brief Convert IplImage to QImage and replace current one if it exists and display in dialog
      @param image IplImage
     */
    void ShowImage(const IplImage* image);

    /**
      @brief Convert cv::Mat to QImage and replace current one if it exists and display in dialog
      @param image cv::Mat
     */
    void ShowImage(const cv::Mat* image);

    /**
      @brief Set up signal to handle accept button being clicked
     */
    void Connectsignals();

    Ui::ImageViewer* m_ui;
};

#endif // IMAGEVIEWER_H
