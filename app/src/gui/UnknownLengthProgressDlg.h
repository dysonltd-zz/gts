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

#ifndef UNKNOWNLENGTHPROGRESSDLG_H_
#define UNKNOWNLENGTHPROGRESSDLG_H_

#include <QtGui/QProgressBar>
#include <QtGui/QWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QCloseEvent>

/**
  @brief A dialog that displays an ever-loading loading bar, to be used
  when you don't know how lunch an operation is going to take or
  the stage of it's current progress.
  **/
class UnknownLengthProgressDlg : public QWidget
{
    Q_OBJECT

public:
    /**
      @brief Sets up layout and dialog items
      @param parent Widget calling this dialog
    **/
    UnknownLengthProgressDlg(QWidget* const parent = 0);

    /**
      @brief Starts the loading bar moving
      @param title Text to place in title bar
      @param message Text to display above loading bar
     */
    void Start(const QString& title, const QString& message);

    /**
      @brief Called after start. Replaces bar with title and text
      @param title Text to place in title bar
      @param message Text to display above loading bar
      @param filePath Used if you wish to present file location of outputs
      **/
    void Complete(const QString& title, const QString& message, const QString& filePath = "");
    void ForceClose();

protected:
    /**
      @brief Handles the close event fired when a user quits the dialog
      @param event Close event received from widget
     **/
    virtual void closeEvent(QCloseEvent* event);

private slots:
    /**
      @brief Wrapper to call OS specific explorer/finder to display output if user presses 'Open'
      @param dirPath File path to directory of relevant outputs
     **/
    void ShowInGraphicalShell(const QString &dirPath);

private:
    /**
      @brief Relaces just the dialog's main message
      @param message Text to be displayed
     */
    void SetLabelText(const QString& message);

    /**
      @brief Resizes widget accordingly
     **/
    void AdjustGeometry();

    QProgressBar* m_bar;
    QGridLayout*  m_layout;
    QLabel*       m_label;
    bool          m_allowClose;
    QString*      m_filePath;
};


#endif

