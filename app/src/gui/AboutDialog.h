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

#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QtGui/QDialog>
#include <QtGui/QTextEdit>

namespace Ui
{
    class AboutDialog;
}
/**
  @brief Handles the content and controls for the About GTS Dialog
 */
class AboutDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit AboutDialog(QWidget *parent = 0);
    ~AboutDialog();
    
private:
    /**
      @brief Sets the version number, date and licenses for About Dialog
      @param The text box to be filled with relevant information
     */
    void UpdateAboutText(QTextEdit* const aboutTextBox);
    Ui::AboutDialog* m_ui;
};

#endif // ABOUTDIALOG_H
