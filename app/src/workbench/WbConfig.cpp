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

#include "WbConfig.h"

#include <cassert>
#include <string>

#include <QtCore/qdir.h>

#include "WbConfigTools.h"
#include "WbConfigFileReader.h"
#include "WbConfigFileWriter.h"
#include "WbDefaultKeys.h"
#include "WbPath.h"
#include "Container.h"

#include "Debugging.h"

#if !defined(__MINGW32__) && !defined(__GNUC__)
#include <functional>
#endif

GloballyUniqueId WbConfig::WbConfigData::globallyUniqueId = 0;

/** Construct a new WbConfigData structure.
 *
 *  It is assigned a new, globally unique id (unique amongst WbConfigData objects).
 */
WbConfig::WbConfigData::WbConfigData() :
    m_schema                   (),
    m_subConfigs               (),
    m_parent                   ( new WbConfig ),
    m_keyValues                (),
    m_possiblyRelativeFileInfo (),
    m_treeWidgetItem           ( 0 ),
    m_modified                 ( false ),
    m_id                       ( globallyUniqueId++ )
{}

//================================================

WbConfig::GlobalRegisterType WbConfig::globalRegister;

/** @brief Constructs a null WbConfig.
 */
WbConfig::WbConfig()
{
    //null config
}

/** @brief Constructs a new WbConfig with new data.
 *
 *  It also registers a copy of @a this in the global register of configurations.
 *  The default values specified by the schema are set into the config.
 *  @param schema The schema followed by this config file.
 *  @param fileInfo The location of the config file on disk.
 */
WbConfig::WbConfig( const WbSchema& schema, const QFileInfo& fileInfo ) :
    m_internalData( new WbConfigData )
{
    globalRegister[ QueryData().m_id ] = *this; /// @todo We never clear this register so it
                                                /// contains all configs used since we started
                                                /// the app - even if we've changed workbench.

    ModifyData().m_schema = schema;
    ModifyData().m_possiblyRelativeFileInfo = fileInfo;
    SetDefaults();
}

/// Destructor
WbConfig::~WbConfig()
{
}

//------------------------------------------------

/** @brief Create a sub-config file for the key specified.
 *
 *  @param name The name of sub-schema, and the key where the sub config will be stored.
 *  @param fileName The file where the sub-config is on disk.
 *  The file name can be absolute, or relative to the parent config file (i.e. this one).
 *  @param id The ID used to distinguish keys with the same name.
 *  @return The newly created sub-config, or a null config if this one is null.
 */
WbConfig WbConfig::CreateSubConfig( const KeyName& name,
                                    const QString& fileName,
                                    const KeyId& id )
{
    if ( !IsNull() )
    {
        WbConfig subConfig( QueryData().m_schema.FindSubSchema( name ), QFileInfo( fileName ) );
        subConfig.SetParent( *this );
        subConfig.SetListener( QueryData().m_listener.get() );
        ModifyData().m_subConfigs.SetKeyValue( name, subConfig, id );
        return subConfig;
    }

    return WbConfig();
}

WbConfig WbConfig::AddSubConfig( const KeyName& name,
                                 const QString& fileName,
                                 const QString& keyIdFormat )
{
    return CreateSubConfig( name, fileName, GenerateNewId( name, keyIdFormat ) );
}

/** @brief Return the stored file info representing this config file's location on disk.
 *
 * The file location may be absolute or relative to its parent's location.
 * @return The possibly relative file info for this config's location on disk,
 * or a null QFileInfo if this is null.
 */
const QFileInfo WbConfig::GetPossiblyRelativeFileInfo() const
{
    if ( !IsNull() )
    {
        return QueryData().m_possiblyRelativeFileInfo;
    }

    return QFileInfo();
}

/** @brief Return the absolute file info representing this config file's location on disk.
 *
 * The file location will always be absolute (by resolving the location relative to the
 * parent if necessary).
 * @return The absolute file info for this config's location on disk,
 * or a null QFileInfo if this is null.
 */
