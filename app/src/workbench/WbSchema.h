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

#ifndef WBSCHEMA_H
#define WBSCHEMA_H

#include "WbKeyValues.h"
#include "WbSchemaElement.h"
#include "KeyValue.h"
#include <map>
#include <vector>
#include <memory>

class WbPath;
class WbConfig;
class WbConfigFileReader;
class WbConfigFileWriter;
class WbSubSchema;

class DefaultValueMap
{
public:
    DefaultValueMap();

    DefaultValueMap WithDefault(const KeyName& name, const KeyValue& value);

    const KeyValue DefaultFor(const KeyName& name) const;

private:
    typedef std::map< KeyName, KeyValue > DefaultsMap;
    DefaultsMap m_defaults;
};

/** @brief Contains the definition of the structure expected for a
 *  configuration file used in the application.
 *
 *  All elements of the schema are effectively stored by value, so
 *  changing an element (e.g. a sub-schema) after insertion, will not
 *  affect the sub-schema stored.
 *
 *  @todo There are a number of rules governing the use of WbSchema,
 *  which are currently not enforced.
 *
 *  @par Schema rules
 *  - Schema names must be unique in the application.\n
 *  - Schema key names:
 *      - are unique within the schema except that multiple
 *      keys with the same name which serve the same purpose can
 *      exist, distinguished by an ID, which is unique across
 *      these identical keys.
 *      - May not begin with an underscore ('_') as this is reserved
 *      for the reader/writer implementation.
 *  - The key name used for a sub-schema is its schema name.
 *  - Key values are a list of strings. This list is meant to be
 *      retrieved as one value and therefore can only have one ID.
 *      It is up to the WbConfigFileReader/WbConfigFileWriter
 *      subclasses to decide how to store the different elements.
 *  - Schema groups currently serve no purpose other than grouping
 *      the data logically.  They do @em NOT provide a different
 *      namespace for keys. If multiple groups are to use the
 *      same key name then they need an ID. This is implicitly
 *      determined by the ID of the keys within the group, hence
 *      all the keys in a group need the same ID.  Groups cannot
 *      be more than one level deep.
 */

class WbSchema
{
public:
    struct SchemaKeyPair
    {
        KeyName schema;
        KeyName key;
    };

    typedef QList< SchemaKeyPair > SchemaKeyPairList;

    explicit WbSchema(const KeyName& name);
    WbSchema(const WbSchema& other);

    WbSchema();
    ~WbSchema();

    WbSchema& operator =(const WbSchema& other);

    const KeyName& Name() const;

    void AddSingleValueKey(const KeyName& keyName,
                            const WbSchemaElement::Multiplicity::Type& multiplicity,
                            const KeyValue& defaultValue =  KeyValue());
    void AddKeyGroup      (const KeyName& groupName,
                            const WbSchemaElement::Multiplicity::Type& multiplicity,
                            const KeyNameList& keyNames,
                            const DefaultValueMap& defaults = DefaultValueMap());
    void AddSubSchema     (const WbSchema& subSchema,
                            const WbSchemaElement::Multiplicity::Type& multiplicity,
                            const QString& defaultFileName = QString());

    void AddDependant     (const KeyName& schemaName, const KeyName& keyName);

    bool AddSubSchemaToSchema(const WbSchema& subSchema,
                                     const KeyName& nameOfSchemaToAddTo,
                                     const WbSchemaElement::Multiplicity::Type& multiplicity,
                                     const QString& defaultFileName = QString());

    const WbSchema FindSubSchema(const KeyName& name) const;
    const WbSchema GetMostSpecificSubSchema() const;

    const WbPath FindPathToSchema(const WbSchema& schema,
                                   const WbSchemaElement::Multiplicity::Type& multiplicity =
                                       WbSchemaElement::Multiplicity::One) const;

    const size_t GetNumSubSchemas() const;

    const SchemaKeyPairList GetDependants() const;

    bool ReadFrom(WbConfigFileReader& reader, WbConfig& config) const;
    bool WriteTo(WbConfigFileWriter& writer, const WbConfig& config) const;
    void SetDefaultsTo(WbConfig& config) const;

    void PrintOn(std::ostream& os, const std::string& indent = "") const;

    bool IsNull() const;
    bool ContainsSchemaAnywhere(const KeyName& schemaName) const;

    bool ContainsKey      (const KeyName& keyName) const;
    bool ContainsSubSchema(const KeyName& schemaName) const;

    const WbSchemaElement::Multiplicity::Type GetMultiplicity(const KeyName& keyName) const;

private:
    typedef std::map< KeyName, WbSubSchema* > SubSchemaMap;
    typedef SubSchemaMap::const_iterator      SubSchemaConstIterator;
    typedef SubSchemaMap::iterator            SubSchemaIterator;

    typedef std::unique_ptr<WbSchemaElement>  SchemaElementPtr;
    typedef std::vector<SchemaElementPtr>     SchemaElementVector;

    KeyName             m_keyName;
    SchemaElementVector m_elements;
    SchemaKeyPairList   m_dependants;
    SubSchemaMap        m_subSchemas;
};

/** @brief Print out a readable description of the schema
 *
 *  @param os The output stream to print on.
 *  @param schema The schema to print
 */
inline std::ostream& operator <<(std::ostream& os, const WbSchema& schema)
{
    schema.PrintOn(os);
    return os;
}

#endif
