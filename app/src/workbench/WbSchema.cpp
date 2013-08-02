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

#include "WbSchema.h"
#include "WbConfig.h"

#include <iostream>
#include <memory>
#include "WbSchemaValueKey.h"
#include "WbSchemaKeyGroup.h"
#include "WbSubSchema.h"
#include "WbConfigFileReader.h"
#include "WbConfigFileWriter.h"
#include "WbPath.h"

DefaultValueMap::DefaultValueMap()
:
m_defaults()
{
}

DefaultValueMap DefaultValueMap::WithDefault(const KeyName & name, const KeyValue & value)
{
    DefaultValueMap newMap( *this );
    newMap.m_defaults[ name ] = value;
    return newMap;
}



const KeyValue DefaultValueMap::DefaultFor(const KeyName & name) const
{
    DefaultsMap::const_iterator foundItr = m_defaults.find( name );
    if ( foundItr != m_defaults.end() )
    {
        return foundItr->second;
    }
    return  KeyValue();
}

//=================================================================================================================

/** @brief Construct a new schema identified by @a name
 *
 *  @param name Application-wide unique schema name.
 */
WbSchema::WbSchema( const KeyName& name )
    :
    m_keyName( name )
{
    assert( !m_keyName.IsNull() );
}

/** @brief Construct a new schema which is a copy of the current value of @a other
 *
 *  @copydetails WbSchema::operator=()
 */
WbSchema::WbSchema( const WbSchema& other )
:
m_keyName( other.m_keyName )
{
    *this = other;
}

/** @brief Construct a new null schema.
 *
 *  Null schemas are used to enable returning a schema by value when there is no valid result.
 *  The should never contain any keys/groups/sub-schemas, or a name.
 */
WbSchema::WbSchema()
{
    // null schema
}

WbSchema::~WbSchema()
{

}

/** @brief set this schema to a copy of the current value of @a other.
 *
 *  The individual elements are cloned or copied, not referenced, so they are no longer linked to the originals.
 *  @param other Schema to copy.
 */
WbSchema& WbSchema::operator =( const WbSchema& other )
{
    if ( this != &other )
    {
        m_keyName = other.m_keyName;

        for ( size_t i = 0; i < other.m_elements.size(); ++i )
        {
            const SchemaElementPtr& otherElement = other.m_elements.at( i );
            WbSchemaElement*  thisElement(otherElement->Clone());
            if ( other.ContainsSubSchema(otherElement->GetKeyName()) )
            {
                m_subSchemas[otherElement->GetKeyName()] = dynamic_cast<WbSubSchema*>(thisElement);
            }
            m_elements.push_back(SchemaElementPtr(thisElement));
        }

        for ( int i = 0; i < other.m_dependants.size(); ++i )
        {
            m_dependants.push_back(other.m_dependants.at(i));
        }
    }

    return *this;
}

/** @brief Get the application-wide unique name of this schema.
 *
 *  @return The unique name of the schema.
 */
const KeyName& WbSchema::Name() const
{
    return m_keyName;
}

/** @brief Add a single key which has a single KeyValue
 *
 *  @param keyName The name to give the key.
 *  @param multiplicity How many times the key can appear in the schema.
 *  @param defaultValue The default value to use if the value is not specified by the config file.
 *  If this is set to  KeyValue() (the default) then there is no default value.
 */
void WbSchema::AddSingleValueKey( const KeyName& keyName,
                                  const WbSchemaElement::Multiplicity::Type& multiplicity,
                                  const KeyValue& defaultValue )
{
    assert( !keyName.IsNull() );
    m_elements.push_back(SchemaElementPtr(new WbSchemaValueKey(keyName, multiplicity, defaultValue)));
}

/** @brief Add a key group
 *
 *  @param groupName The name to give the group.
 *  @param multiplicity How many times the key can appear in the schema.
 *  @param keyNames The names of the keys within the group.
 */
void WbSchema::AddKeyGroup( const KeyName& groupName,
                            const WbSchemaElement::Multiplicity::Type& multiplicity,
                            const KeyNameList& keyNames,
                            const DefaultValueMap& defaults )
{
    assert( !groupName.IsNull() );
    WbSchemaKeyGroup* newGroup = new WbSchemaKeyGroup( groupName, multiplicity );

    for ( int i = 0; i < keyNames.size(); ++i )
    {
        newGroup->AddKey( WbSchemaValueKey( keyNames.at( i ),
                                            multiplicity,
                                            defaults.DefaultFor( keyNames.at( i ) ) ) );
    }

    m_elements.push_back(SchemaElementPtr(newGroup));
}

void WbSchema::AddDependant( const KeyName& schemaName, const KeyName& keyName )
{
    SchemaKeyPair p;
    p.schema = schemaName;
    p.key = keyName;

    m_dependants.push_back( p );
}

