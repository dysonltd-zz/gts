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

#ifndef WBCONFIG_H
#define WBCONFIG_H

#include "WbSchema.h"

#include "ConfigListener.h"

#include <QtCore/QStringList>
#include <QtCore/QFileInfo>
#include <QtCore/QSharedPointer>
#include <QtGui/QTreeWidget>

#include <memory>
#include <map>

typedef unsigned int GloballyUniqueId;


/** @brief An abstraction of configuration data stored as elements of type
 *  KeyValue, and indexed by a KeyName and a KeyId.
 *
 *  Storage is highly simplified and data is assumed to follow the rules specified
 *  by WbSchema.  The coupling with the WbSchema which the config is deemed to follow
 *  is rather loose, and mainly affects the data that is read from or written to disk,
 *  and default values.  Any other data conforming to schema rules can be successfully
 *  stored and retrieved, but is not persisted, and this is not recommended.
 *
 */
class WbConfig
{
public:
    typedef WbKeyValueTypeContainer< WbConfig > SubConfigs;

    //Creation/destruction
    WbConfig();
    WbConfig( const WbSchema& schema, const QFileInfo& fileInfo );
    ~WbConfig();

    //???
    const KeyName GetSchemaName() const;

    // persistence
    bool WriteUsing( WbConfigFileWriter& writer ) const;
    bool ReadUsing( WbConfigFileReader& reader );

    // Relative file info
    const QFileInfo GetAbsoluteFileInfo() const;
    const QFileInfo GetPossiblyRelativeFileInfo() const;
    const QString   GetAbsoluteFileNameFor( const QString& possiblyRelativeFileName ) const;


    //SubConfigs
    WbConfig CreateSubConfig( const KeyName& schemaName,
                              const QString& fileName,
                              const KeyId&   id = KeyId() );
    WbConfig AddSubConfig( const KeyName& schemaName,
                           const QString& keyIdFormat = "%1" );

    WbConfig GetSubConfig( const KeyName& name,
                           const KeyId&   id = KeyId() ) const;
    const SubConfigs::ValueIdPairList GetSubConfigs( const KeyName& keyName ) const;

    void RemoveSubConfigs( const KeyName& keyName,
                           const std::vector<KeyId>& idsToRemove = std::vector<KeyId>());

    // Key values
    void KeepKeys( const KeyName& keyName,
                   const std::vector<KeyId>& idsToKeep = std::vector<KeyId>());
    void RemoveKeys( const KeyName& keyName,
                     const std::vector<KeyId>& idsToRemove = std::vector<KeyId>());

    void SetKeyValues( const WbKeyValues& keyValues );
    void SetKeyValue( const KeyName&  keyName,
                      const KeyValue& value,
                      const KeyId&    keyId = KeyId() );

    const KeyId AddKeyValue( const KeyName& keyName,
                             const KeyValue& value,
                             const QString& keyIdFormat = "%1" );

    const KeyValue GetKeyValue( const KeyName& keyName,
                                const KeyId& keyId = KeyId() ) const;
    const WbKeyValues::ValueIdPairList GetKeyValues( const KeyName& keyName ) const;

    //Workbench tree interaction
    void AddTo( QTreeWidget& tree ) const;
    QTreeWidgetItem* const GetLinkedTreeItem() const;
    static const WbConfig FromTreeItem( const QTreeWidgetItem& item );

    // Test Functions
    bool IsTheSameAs( const WbConfig& other ) const;
    bool IsNull() const;
    bool SchemaIsDescendantOf( const KeyName& schemaName ) const;
    bool Contains( const KeyName& keyName, const KeyId& id = KeyId() ) const;
    bool IsModified() const;
    bool DependentExists( const KeyValue& id ) const;

    // Hierarchical access
    const WbConfig GetParent() const;
    const WbConfig FindAncestor    ( const KeyName& ancestorSchemaName ) const;
    const WbConfig FindRootAncestor() const;
    const KeyId FindSubConfigId( const WbConfig& subconfig ) const;
    const WbConfig GetFromPath( const WbPath& path ) const;

    void SetListener( ConfigListener* listener );

private:
    void SetParent( WbConfig& parent );
    void Clear();

    void AddToItem( QTreeWidgetItem* const treeItem ) const;
    void AddSubConfigsTo( QTreeWidgetItem& treeItem ) const;
    QTreeWidgetItem* const CreateTreeItem() const;
    const QFileInfo GetAbsoluteFileInfoFor( const QFileInfo& possiblyRelativeFileInfo ) const;

    void SetDefaults();

    const QString GetSubConfigFileName( const KeyId& elementKey,
                                        const KeyName& schemaName ) const;

    const KeyId GenerateNewId( const KeyName& keyName,
                               const QString& keyIdFormat ) const;

    bool DependentExists( WbSchema::SchemaKeyPairList dependants, const KeyValue& id ) const;

    /** @brief Stores the member data for WbConfig.
     *
     *  To allow copies to share data.
     */
    struct WbConfigData
    {
        WbConfigData();

        WbSchema                         m_schema;
        SubConfigs                       m_subConfigs;
        std::auto_ptr< WbConfig >        m_parent; // owned
        WbKeyValues                      m_keyValues;
        QFileInfo                        m_possiblyRelativeFileInfo;
        mutable QTreeWidgetItem*         m_treeWidgetItem;
        mutable bool                     m_modified;
        const GloballyUniqueId           m_id;
        std::auto_ptr< ConfigListener >  m_listener;

    private:
        WbConfigData( const WbConfigData& );
        WbConfigData& operator =( const WbConfigData& );

        static GloballyUniqueId globallyUniqueId;
    };

    WbConfigData& ModifyData();
    const WbConfigData& ModifyMutableData() const;
    const WbConfigData& QueryData() const;

    QSharedPointer<WbConfigData> m_internalData;

    typedef std::map< GloballyUniqueId, WbConfig > GlobalRegisterType ;
    static GlobalRegisterType globalRegister; ///< Register for WbConfigs
                                              ///< so that we can fin any specific
                                              ///< instance based on a GloballyUniqueId
                                              ///< stored in the workbench tree user
                                              ///< data.
};

#endif // WBCONFIG_H
