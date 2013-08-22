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

#ifndef TOOL_H
#define TOOL_H

#include "ToolInterface.h"
#include "Workbench.h"
#include "WbSchema.h"
#include "WbConfig.h"
#include "WbPath.h"
#include "ConfigKeyMapper.h"
#include "Collection.h"
#include "HelpViewer.h"

#include <vector>
#include <memory>

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtGui/QWidget>
#include <QtGui/QLabel>

class Workbench;
class ScopedQtSignalsBlocker;

template < class Type > class ManagedVector;

class Tool : public QWidget, public ToolInterface
{
    Q_OBJECT
public:
    Tool( QWidget* const parent, const WbSchema& schema );
    virtual ~Tool() {}

    virtual bool TryToOpenTool( const WbConfig& config );

    virtual void AddFullWorkbenchSchemaSubTreeTo( WbSchema& parentSchema,
                                                  const KeyName& schemaToAttachTo ) const;
    virtual void UpdateToolMenu( QMenu& toolMenu );
    virtual void CallOnSelfAndActiveSubTools( ToolFunction& func );

    virtual void Reload( const WbConfig& config );

    virtual const WbConfig GetCurrentConfig() const;

    virtual const HelpBookmark GetHelpText() const;
    virtual const WbSchema GetMostSpecificSubSchema() const;
    virtual QWidget* Widget() { return this; }
    void ReloadCurrentConfig( const ConfigKeyMapper* const excludeMapper = 0 );

    virtual bool CanClose() const;
    virtual const QString CannotCloseReason() const;

protected:

    virtual void SetEnabled( const bool shouldEnable );

    WbConfig& GetCurrentConfig();

    const WbSchema GetHandledSchema() const;
    bool CanHandleSchema( const KeyName& schemaName ) const;

    template< class WidgetClass >
    void AddMapper( const KeyName& keyToMap, WidgetClass* const widgetToMap );
    void AddMapper( ConfigKeyMapper* const mapper );

    static const QString Unnamed( const QString& typeName );
    static const WbSchema CreateWorkbenchSubSchema( const KeyName& schemaName,
                                                    const QString& defaultName );
    void RegisterCollectionCombo( QComboBox* const comboBox,
                                  const Collection& collection,
                                  const bool hasSelect = false );

    virtual void FillOutComboBoxes( const ConfigKeyMapper* const requester );

    static const QString selectItemString;

    virtual void Activated();

    void HighlightLabel( QLabel* label, bool highlight ) const;

private slots:
    void CommitDataAndUpdate( ConfigKeyMapper& mapper );

private:
    Tool( const Tool& );
    Tool& operator =( const Tool& );


    virtual const WbSchema GetFullWorkbenchSchemaSubTree() const;
    virtual const QString GetSubSchemaDefaultFileName() const;
    virtual const WbConfig GetMappedConfig() const;
    virtual void ReloadCurrentConfigToolSpecific() {}

    typedef std::unique_ptr<ScopedQtSignalsBlocker> BlockerPtr;
    typedef std::vector<BlockerPtr> BlockerVector;

    void BlockAllMapperSignalsUsing(BlockerVector& blockers);
    void SetCurrentConfigFrom( const WbConfig& config );

    const WbConfig GetConfigToOpen( const WbConfig& config ) const;
    bool ShouldEnableForConfig( const WbConfig& configToCheck ) const;
    const KeyName EnableToolSchemaName() const;

    static const WbSchema AddKeysRequiredByWorkbench( const WbSchema& schema,
                                                      const QString& defaultName );
    WbConfig m_currentConfig;
    const WbSchema m_schema;
    std::vector< std::unique_ptr<ConfigKeyMapper> > m_mappers;

    struct CollectionCombo
    {
        QComboBox* comboBox;
        Collection collection;
        bool       hasSelect;
    };

    std::vector< CollectionCombo > m_collectionCombos;
};

template< class WidgetClass >
void Tool::AddMapper( const KeyName& keyToMap, WidgetClass* const widgetToMap )
{
    AddMapper( new ConfigKeyMapperImplementation< WidgetClass >( keyToMap,
                                                                 widgetToMap ) );
}


#endif // TOOL_H