const QFileInfo WbConfig::GetAbsoluteFileInfo() const
{
    QFileInfo absoluteFileInfo;

    if ( !IsNull() )
    {
        absoluteFileInfo =
            QueryData().m_parent->GetAbsoluteFileInfoFor( GetPossiblyRelativeFileInfo() );
    }

    return absoluteFileInfo;
}

/** @brief Return the absolute file info representing the location on disk,
 *  specified by the argument
 *
 * The file location will always be absolute (by resolving the location relative to this
 * & its parent if necessary), unless this is a null config, when it is returned unchanged.
 * If the @a possiblyRelativeFileInfo is already absolute it is returned unchanged.
 *
 * @param possiblyRelativeFileInfo A file info for a file to be resolved relative to
 * this config.
 *
 * @return The absolute file info for the location on disk,
 * or @a possiblyRelativeFileInfo if this is a null config.
 */
const QFileInfo
WbConfig::GetAbsoluteFileInfoFor( const QFileInfo& possiblyRelativeFileInfo ) const
{
    QFileInfo absoluteFileInfo( possiblyRelativeFileInfo );

    if ( !IsNull() )
    {
        if ( possiblyRelativeFileInfo.isRelative() )
        {
            absoluteFileInfo.setFile( GetAbsoluteFileInfo().dir(),
                                      possiblyRelativeFileInfo.filePath() );
            absoluteFileInfo.setFile( QDir::cleanPath( absoluteFileInfo.filePath() ) );
        }
    }

    return absoluteFileInfo;
}

/** @brief Return the absolute file name for the file in the argument
 *
 * The file name will always be absolute (by resolving the location relative to this
 * & its parent if necessary), unless this is a null config or it is empty,
 *  when it is returned unchanged.
 * If the @a possiblyRelativeFileName is already absolute it is returned unchanged.
 *
 * @param possiblyRelativeFileName A file name to be resolved relative to
 * this config.
 *
 * @return The absolute file name for the file, or @a possiblyRelativeFileName if
 * this is a null config or @possiblyRelativeFileName is empty.
 */
const QString
WbConfig::GetAbsoluteFileNameFor( const QString& possiblyRelativeFileName ) const
{
    QString absoluteFileName( possiblyRelativeFileName );

    if ( !IsNull() && !possiblyRelativeFileName.isEmpty() )
    {
        const QFileInfo possiblyRelativeFileInfo( possiblyRelativeFileName );
        const QFileInfo absoluteFileInfo( GetAbsoluteFileInfoFor( possiblyRelativeFileInfo ) );
        absoluteFileName = absoluteFileInfo.absoluteFilePath();
    }

    return absoluteFileName;
}

/** @brief Retrieve the sub-config file for the key specified.
 *
 *  @param name The key where the sub-config is stored.
 *  @param id The ID used to distinguish keys with the same name.
 *  @return The requested sub-config, or a null config if this one is null,
 *  or if the specified key does not contain a sub-config.
 */
WbConfig WbConfig::GetSubConfig( const KeyName& name, const KeyId& id ) const
{
    if ( !IsNull() )
    {
        return QueryData().m_subConfigs.GetKeyValue( name, id );
    }

    return WbConfig();
}

/** @brief Remove all the values with this key name, except for those
 *  with the IDs specified.
 *
 *  No effect if this is a null config.

 * @param keyName    The name of the keys the operation should affect
 * @param idsToKeep  The IDs which should @em not be removed
 */
void WbConfig::RemoveOldKeys( const KeyName& keyName,
                              const std::vector<KeyId>& idsToKeep )
{
    if ( !IsNull() )
    {
        WbKeyValues::ValueIdPairList values( GetKeyValues( keyName ) );
        for ( size_t valueIndex = 0; valueIndex < values.size(); ++valueIndex )
        {
            const KeyId thisValueId( values.at( valueIndex ).id );
            if ( !container( idsToKeep ).contains( thisValueId ) )
            {
                ModifyData().m_keyValues.RemoveKeyValue( keyName, thisValueId );
            }
        }
    }
}

/** @brief Set all the key values specified into this config.
 *
 *  No effect if this is a null config.
 *
 *  @param keyValues The key values to set.
 */
