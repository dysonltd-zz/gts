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

#ifndef WBKEYVALUES_H
#define WBKEYVALUES_H

#include <vector>

#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>

#include <map>
#include <vector>
#include <memory>
#include <cassert>
#include "KeyValue.h"
#include "KeyId.h"
#include "KeyName.h"
#include "Container.h"

/** @brief Class to store and retrieve key values by name and id
 *
 *  @param KeyValueType the type to use for the values.
 */
template< class KeyValueType >
class WbKeyValueTypeContainer
{
    typedef std::map< KeyId,   KeyValueType > KeyIdMap;
    typedef std::map< KeyName, KeyIdMap > KeyNameMap;

public:
    /** @brief A pair containing the key's id and value, to be returned as part of a ValueIdPairList by
     *  the GetKeyValues() function.
     */
    struct ValueIdPair
    {
        KeyId        id;
        KeyValueType value;

        /** @brief Equality operator
         */
        const bool operator ==( const ValueIdPair& other ) const
        {
            return ( id == other.id ) && ( value == other.value );
        }
    };

    /** @brief A list of ValueIdPair structs to be returned by
     *  the GetKeyValues() function.
     */
    typedef std::vector< ValueIdPair > ValueIdPairList;

    /** @brief Remove all stored values.
     */
    void Clear()
    {
        m_keys.clear();
        m_orderedKeys.clear();
    }

    /** @brief Get all the values stored in keys with the supplied name
     *
     *  @param keyName The key name for the keys to retrieve.
     *  @return A list of values and the IDs associated with them.
     */
    const ValueIdPairList GetKeyValues( const KeyName& keyName ) const
    {
        ValueIdPairList keyValues;

        typename KeyNameMap::const_iterator keyIdMap = m_keys.find( keyName );

        if ( keyIdMap != m_keys.end() )
        {
            for ( typename KeyIdMap::const_iterator itr = keyIdMap->second.begin(); itr != keyIdMap->second.end(); ++itr )
            {
                ValueIdPair pair;
                pair.id    = itr->first;
                pair.value = itr->second;

                keyValues.push_back( pair );
            }
        }

        return keyValues;
    }

    /** @brief Set the key to the given value.
     *
     *  @param keyName The key on which to specify the value.
     *  @param value The value to store.
     *  @param keyId The ID to use to distinguish multiple keys with the same name.  The default (KeyId()) is
     *  intended to be used for keys that should only occur once in the schema.
     */
    void SetKeyValue( const KeyName& keyName, const KeyValueType& value, const KeyId& keyId = KeyId() )
    {
        m_keys[ keyName ][ keyId ] = value;

        if ( !container( m_orderedKeys ).contains( keyName ) )
        {
            // if we haven't previously added this key, remember its
            // insertion order
            m_orderedKeys.push_back( keyName );
        }
    }

    /** @brief Removes the value specified.
     *
     *  @param keyName The name of the key to remove.
     *  @param keyId The ID of the key to remove.
     */
    void RemoveKeyValue( const KeyName& keyName, const KeyId& keyId = KeyId() )
    {
        typename KeyNameMap::iterator keyIdMap = m_keys.find( keyName );
        if ( keyIdMap != m_keys.end() )
        {
            keyIdMap->second.erase( keyId );
        }
    }

    /** @brief Retrieve the value of the key specified.
     *
     *  @param keyName The key to look up.
     *  @param keyId The ID to use to distinguish multiple keys with the same name.  The default (KeyId()) is
     *  intended to be used for keys that should only occur once in the schema.
     */
    const KeyValueType GetKeyValue( const KeyName& keyName, const KeyId& keyId = QString() ) const
    {
        KeyValueType value;
        typename KeyNameMap::const_iterator keyIdMap = m_keys.find( keyName );

        if ( keyIdMap != m_keys.end() )
        {
            typename KeyIdMap::const_iterator keyValue = keyIdMap->second.find( keyId );

            if ( keyValue != keyIdMap->second.end() )
            {
                value = ( keyValue->second );
            }
        }

        return value;
    }

    /** @brief Apply a function to each of the keys-value-id triples in turn.
     *
     *  And do this in order of the key names.
     *
     *  @param func A function or functor to apply to the triples.
     */
    template < class FunctionType >
    void ForEachOrderedByKeyName( const FunctionType& func ) const
    {
        for ( typename KeyNameMap::const_iterator keyIdMap = m_keys.begin();
                keyIdMap != m_keys.end(); ++keyIdMap )
        {
            for ( typename KeyIdMap::const_iterator keyValueItr = keyIdMap->second.begin();
                    keyValueItr != keyIdMap->second.end(); ++keyValueItr )
            {
                func( keyValueItr->second );
            }
        }
    }

    /** @brief Apply a function to each of the keys-value-id triples in turn.
     *
     *  And do this in the order they were initially added.  If a value is initially
     *  set, then set again, it is the order of the initial insertion which is important.
     *  Only the insertion order of key @em names is considered not individual IDs.
     *
     *  @param func A function or functor to apply to the triples.
     */
    template < class FunctionType >
    void ForEachOrderedByInsertion( const FunctionType& func ) const
    {
        for ( size_t i = 0; i < m_orderedKeys.size(); ++i )
        {
#if defined(__MINGW32__) || defined(__GNUC__)
            typename KeyNameMap::const_iterator keyIdMapItr = m_keys.find( m_orderedKeys[ i ] );
#else
            const KeyNameMap::const_iterator keyIdMapItr = m_keys.find( m_orderedKeys[ i ] );
#endif
            assert( keyIdMapItr != m_keys.end() );
            const KeyIdMap& keyIdMap = keyIdMapItr->second;
            for ( typename KeyIdMap::const_iterator keyValueItr = keyIdMap.begin();
                    keyValueItr != keyIdMap.end(); ++keyValueItr )
            {
                func( keyValueItr->second );
            }
        }
    }

    /** @brief Copy all the values from @a other into this container.
     *
     *  Do not clear this first and overwrite existing keys with the same name and id.
     *
     *  @param other The container with the keys to copy.
     */
    void SetKeys( const WbKeyValueTypeContainer& other )
    {
        for ( typename KeyNameMap::const_iterator keyIdMap = other.m_keys.begin();
                keyIdMap != other.m_keys.end(); ++keyIdMap )
        {
            for ( typename KeyIdMap::const_iterator keyValueItr = keyIdMap->second.begin();
                    keyValueItr != keyIdMap->second.end(); ++keyValueItr )
            {
                SetKeyValue( keyIdMap->first, keyValueItr->second, keyValueItr->first );
            }
        }
    }


    /** @brief Get a vector of all the values.
     *
     *  Order of the list is unspecified.
     *
     *  @return List of all key values in unspecified order
     */
    const std::vector< KeyValueType > EnumerateValues() const
    {
        std::vector< KeyValueType > values;
        for ( typename KeyNameMap::const_iterator keyIdMap = m_keys.begin();
                keyIdMap != m_keys.end(); ++keyIdMap )
        {
            for ( typename KeyIdMap::const_iterator keyValueItr = keyIdMap->second.begin();
                    keyValueItr != keyIdMap->second.end(); ++keyValueItr )
            {
                values.push_back( keyValueItr->second );
            }
        }
        return values;
    }

private:
    KeyNameMap m_keys;
    std::vector< KeyName > m_orderedKeys;
};

/** @brief The default type of WbKeyValueTypeContainer (using KeyValue as the KeyValueType) */
typedef WbKeyValueTypeContainer< KeyValue > WbKeyValues;

#endif // WBKEYVALUES_H
