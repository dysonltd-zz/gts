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

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow( QWidget* parent = 0 );
    virtual ~MainWindow();

    void Reload();
    void MergeWithActivePath( const WbPath& desiredPath );
    void Start();

protected:
    virtual void closeEvent( QCloseEvent* event );

private slots:
    void ShowHelp();
    void ShowAboutGTS();
    void ShowAboutQt();

private:
    Ui::MainWindow* m_ui;
    WorkbenchUi* m_workbenchUi;
    HelpViewer*  m_helpViewer;
    QToolButton* m_cornerButton;
};

#endif // MAINWINDOW_H