void WbConfig::SetKeyValues( const WbKeyValues& keyValues )
{
    if ( !IsNull() )
    {
        ModifyData().m_keyValues.SetKeys( keyValues );
    }
}

/** @brief Set the value of the key specified into this config.
 *
 *  No effect if this is a null config.  If the key value is the same as the current one
 *  it is not set - to avoid marking the config as modified
 *  @param keyName The name of the key to set.
 *  @param value The value to set in that key.
 *  @param keyId The ID used to distinguish keys with the same name.
 */
void WbConfig::SetKeyValue( const KeyName& keyName,
                            const KeyValue& value,
                            const KeyId& keyId )
{
    if ( !IsNull() )
    {
        if ( GetKeyValue( keyName, keyId ) != value )
        {
            ModifyData().m_keyValues.SetKeyValue( keyName, value, keyId );
        }
    }
}

/** @brief Retrieve the value of the key specified.
 *
 *  @param keyName The name of the key to get.
 *  @param keyId The ID used to distinguish keys with the same name.
 *  @return The value of the specified key, or a null KeyValue if this is a null config,
 *  or the key does not exist.
 */
const KeyValue WbConfig::GetKeyValue( const KeyName& keyName, const KeyId& keyId ) const
{
    if ( !IsNull() )
    {
        return QueryData().m_keyValues.GetKeyValue( keyName, keyId );
    }

    return  KeyValue();
}

/** @brief Retrieve the values of all keys with the specified name.
 *
 *  @param keyName The name of the keys to get.
 *  @return A WbKeyValues::ValueIdPairList of the keys with the specified name for all
 *  IDs.  The list is empty if this is a null config, or the key does not exist.
 */
const WbKeyValues::ValueIdPairList WbConfig::GetKeyValues( const KeyName& keyName ) const
{
    if ( !IsNull() )
    {
        return QueryData().m_keyValues.GetKeyValues( keyName );
    }

    return WbKeyValues::ValueIdPairList();
}

/** @brief Retrieve the sub-configs of with the specified key name.
 *
 *  @param keyName The key name of the sub-configs to get.
 *  @return A WbKeyValues::ValueIdPairList of the sub-configs with the specified name
 *  for all IDs.  The list is empty if this is a null config, or the key does not exist.
 */
const WbConfig::SubConfigs::ValueIdPairList WbConfig::GetSubConfigs( const KeyName& keyName ) const
{
    if ( !IsNull() )
    {
        return QueryData().m_subConfigs.GetKeyValues( keyName );
    }

    return SubConfigs::ValueIdPairList();
}

/** @brief Write out the config file using the WbConfigFileWriter provided.
 *
 *  @bug [minor] creates directories even if writer chooses not to actually do any
 *  file writing ( e.g. OStreamConfigFileWriter).  Should really be writer's
 *  responsibility to create directory & file
 *
 *  @param writer The WbConfigFileWriter to use to write the contents.
 *  @return @a true if the data was successfully written, @a false otherwise. If this
 *  is null no writing takes place and @a false is returned.
 */
bool WbConfig::WriteUsing( WbConfigFileWriter& writer ) const
{
    bool successful = false;

    if ( !IsNull() )
    {
        successful = QueryData().m_schema.WriteTo( writer, *this );

        QFileInfo absoluteFileInfo( GetAbsoluteFileInfo() );

        if ( successful )
        {
            QDir absoluteDirectory( absoluteFileInfo.absoluteDir() );
            successful = absoluteDirectory.mkpath( absoluteDirectory.path() );
        }

        if ( successful )
        {
            QFile configFile( absoluteFileInfo.absoluteFilePath() );

            successful = writer.WriteTo( configFile );
        }
    }

    if ( successful )
    {
        ModifyMutableData().m_modified = false;
    }

    return successful;
}

/** @brief Read in the config file using the WbConfigFileReader provided.
 *
 *  The current data is cleared first, and defaults are set,
 *  so that only the data in the file is contained differs from the config defaults
 *  after reading.
 *  @param reader The WbConfigFileReader to use to reader the contents.
 *  @return @a true if the data was successfully read, @a false otherwise. If this is null
 *  no reading takes place and @ false is returned.
 */
