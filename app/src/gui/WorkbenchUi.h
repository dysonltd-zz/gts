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

#ifndef WORKBENCHUI_H
#define WORKBENCHUI_H

#include <QtGui/QWidget>
#include <memory>
#include <QtGui/QMenu>
#include <QtGui/qtreewidget.h>
#include "WbPath.h"

namespace Ui
{
    class WorkbenchUiClass;
}
class Workbench;
class CameraHardware;
class MainWindow;
class HelpViewer;
class ToolTabsContainerWidget;

class WorkbenchUi : public QWidget
{
    Q_OBJECT

public:
    WorkbenchUi( MainWindow& mainWindow );
    virtual ~WorkbenchUi();

    void SetToolMenu( QMenu& toolMenu );
    void Reload( const bool updateWorkbench = true );
    void MergeWithActivePath( const WbPath& desiredPath );
    void SetCornerWidget( QWidget* const widget );
    void SetHelpViewer( HelpViewer& helpViewer );

    bool HasOpenModifiedWorkbench() const;

public slots:
    void OpenWorkbench();
    void NewWorkbench();
    void SaveWorkbench();

private slots:
    void TreeCurrentItemChanged( QTreeWidgetItem *current, QTreeWidgetItem *previous );
    void TreeSelectionChanged();

private:
    const WbPath GetActivePathFromTools();

    void SwitchBackToItemAfterSelectionChanges( QTreeWidgetItem* const item );

    void CreateToolTabs(MainWindow & mainWindow);
    void CreateTools(MainWindow & mainWindow);
    void SetupSplitter();
    void SetupWorkbench();
    void OpenWorkbench( const QString & workbenchConfigFileName );
    void SetUpWorkbenchSchema();
    void ConnectSignals();

    const QString GetWorkbenchToOpenAtStartup() const;
    const QString TryToGetWorkbenchFromCmdLineArgs() const;
    const QString TryToGetWorkbenchFromInPlaceConfigFile() const;

    std::auto_ptr< Ui::WorkbenchUiClass > m_ui;
    std::auto_ptr< Workbench >            m_workbench;
    std::auto_ptr< CameraHardware >       m_cameraHardware;

    QMenu* m_toolMenu;
    HelpViewer* m_helpViewer;
    ToolTabsContainerWidget* m_toolTabs;
    QTreeWidgetItem* m_itemToSwitchBackTo;
    WbPath m_activePath;
    MainWindow& m_mainWindow;
};

#endif // WORKBENCHUI_H
