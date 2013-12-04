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

#include "WbPath.h"

#include <QtGui/QWidget>
#include <QtGui/QMenu>
#include <QtGui/QTreeWidget>

#include <memory>

namespace Ui
{
    class WorkbenchUiClass;
}

class Workbench;
class CameraHardware;
class MainWindow;
class HelpViewer;
class ToolTabsContainerWidget;

/**
 * @brief Manages the Workbench logic
 */
class WorkbenchUi : public QWidget
{
    Q_OBJECT

public slots:
    /**
      @brief Try to open last used workbench.
      If this fails, call NewOrOpenWorkbenchQuestion() to ask user
     */
    void OpenWorkbench();

    /**
      @brief Launch dialog to select where to place new workbench and call save with new config
     */
    void NewWorkbench();

    /**
      @brief Save workbench to disk and refresh UI
     */
    void SaveWorkbench();

    /**
      @brief Signal that saved config and in-memory config are different
      and call save
     */
    void ConfigChanged();

public:
    /**
      @brief Set up the workbench UI
      @param mainWindow Reference to the Main Window of the app that holds this workbench sub-ui.
     */
    WorkbenchUi(MainWindow& mainWindow);

    void SetToolMenu(QMenu& toolMenu);
    void Reload(const bool updateWorkbench = true);
    void MergeWithActivePath(const WbPath& desiredPath);
    bool HasOpenModifiedWorkbench() const;

private slots:
    void TreeCurrentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void TreeSelectionChanged();

private:
    const WbPath GetActivePathFromTools();
    const QString GetWorkbenchToOpenAtStartup() const;
    const QString TryToGetWorkbenchFromCmdLineArgs() const;
    const QString TryToGetWorkbenchFromInPlaceConfigFile() const;

    void SwitchBackToItemAfterSelectionChanges(QTreeWidgetItem* const item);
    void CreateToolTabs(MainWindow& mainWindow);
    void CreateTools(MainWindow& mainWindow);

    /**
      @brief set splitter to collapse workbench tree completely at start
     **/
    void SetupSplitter();

    /**
      @brief Reset the UI for the tool tabs, set up a new schema
     */
    void SetupWorkbench();

    /**
      @brief Connect Signals and Slots for Workbench UI
    **/
    void ConnectSignals();

    /**
      @brief If no workbench found, present user with a dialog to create
      new or open existing workbench
     */
    void NewOrOpenWorkbenchQuestion();

    /**
      @brief Open a workbench with exact path
      @param workbenchConfigFileName File path to workbench.xml file
     */
    void OpenWorkbench(const QString& workbenchConfigFileName);

    /**
      @brief Set up workbench schema (in-memory Key-Value config)
     */
    void SetUpWorkbenchSchema();

    std::auto_ptr< Ui::WorkbenchUiClass >   m_ui;
    std::auto_ptr< Workbench >              m_workbench;
    std::auto_ptr< CameraHardware >         m_cameraHardware;

    QMenu*                                  m_toolMenu;
    ToolTabsContainerWidget*                m_toolTabs;
    QTreeWidgetItem*                        m_itemToSwitchBackTo;
    WbPath                                  m_activePath;
    MainWindow&                             m_mainWindow;

    bool m_currentlyLoadedWorkbench;
};

#endif // WORKBENCHUI_H
