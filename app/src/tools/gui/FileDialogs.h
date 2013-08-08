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

/// @brief Functions to create standard dialog types
namespace FileDialogs
{
    const QString GetEmptyDirectoryName( QWidget* const parent,
                                         const QString& title,
                                         const QDir&    startDirectory  );

    class ExtendedFileDialog : public QFileDialog
    {
        Q_OBJECT

    public:
        ExtendedFileDialog( QWidget* const parent,
                            const QString& title,
                            const QString& startingDirectory,
                            const QString& fileFilters,
                            const bool singleFile );

        bool RelativeSelected() const;
        bool CopyFileSelected() const;

    private:
        void CreateCheckBoxes();
        void CreateCheckBox( QCheckBox*& checkBox,
                             const QString& title,
                             const Qt::CheckState& initialState,
                             const QString& toolTip );

        QCheckBox* m_copyFilesCheckBox;
        QCheckBox* m_relativeCheckBox;
    };
}

#endif // FILEDIALOGS_H_
