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

#include "Tool.h"

#include <iostream>

#include "WbDefaultKeys.h"
#include <QtCore/QStringBuilder>

#include "OStreamConfigFileWriter.h"
#include "ScopedQtSignalsBlocker.h"
#include "WbConfigTools.h"

const QString Tool::selectItemString( tr( "Select" ) );

/** @brief construct with the schema handled.
 *
 *  This schema does NOT include sub-schemas handled by sub-tools
 */
Tool::Tool( QWidget* const parent, const WbSchema& schema ) :
    QWidget( parent ),
    m_schema ( schema ),
    m_mappers()
{
}

bool Tool::TryToOpenTool( const WbConfig& config )
{
    WbConfig configToOpen( GetConfigToOpen( config ) );

    return !configToOpen.IsNull();
}

const WbConfig Tool::GetConfigToOpen( const WbConfig& config ) const
{
    WbConfig configToOpen;
    if ( CanHandleSchema( config.GetSchemaName() ) )
    {
        configToOpen = config;
    }

    if ( configToOpen.IsNull() )
    {
        configToOpen = config.FindAncestor(
            GetHandledSchema().GetMostSpecificSubSchema().Name() );
    }
    return configToOpen;
}

const WbSchema Tool::GetMostSpecificSubSchema() const
{
    return m_schema.GetMostSpecificSubSchema();
}

void Tool::UpdateToolMenu( QMenu& toolMenu )
{
    Q_UNUSED(toolMenu);
}

void Tool::AddFullWorkbenchSchemaSubTreeTo( WbSchema& parentSchema, const KeyName& schemaToAttachTo ) const
{
    const WbSchema fullSubTree( GetFullWorkbenchSchemaSubTree() );
    if ( !fullSubTree.IsNull() )
    {
        parentSchema.AddSubSchemaToSchema( fullSubTree,
                                           schemaToAttachTo,
                                           WbSchemaElement::Multiplicity::One,
                                           GetSubSchemaDefaultFileName() );
    }
}

const WbSchema Tool::GetFullWorkbenchSchemaSubTree() const
{
    return m_schema;
}

const WbSchema Tool::AddKeysRequiredByWorkbench( const WbSchema& schema,
                                                 const QString& defaultName )
{
    WbSchema newSchema( schema );
    newSchema.AddSingleValueKey( WbDefaultKeys::displayNameKey,
                                 WbSchemaElement::Multiplicity::One,
                                 KeyValue::from( defaultName ) );
    return newSchema;
}

const QString Tool::GetSubSchemaDefaultFileName() const
{
    return QString();
}

const QString Tool::Unnamed( const QString& typeName )
{
    return QObject::tr( "Unnamed " ) + typeName;
}

WbConfig& Tool::GetCurrentConfig()
{
    return m_currentConfig;
}

const WbConfig Tool::GetCurrentConfig() const
{
    return m_currentConfig;
}

const WbSchema Tool::CreateWorkbenchSubSchema( const KeyName& schemaName,
                                               const QString& defaultName )
{
    return AddKeysRequiredByWorkbench( WbSchema( schemaName ), defaultName );
}

void Tool::CallOnSelfAndActiveSubTools( ToolFunction& func )
{
    func( *this );
}

bool Tool::CanHandleSchema( const KeyName& schemaName ) const
{
    return GetHandledSchema().ContainsSchemaAnywhere( schemaName );
}

void Tool::SetCurrentConfigFrom(const WbConfig & config )
{
    m_currentConfig = GetConfigToOpen( config );
}

void Tool::Reload( const WbConfig& config )
{
    SetCurrentConfigFrom( config );
    SetEnabled( ShouldEnableForConfig( config ) );
    ReloadCurrentConfig();
}

void Tool::SetEnabled( const bool shouldEnable )
{
    Widget()->setEnabled( shouldEnable );
}

const KeyName Tool::EnableToolSchemaName() const
{
    return GetHandledSchema().GetMostSpecificSubSchema().Name();
}

bool Tool::ShouldEnableForConfig( const WbConfig& configToCheck ) const
{
    if ( configToCheck.FindAncestor( EnableToolSchemaName() ).IsNull() )
    {
        return false;
    }
    return true;
}

void Tool::BlockAllMapperSignalsUsing(BlockerVector& blockers)
{
    for (auto mapper = m_mappers.begin(); mapper != m_mappers.end(); ++mapper)
    {
        blockers.push_back(BlockerPtr(new ScopedQtSignalsBlocker(**mapper)));
    }
}

void Tool::ReloadCurrentConfig( const ConfigKeyMapper* const excludeMapper )
{
    BlockerVector blockAllMapperSignals;
    BlockAllMapperSignalsUsing( blockAllMapperSignals );

    FillOutComboBoxes( excludeMapper );
    for ( size_t i = 0; i < m_mappers.size(); ++i )
    {
        if ( m_mappers.at( i ).get() != excludeMapper )
        {
            m_mappers.at( i )->SetConfig( GetMappedConfig() );
        }
    }
    ReloadCurrentConfigToolSpecific();
}

void Tool::FillOutComboBoxes( const ConfigKeyMapper* const requester )
{
    for ( size_t i = 0; i < m_collectionCombos.size(); ++i )
    {
        CollectionCombo& collectionCombo( m_collectionCombos.at( i ) );

        QComboBox* const comboBox = collectionCombo.comboBox;

        if ( !requester || !requester->Maps( comboBox ) )
        {
            comboBox->clear();

            if ( collectionCombo.hasSelect )
            {
                comboBox->addItem( selectItemString );
            }
            const Collection::StatusType collectionStatus =
                        collectionCombo.collection.SetConfig( GetCurrentConfig() );
            if ( collectionStatus == Collection::Status_Ok )
            {
                WbConfigTools::FillOutComboBoxWithCollectionElements( *comboBox,
                                                            collectionCombo.collection );
            }
        }
    }
}

const WbSchema Tool::GetHandledSchema() const
{
    return m_schema;
}

const WbConfig Tool::GetMappedConfig() const
{
    return GetCurrentConfig();
}

void Tool::AddMapper( ConfigKeyMapper* const mapper )
{
    m_mappers.push_back(std::unique_ptr<ConfigKeyMapper>(mapper));

    QObject::connect( mapper,
                      SIGNAL( RequestCommit( ConfigKeyMapper& ) ),
                      this,
                      SLOT( CommitDataAndUpdate( ConfigKeyMapper& ) ) );
}

void Tool::CommitDataAndUpdate( ConfigKeyMapper& mapper )
{
    WbConfig& config = GetCurrentConfig();
    mapper.CommitData(config);
    config.ChangeCompleted();
    ReloadCurrentConfig( &mapper );
}

void Tool::Activated()
{
}

void Tool::RegisterCollectionCombo( QComboBox* const comboBox,
                                    const Collection& collection,
                                    const bool hasSelect )
{
    const CollectionCombo collectionCombo =
    {
         comboBox,
         collection,
         hasSelect
    };
    m_collectionCombos.push_back( collectionCombo );
}

bool Tool::CanClose() const
{
    return true;
}

const QString Tool::CannotCloseReason() const
{
    return QString();
}

void Tool::HighlightLabel( QLabel* label, bool highlight ) const
{
    if (highlight)
    {
        QPalette palette;
	    palette.setColor(QPalette::WindowText, Qt::red);
	    label->setPalette(palette);
    }
    else
    {
        label->setPalette(label->style()->standardPalette());
    }
}