bool WbConfig::ReadUsing( WbConfigFileReader& reader )
{
    Clear();
    SetDefaults();

    bool successful = false;

    if ( !IsNull() )
    {
        QFileInfo absoluteFileInfo( GetAbsoluteFileInfo() );
        QFile configFile( absoluteFileInfo.absoluteFilePath() );

        successful = reader.ReadFrom( configFile );

        if ( successful )
        {
            successful = ModifyData().m_schema.ReadFrom( reader, *this );
        }
    }

    if ( successful )
    {
        ModifyMutableData().m_modified = false;
    }

    return successful;
}

/** @brief Sets the parent of this config file
 *
 *  No effect if this is a null config.
 *  @param parent The new parent to set.
 */
void WbConfig::SetParent( WbConfig& parent )
{
    if ( !IsNull() )
    {
        *ModifyData().m_parent = parent;
    }
}

void WbConfig::SetListener( ConfigListener* listener )
{
    if ( !IsNull() )
    {
        ModifyData().m_listener.reset( listener );

        std::vector< WbConfig > subConfigList( QueryData().m_subConfigs.EnumerateValues() );

        for ( size_t i = 0; i < subConfigList.size(); ++i )
        {
            subConfigList.at( i ).SetListener( listener );
        }
    }
}

/** @brief Clear all data stored in this config.
 *
 *  No effect if this is a null config.
 */
void WbConfig::Clear()
{
    if ( !IsNull() )
    {
        ModifyData().m_subConfigs.Clear();
        ModifyData().m_keyValues.Clear();
    }
}

/** @brief Return the name of the schema followed by this config.
 *
 *  @return The name of the schema followed by this config, or a null KeyName
 *  if this is the null config.
 */
const KeyName WbConfig::GetSchemaName() const
{
    if ( !IsNull() )
    {
        return QueryData().m_schema.Name();
    }

    return KeyName();
}

/** @brief Add a node representing this config file to the @a QTreeWidget.
 *
 *  Also add sub-configs as children of the item added.
 *  No effect if this is the null config.
 *  @param tree The tree to add an item to.
 */
void WbConfig::AddTo( QTreeWidget& tree ) const
{
    if ( !IsNull() )
    {
        QTreeWidgetItem* const myTreeItem = CreateTreeItem();
        tree.addTopLevelItem( myTreeItem );
        AddSubConfigsTo( *myTreeItem );
    }
}

/** @brief Add a node representing this config as a child of the QTreeWidgetItem.
 *
 *  Also add sub-configs as children of the child item added.
 *  No effect if this is the null config.
 *  @param treeItem The tree item to add a child to.
 */
void WbConfig::AddToItem( QTreeWidgetItem* const treeItem ) const
{
    assert( treeItem != 0 );

    if ( !IsNull() )
    {
        QTreeWidgetItem* const myTreeItem = CreateTreeItem();
        treeItem->addChild( myTreeItem );
        AddSubConfigsTo( *myTreeItem );
    }
}

/** @brief Create a @a QTreeWidgetItem representing this config.
 *
 *  The item is expected to be added to a @a QTreeWidget, so it is @em not owned
 *  (usually the @a QTreeWidget will delete its child items). The ID of this
 *  config's data is encoded in the user data of the item, so that the config
 *  can be located if the user selects an item in the tree.
 *
 *  @return A heap-allocated @a QTreeWidgetItem representing this config, or @a 0
 *  if this is the a null config.
 */
QTreeWidgetItem* const WbConfig::CreateTreeItem() const
{
    using namespace WbConfigTools;

    if ( !IsNull() )
    {
        QTreeWidgetItem* const treeItem = new QTreeWidgetItem;
        treeItem->setText( 0, DisplayNameOf( *this ) );
        treeItem->setData( 0, Qt::UserRole, QVariant( QueryData().m_id ) );
        ModifyMutableData().m_treeWidgetItem = treeItem;
        return treeItem;
    }

    return 0;
}

