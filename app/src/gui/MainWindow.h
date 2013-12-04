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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Workbench.h"

#include <QtGui/QMainWindow>

#include <memory>

class WorkbenchUi;
class HelpViewer;
class QToolButton;

namespace Ui
{
    class MainWindow;
}

/**
 * @brief Owns the main UI window that is found inside the application
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Initialises the MainWindow with a pointer to the main application.
     * @param parent is the owning widget (the application in this case)
     */
    explicit MainWindow(QWidget* parent = 0);
    virtual ~MainWindow();

    /**
     * @brief Refreshes the MainWindow UI
     */
    void Reload();

    /**
     * @brief When the config is reloaded, the unknown path elements are filled out based
     * on the current set up.
     * @param Contains the unknown element
     */
    void MergeWithActivePath(const WbPath& desiredPath);

    /**
     * @brief Sets up MainWindow's layout, Signal & Slots as well as Actions
     */
    void Start();

protected:
    /**
     * @brief Received when the main window is closed.
     * @param Close event
     */
    virtual void closeEvent(QCloseEvent* event);

private slots:
    /**
     * @brief Launches Help Dialog
     */
    void ShowHelp();

    /**
     * @brief Launches About GTS Dialog with Version Number, Date and Open Source Licenses
     */
    void ShowAboutGTS();

    /**
     * @brief Launches About QT Dialog
     */
    void ShowAboutQt();

private:
    Ui::MainWindow* m_ui;
    WorkbenchUi*    m_workbenchUi;
    HelpViewer*     m_helpViewer;
};

#endif // MAINWINDOW_H
