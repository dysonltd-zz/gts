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

#ifndef TOOLTABSCONTAINERWIDGET_H
#define TOOLTABSCONTAINERWIDGET_H

#include <QtGui/QTabWidget>
#include <vector>
#include <memory>
#include "ToolInterface.h"
#include "WbKeyValues.h"

class MainWindow;

class ToolContainer
{
public:
    virtual void AddTool(ToolInterface* const tool) = 0;
    virtual bool TryToOpenTool(const WbConfig& config) = 0;

    virtual void CallOnActiveTools(ToolFunction& func) = 0;

    virtual void AddToolsFullWorkbenchSchemaSubTreeTo(WbSchema& parentSchema) = 0;
    virtual void AddToolsFullWorkbenchSchemaSubTreeTo(WbSchema& parentSchema,
                                                       const KeyName& schemaToAddTo) = 0;

    virtual bool ActiveToolCanClose() const = 0;
    virtual const QString ActiveToolCannotCloseReason() const = 0;
    virtual void ShowCannotCloseMessage() = 0;
};

class ToolTabsContainerWidget : public QTabWidget, public ToolContainer
{
    Q_OBJECT

public:
    explicit ToolTabsContainerWidget(MainWindow* const mainWindow, QWidget* const parent = 0);
    virtual ~ToolTabsContainerWidget();

    void AddTool(ToolInterface* const tool);

    virtual void CallOnActiveTools(ToolFunction& func);
    virtual bool TryToOpenTool(const WbConfig & config);
    virtual void AddToolsFullWorkbenchSchemaSubTreeTo(WbSchema& parentSchema);
    virtual void AddToolsFullWorkbenchSchemaSubTreeTo(WbSchema& parentSchema,
                                                       const KeyName& schemaToAddTo);

    virtual bool ActiveToolCanClose() const;
    virtual const QString ActiveToolCannotCloseReason() const;
    virtual void ShowCannotCloseMessage();

private slots:
    void CurrentTabChanged(const int newTabIndex);

private:
    typedef std::shared_ptr<ToolInterface> ToolPtr;
    virtual const ToolPtr FindActiveTool() const;
    void ActivateTabWithoutInvokingSignals(size_t i);

    MainWindow* m_mainWindow;
    std::vector<ToolPtr> m_tools;
};

#endif // TOOLTABSCONTAINERWIDGET_H