QTreeWidgetItem* const WbConfig::GetLinkedTreeItem() const
{
    if ( !IsNull() )
    {
        return QueryData().m_treeWidgetItem;
    }
    return 0;
}

/** @brief Add all sub-configs as children of the @a QTreeWidgetItem.
 *
 *  No effect if this is the null config.
 *  @param treeItem The tree item to add children to.
 */
void WbConfig::AddSubConfigsTo( QTreeWidgetItem& treeItem ) const
{
    if ( !IsNull() )
    {
        QueryData().m_subConfigs.ForEachOrderedByInsertion(
            std::bind2nd( std::mem_fun_ref( &WbConfig::AddToItem ), &treeItem ) );
    }
}

/** @brief Set all default values specified in the schema.
 *
 *  This will overwrite existing values.
 *  No effect if this is a null config.
 */
void WbConfig::SetDefaults()
{
    if ( !IsNull() )
    {
        ModifyData().m_schema.SetDefaultsTo( *this );
    }
}

/** @brief Locate and return the WbConfig corresponding to an QTreeWidgetItem
 *
 *  Use the user data set in CreateTreeItem() to determine the ID, then look up the config
 *  in a global register of configs.
 *
 *  @param item The @a QTreeWidgetItem containing the user data.
 *  @return the Corresponding WbConfig, or a null config is the ID was not registered.
 */
const WbConfig WbConfig::FromTreeItem( const QTreeWidgetItem& item )
{
    GloballyUniqueId configId = item.data( 0, Qt::UserRole ).value<GloballyUniqueId>();

    GlobalRegisterType::const_iterator itr = globalRegister.find( configId );

    if ( itr != globalRegister.end() )
    {
        return itr->second;
    }

    return WbConfig();
}

/** @brief Determine if @a other is the same data as this.
 *
 *  This means the configs are actually sharing data, and changes to other affect this,
 *  and vice-versa.
 *
 *  @param other The WbConfig to compare to.
 *  @return @a true if the configs are the same, @a false otherwise.
 */
bool WbConfig::IsTheSameAs( const WbConfig& other ) const
{
    if ( m_internalData.data() == other.m_internalData.data() )
    {
        return true;
    }

    return false;
}

/** @brief Return whether config is null.
 *
 */
bool WbConfig::IsNull() const
{
    return m_internalData.isNull();
}

/** @brief Get this config's parent
 *
 *  Parent is null config if it hasn't been set and is set when the config is created as a
 *  sub-config. If this is null config so is its parent.
 *  @return This config's parent.
 */
const WbConfig WbConfig::GetParent() const
{
    if ( !IsNull() )
    {
        return *( QueryData().m_parent );
    }

    return WbConfig();
}

/** @brief Find the first ancestor with a schema name given by @a ancestorSchemaName
 *
 *  Ancestor is null config if there is no ancestor with the appropriate schema, or if
 *  this config is null.
 *  @return This config's ancestor with the appropriate schema name. Returns @a *this if
 *  the schema name supplied matches our own.
 */
const WbConfig WbConfig::FindAncestor( const KeyName& ancestorSchemaName ) const
{
    WbConfig ancestorConfig( *this );

    while ( !ancestorConfig.IsNull() &&
            ( ancestorConfig.GetSchemaName() != ancestorSchemaName ) )
    {
        ancestorConfig = ancestorConfig.GetParent();
    }

    return ancestorConfig;
}

/** @brief Find the ancestor at the top of the hierarchy
 *
 *  Ancestor is null config if this config is null otherwise we follow the parent
 *  configs until the next is null;
 *
 *  @return Thhe hierarchy's root config, or a null config if this is null.
 */
const WbConfig WbConfig::FindRootAncestor() const
{
    WbConfig ancestorConfig( *this );

    while ( !ancestorConfig.GetParent().IsNull() )
    {
        ancestorConfig = ancestorConfig.GetParent();
    }

    return ancestorConfig;
}

/** @brief Find out if this config's is a descendant of the the named schema.
 *
 *  @return @a true if this config's schema is a descendant of the named one,
 *  @a false otherwise.
 *  Also false if this is null.
 */