/** @brief Add a sub-schema.
 *
 *  @param subSchema The sub-schema specification.
 *  @param multiplicity How many times the key can appear in the schema.
 *  @param defaultFileName The default name of the config file corresponding the this sub-schema. If
 *  this is empty there is no default config file.
 */
void WbSchema::AddSubSchema( const WbSchema& subSchema,
                             const WbSchemaElement::Multiplicity::Type& multiplicity,
                             const QString& defaultFileName )
{
    assert( !subSchema.IsNull() );
    WbSubSchema* subSchemaElement = new WbSubSchema( subSchema,
                                                     multiplicity,
                                                     defaultFileName );
    m_elements.push_back(SchemaElementPtr(subSchemaElement));
    m_subSchemas[ subSchema.Name() ] = subSchemaElement;
}

/** @brief Add a sub-schema to this schema or a descendant schema by name.
 *
 *  The rules forbid duplicate schema names, but if they do occur the schema which the
 *  sub-schema is added to is undefined.
 *
 *  @param subSchemaKeyName The name to give the key representing the subschema in the config file.
 *  @param subSchema The sub-schema specification.
 *  @param defaultFileName The default name of the config file corresponding the this sub-schema. If
 *  this is empty there is no default config file.
 *  @param nameOfSchemaToAddTo The schema to add the new sub-schema to.
 *  @return whether the operation was successful (i.e. whether an appropriate schema was found to add
 *  the sub-schema to).
 */
bool WbSchema::AddSubSchemaToSchema( const WbSchema& subSchema,
                                     const KeyName& nameOfSchemaToAddTo,
                                     const WbSchemaElement::Multiplicity::Type& multiplicity,
                                     const QString& defaultFileName )
{
    if ( nameOfSchemaToAddTo == Name() )
    {
        AddSubSchema( subSchema, multiplicity, defaultFileName );
        return true;
    }
    else
    {
        for ( SubSchemaIterator i = m_subSchemas.begin(); i != m_subSchemas.end(); ++i )
        {
            if ( i->second->ModifiableSchema()
                            .AddSubSchemaToSchema( subSchema,
                                                   nameOfSchemaToAddTo,
                                                   multiplicity,
                                                   defaultFileName ) )
            {
                return true;
            }
        }
    }

    return false;
}

/** @brief return the SubSchema with the specified @a name.
 *
 *  @param name name of the key representing the sub-schema.
 */
const WbSchema WbSchema::FindSubSchema( const KeyName& name ) const
{
    assert( !name.IsNull() );
    SubSchemaConstIterator itr = m_subSchemas.find( name );

    if ( itr != m_subSchemas.end() )
    {
        return itr->second->ModifiableSchema();
    }

    return WbSchema();
}

/** @brief return the SubSchema of the lowest level within this schema.
 *
 *  Obtains this by following the @em first sub-schema recursively until
 *  there are none left.  If there are multiple children behaviour is
 *  undefined.
 *
 *  @return lowest-level sub-schema.
 */
const WbSchema WbSchema::GetMostSpecificSubSchema() const
{
    if ( m_subSchemas.empty() )
    {
        return *this;
    }

    return m_subSchemas.begin()->second->ModifiableSchema().GetMostSpecificSubSchema();
}

/** @brief Read from the config file according to this schema.
 *
 *  Use the WbConfigFileReader specified to fill out the contents in the WbConfig, as specified
 *  by this schema.
 *  Keys not specified by the schema are ignored.
 *  If the config file is not of the correct type nothing is read and we return @a false.
 *
 *  @param reader The WbConfigFileReader to use.
 *  @param config The config structure to fill out.
 *  @return Whether we read the config or not.
 */
bool WbSchema::ReadFrom( WbConfigFileReader& reader, WbConfig& config ) const
{
    if ( reader.GetSchemaName() == Name() )
    {
        for ( size_t i = 0; i < m_elements.size(); ++i )
        {
            m_elements[ i ]->ReadFrom( reader, config );
        }

        return true;
    }

    return false;
}

/** @brief Write to the config file according to this schema.
 *
 *  Use the WbConfigFileWriter specified to write the contents of the WbConfig, as specified
 *  by this schema.
 *  Keys not specified by the schema are ignored.
 *
 *  @param writer The WbConfigFileWriter to use.
 *  @param config The config data to write to the writer.
 *  @return Whether we successfully wrote all keys.
 */
bool WbSchema::WriteTo( WbConfigFileWriter& writer, const WbConfig& config ) const
{
    writer.StartConfigFile( m_keyName );

    for ( size_t i = 0; i < m_elements.size(); ++i )
    {
        const bool successful = m_elements[ i ]->WriteTo( writer, config );
        if ( !successful )
        {
            return false;
        }
    }

    writer.EndConfigFile( m_keyName );

    return true;
}

