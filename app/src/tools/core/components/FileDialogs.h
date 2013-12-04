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

#ifndef FILEDIALOGS_H_
#define FILEDIALOGS_H_

#include <QtGui/QMessageBox>
#include <QtCore/QString>
#include <QtGui/QFileDialog>
#include <QtGui/QLayout>
#include <QtGui/QCheckBox>

#include "WbConfigTools.h"

/**
  @brief Functions to create standard dialog types
**/
namespace FileDialogs
{
    /**
      @brief Get the name of an empty directory to put new files into.
      Gets a directory name using a standard file dialog.  If the directory selected is empty,
      just returns the absolute path of the directory. If the directory is not empty, warn that
      folder contents will be removed, And allow user to decline.  Keep asking for a directory
      until an empty directory is chosen, the user accepts removal of a directory's contents
      (which is performed here too, or the user cancels.

      @param parent The parent of the file dialog
      @param title  The string to use in the title bar
      @param startDirectory The directory to open when the dialog starts
      @return The name of an empty directory, or the empty string if the user cancels.
    **/
    const QString GetEmptyDirectoryName(QWidget* const parent,
                                        const QString& title,
                                        const QDir& startDirectory );




    class ExtendedFileDialog : public QFileDialog
    {
        Q_OBJECT

    public:
        ExtendedFileDialog(QWidget* const parent,
                           const QString& title,
                           const QString& startingDirectory,
                           const QString& fileFilters,
                           const bool singleFile);

        bool RelativeSelected() const;
        bool CopyFileSelected() const;

    private:
        void CreateCheckBoxes();
        void CreateCheckBox(QCheckBox*& checkBox,
                            const QString& title,
                            const Qt::CheckState& initialState,
                            const QString& toolTip);

        QCheckBox* m_copyFilesCheckBox;
        QCheckBox* m_relativeCheckBox;
    };
}

#endif // FILEDIALOGS_H_