bool WbConfig::SchemaIsDescendantOf( const KeyName& schemaName ) const
{
    return !FindAncestor( schemaName ).IsNull();
}

const KeyId WbConfig::FindSubConfigId( const WbConfig& subconfig ) const
{
    SubConfigs::ValueIdPairList
        subConfigs( GetSubConfigs( subconfig.GetSchemaName() ) );

    for ( size_t i = 0; i < subConfigs.size(); ++i )
    {
        if ( subConfigs.at( i ).value.IsTheSameAs( subconfig ) )
        {
            return subConfigs.at( i ).id;
        }
    }

    assert( !"Sub-config not found" );

    return KeyId();
}

const WbConfig WbConfig::GetFromPath( const WbPath& path ) const
{
    WbPath pathRemaining( path );
    WbConfig currentConfig( *this );
    WbPathElement currentElement( pathRemaining.PopFront() );

    if ( currentConfig.GetSchemaName() == currentElement.Name() )
    {
        currentElement = pathRemaining.PopFront();
    }

    while ( currentElement.Type() != WbPathElement::Type_UnknownId )
    {
        if ( currentElement.Type() == WbPathElement::Type_Unique )
        {
            currentConfig = currentConfig.GetSubConfig( currentElement.Name() );
        }
        else if ( currentElement.Type() == WbPathElement::Type_KnownId )
        {
            currentConfig = currentConfig.GetSubConfig( currentElement.Name(),
                                                        currentElement.KnownId() );
        }

        if ( pathRemaining.IsEmpty() )
        {
            break;
        }
        else
        {
            currentElement = pathRemaining.PopFront();
        }
    }

    return currentConfig;
}

const KeyId WbConfig::GenerateNewId( const KeyName& keyName,
                                     const QString& keyIdFormat ) const
{
    size_t counter = 0;
    KeyId newId;
    do
    {
        newId = KeyId( keyIdFormat.arg( counter ) );
        counter++;
    }
    while ( Contains( keyName, newId ) );

    return newId;
}

const KeyId WbConfig::AddKeyValue( const KeyName& keyName,
                                   const KeyValue& value,
                                   const QString& keyIdFormat )
{
    const KeyId newId( GenerateNewId( keyName, keyIdFormat ) );
    SetKeyValue( keyName, value, newId );
    return newId;
}

bool WbConfig::Contains( const KeyName& keyName, const KeyId& id ) const
{
    return !GetKeyValue( keyName, id ).IsNull() ||
           !GetSubConfig( keyName, id ).IsNull();
}

/** @brief Function to access the internal data in such a way as to modify the
 * non-mutable components, thus modifying the config.
 *
 * @return A reference to the internal data
 */
WbConfig::WbConfigData& WbConfig::ModifyData()
{
    assert( !IsNull() );
    ModifyMutableData().m_modified = true;

    if (QueryData().m_listener.get()) QueryData().m_listener->NotifyChanged();

    return *m_internalData;
}

/** @brief Function to access the internal data in such a way as to query the
 * the components. This does not modify the config.
 *
 * @return A const reference to the internal data
 */
const WbConfig::WbConfigData& WbConfig::QueryData() const
{
    assert( !IsNull() );
    return *m_internalData;
}

/** @brief Function to access the internal data in such a way as to modify the
 * mutable components.  This does not mark the config as modified.
 *
 * @return A const reference to the internal data
 */
const WbConfig::WbConfigData& WbConfig::ModifyMutableData() const
{
    return QueryData();
}

/** @brief Has this config been modified since the last time it was read or written?
 *
 * @return @a true if the config has been modified, otherwise @a false.
 */
bool WbConfig::IsModified() const
{
    if ( IsNull() ) return false;
    if ( QueryData().m_modified ) return true;

    std::vector< WbConfig > subConfigList( QueryData().m_subConfigs.EnumerateValues() );

    for ( size_t i = 0; i < subConfigList.size(); ++i )
    {
        if ( subConfigList.at( i ).IsModified() )
        {
            return true;
        }
    }

    return false;
}