/** @brief Print out a readable description of the schema
 *
 *  @param os the output stream to print on.
 *  @param indent the string used before each line of the schema as an indent
 */
void WbSchema::PrintOn( std::ostream& os, const std::string& indent ) const
{
    os << "Schema '" << m_keyName << ":" << std::endl;
    for ( size_t i = 0; i < m_elements.size(); ++i )
    {
        m_elements[ i ]->PrintOn( os, indent );
    }
}

/** @brief Set all default values specified in the schema into @a config
 *
 *  @param config The config structure that needs its defaults set
 */
void WbSchema::SetDefaultsTo( WbConfig& config ) const
{
    for ( size_t i = 0; i < m_elements.size(); ++i )
    {
        m_elements[ i ]->SetDefaultTo( config );
    }
}

/** @brief Returns whether this is a null schema
 *
 *  @return @a true if this is a null schema otherwise @ false.
 */
bool WbSchema::IsNull() const
{
    return Name() == KeyName();
}

/** @brief Returns a count the the number of sub-schemas.
 *
 *  @return The the number of sub-schemas.
 */
const size_t WbSchema::GetNumSubSchemas() const
{
    return m_subSchemas.size();
}

const WbSchema::SchemaKeyPairList WbSchema::GetDependants() const
{
    return m_dependants;
}

/** @brief Returns whether the specified schema name is the name of any of the
 *  direct sub-schemas of this schema.
 *
 *  @param schemaName The name of the schema to test for.
 *  @return @a true if the sub-schema exists at this level otherwise @a false.
 *  Also @a false if this schema's name is @a schemaName.
 */
bool WbSchema::ContainsSubSchema( const KeyName& schemaName ) const
{
    return ( m_subSchemas.find( schemaName ) != m_subSchemas.end() );
}

/** @brief Returns whether the specified schema name is the name of this schema
 *  or if any of its sub-schemas contains a schema of that name
 *
 *  @param schemaName The name of the schema to test for.
 *  @return @a true if the schema exists otherwise @a false.
 */
bool WbSchema::ContainsSchemaAnywhere( const KeyName& schemaName ) const
{
    if ( Name() == schemaName )
    {
        return true;
    }

    for ( SubSchemaConstIterator i = m_subSchemas.begin(); i != m_subSchemas.end(); ++i )
    {
        if ( i->second->ModifiableSchema().ContainsSchemaAnywhere( schemaName ) )
        {
            return true;
        }
    }
    return false;
}

#if defined(__MINGW32__) || defined(__GNUC__)
namespace
{
    struct NameIs
    {
        NameIs( const KeyName& keyName )
        :
            m_keyName( keyName )
        {
        }

        bool operator() ( std::unique_ptr<WbSchemaElement> const & element ) const
        {
            return ( element->GetKeyName() == m_keyName );
        }
    private:
        const KeyName m_keyName;
    };
}
#endif


bool WbSchema::ContainsKey( const KeyName& keyName ) const
{
#if defined(__MINGW32__) || defined(__GNUC__)
    return ( std::find_if( m_elements.begin(), m_elements.end(),
                           NameIs( keyName ) ) != m_elements.end() );
#else
    return (std::find_if(m_elements.begin(), m_elements.end(),
                         [this, keyName](const SchemaElementPtr& element) -> bool
                         {
                            return (element->GetKeyName() == keyName);
                         })
            != m_elements.end());
#endif
}


const WbSchemaElement::Multiplicity::Type WbSchema::GetMultiplicity( const KeyName& keyName ) const
{
    for ( size_t i = 0; i < m_elements.size(); ++i )
    {
        const SchemaElementPtr& thisElement = m_elements.at( i );
        if ( thisElement->GetKeyName() == keyName )
        {
            return thisElement->GetMultiplicity();
        }
    }
    assert( !"Key not found" );
    return WbSchemaElement::Multiplicity::One;
}

const WbPath WbSchema::FindPathToSchema( const WbSchema& schema,
                                         const WbSchemaElement::Multiplicity::Type& multiplicity ) const
{
    WbPath path;
    if ( ContainsSchemaAnywhere( schema.Name() ) )
    {
        path << WbPathElement::FromMultiplicity( Name(), multiplicity );
        for ( SubSchemaConstIterator i = m_subSchemas.begin(); i != m_subSchemas.end(); ++i )
        {
            path << i->second->ModifiableSchema()
                .FindPathToSchema( schema,
                                   GetMultiplicity(
                                       i->second->ModifiableSchema().Name() ) );
        }
    }
    return path;
}

