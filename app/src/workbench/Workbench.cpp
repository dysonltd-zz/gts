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

#include "Workbench.h"
#include <iostream>
#include "ToolTabsContainerWidget.h"
#include "WbSchema.h"
#include "XmlConfigFileReader.h"
#include <QtGui/QMessageBox>
#include "XmlConfigFileWriter.h"
#include "Message.h"

Workbench::Workbench( QTreeWidget& treeWidget, ToolContainer& toolContainer ) :
    m_treeWidget( treeWidget ),
    m_toolContainer( toolContainer ),
    m_handlerTools(),
    m_workbenchConfig()
{
}

Workbench::~Workbench()
{
}

bool Workbench::Open( const QFileInfo& configFileInfo )
{
    WbConfig openedConfig( m_schema, configFileInfo );

    XmlConfigFileReader reader;
    QFile configFile( configFileInfo.absoluteFilePath() );

    bool successful = openedConfig.ReadUsing( reader );

    if ( successful )
    {
        SwitchConfig( openedConfig );
    }

    return successful;
}

bool Workbench::New( const QFileInfo& configFileInfo )
{
    WbConfig newConfig( m_schema, configFileInfo );

    const bool successful = TryWriteConfig( newConfig );

    if ( successful )
    {
        SwitchConfig( newConfig );
    }
    return successful;
}

bool Workbench::Save()
{
    return TryWriteConfig( m_workbenchConfig );
}

bool Workbench::TryWriteConfig( const WbConfig& config )
{
    XmlConfigFileWriter writer;
    QFile configFile( config.GetAbsoluteFileInfo().absoluteFilePath() );
    return config.WriteUsing( writer );
}

const WbSchema Workbench::Schema() const
{
    return m_schema;
}

void Workbench::ReloadConfig()
{
    m_treeWidget.clear();
    m_workbenchConfig.AddTo( m_treeWidget );
}

void Workbench::SwitchConfig( const WbConfig& newConfig )
{
    m_workbenchConfig = newConfig;
    ReloadConfig();
}

bool Workbench::ActivateToolFor( const QTreeWidgetItem& newItem )
{
    bool succeeded = false;
    if ( m_toolContainer.ActiveToolCanClose() )
    {
        const WbConfig itemsConfig( WbConfig::FromTreeItem( newItem ) );
        succeeded = m_toolContainer.TryToOpenTool( itemsConfig );
    }
    else
    {
        m_toolContainer.ShowCannotCloseMessage();
    }

    return succeeded;
}

void Workbench::SetSchema( const WbSchema& newSchema )
{
    m_schema = newSchema;
}

const WbConfig Workbench::GetCurrentConfig() const
{
    return m_workbenchConfig;
}
