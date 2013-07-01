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

#include "FileDialogs.h"

#include <QtGui/qfiledialog.h>

#include <cassert>

namespace FileDialogs
{
    namespace
    {
        const QDir::Filters ALL_ENTRIES_EXCEPT_DOT_ANT_DOTDOT =
            QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System;

        /** @brief ask the user if it's OK to delete a directories content
         *
         *  @param parent The parent of the message box.
         *  @return User's response.
         */
        const QMessageBox::StandardButton CheckIfCanDeleteContents( QWidget* const parent )
        {
            return QMessageBox::question( parent,
                                          QObject::tr( "Warning" ),
                                          QObject::tr( "This will remove all of the directory's "
                                                       "current contents. Continue?" ),
                                          QMessageBox::Yes | QMessageBox::No,
                                          QMessageBox::No );
        }

        /** @brief Remove all contents of a directory on disk
         *
         *  @param dir The directory whose contents should be removed
         */
        void RemoveDirectoryContents(QDir & dir)
        {
            const QFileInfoList entryInfoList(dir.entryInfoList(ALL_ENTRIES_EXCEPT_DOT_ANT_DOTDOT));
            for ( int i = 0; i < entryInfoList.size(); ++i )
            {
                const QFileInfo thisEntry(entryInfoList.at(i));
                if ( thisEntry.isFile() )
                {
                    dir.remove(thisEntry.absoluteFilePath());
                }
                else if ( thisEntry.isDir() )
                {
                        QDir subdir(thisEntry.absoluteFilePath());
                        RemoveDirectoryContents(subdir);
                        dir.rmdir(subdir.absolutePath());
                }
            }
        }
    }


    /** @brief Get the name of an empty directory to put new files into.
     *
     *  Gets a directory name using a standard file dialog.  If the directory selected is empty,
     *  just returns the absolute path of the directory. If the directory is not empty, warn that
     *  folder contents will be removed, And allow user to decline.  Keep asking for a directory
     *  until an empty directory is chosen, the user accepts removal of a directory's contents
     *  (which is performed here too, or the user cancels.
     *
     *  @param parent The parent of the file dialog
     *  @param title  The string to use in the title bar
     *  @param startDirectory The directory to open when the dialog starts
     *  @return The name of an empty directory, or the empty string if the user cancels.
     */
    const QString GetEmptyDirectoryName( QWidget* const parent,
                                         const QString& title,
                                         const QDir& startDirectory )
    {
        QString outputDirectoryName;
        QFileDialog videoDirectoryDialog(parent, title);
        videoDirectoryDialog.setOptions(QFileDialog::ShowDirsOnly | QFileDialog::HideNameFilterDetails);
        videoDirectoryDialog.setFileMode(QFileDialog::Directory);
        videoDirectoryDialog.setDirectory(startDirectory);
        int fileChooserResult = QDialog::Rejected;
        do
        {
            fileChooserResult = videoDirectoryDialog.exec();

            if ( fileChooserResult == QDialog::Accepted )
            {
                const QStringList chosenDirectoryNames(videoDirectoryDialog.selectedFiles());

                QDir directoryChosen(chosenDirectoryNames.at(0));

                QStringList entries(directoryChosen.entryList(ALL_ENTRIES_EXCEPT_DOT_ANT_DOTDOT));
                QMessageBox::StandardButton overwriteAnswer = QMessageBox::NoButton;
                if ( entries.count() != 0 )
                {
                    overwriteAnswer = CheckIfCanDeleteContents(&videoDirectoryDialog);
                }

                if ( ( entries.count() == 0 ) || ( overwriteAnswer == QMessageBox::Yes ) )
                {
                    outputDirectoryName = directoryChosen.absolutePath();
                    RemoveDirectoryContents(directoryChosen);
                }
            }

        } while ( outputDirectoryName.isEmpty() && ( fileChooserResult == QDialog::Accepted ) );

        return outputDirectoryName;
    }

    ExtendedFileDialog::ExtendedFileDialog( QWidget *const parent,
                                            const QString& title,
                                            const QString& startingDirectory,
                                            const QString& fileFilters,
                                            const bool singleFile )
    :
        QFileDialog(parent, title, startingDirectory, fileFilters),
        m_copyFilesCheckBox(0),
        m_relativeCheckBox(0)
    {
        if (singleFile)
        {
            setFileMode(QFileDialog::ExistingFile);
        }
        else
        {
            setFileMode(QFileDialog::ExistingFiles);
        }

        CreateCheckBoxes();
    }

    void ExtendedFileDialog::CreateCheckBoxes()
    {
        CreateCheckBox( m_copyFilesCheckBox,
                        tr( "Co&py Files" ),
                        Qt::Checked,
                        tr( "<p>Whether to copy files into the workbench directory,"
                            " or to link to an external location.</p><p>Default is to copy "
                            " files located outside the workbench directory, "
                            " and not copy those inside.</p>" ) );

        CreateCheckBox( m_relativeCheckBox,
                        tr("&Relative Paths"),
                        Qt::Checked,
                        tr("<p>Whether to use paths relative to the workbench directory, "
                           "or to use the full file path (when a file is externally located "
                           "and is not copied into the workbench directory.</p><p>Default is "
                           "to use full path.</p>"));
    }

    void ExtendedFileDialog::CreateCheckBox( QCheckBox*& checkBox,
                                             const QString& title,
                                             const Qt::CheckState& initialState,
                                             const QString& toolTip )
    {
        checkBox = new QCheckBox( title );
        checkBox->setCheckState( initialState );
        checkBox->setToolTip( toolTip );
        layout()->addWidget( checkBox );
    }

    bool ExtendedFileDialog::RelativeSelected() const
    {
        return m_relativeCheckBox->checkState() == Qt::Checked;
    }

    bool ExtendedFileDialog::CopyFileSelected() const
    {
        return m_copyFilesCheckBox->checkState() == Qt::Checked;
    }
}
