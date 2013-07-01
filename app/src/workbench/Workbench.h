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

#ifndef WORKBENCH_H_
#define WORKBENCH_H_

#include <memory>

#include <QtGui/qtreewidget.h>

#include "WbConfig.h"
#include <QtCore/qfileinfo.h>

class ToolContainer;
class ToolInterface;
class WbConfig;

/** @brief Class to load and save the configuration
 */
class Workbench
{
public:
    Workbench( QTreeWidget& treeWidget, ToolContainer& toolContainer );
    virtual ~Workbench();

    bool Open( const QFileInfo& configFileInfo );
    bool New ( const QFileInfo& configFileInfo );
    bool Save();

    bool ActivateToolFor( const QTreeWidgetItem& newItem );
    void ReloadConfig();

    void SetSchema( const WbSchema& newSchema );
    const WbConfig GetCurrentConfig() const;

    const WbSchema Schema() const;

private:
    bool TryWriteConfig( const WbConfig& config );
    void SwitchConfig( const WbConfig& newConfig );

    QTreeWidget& m_treeWidget;
    ToolContainer& m_toolContainer;

    std::map< QString, ToolInterface* > m_handlerTools;

    WbSchema m_schema;
    WbConfig m_workbenchConfig;
};

#endif // WORKBENCH_H_
