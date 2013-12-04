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

#include "WbSchemaKeyGroup.h"

#include <iostream>

#include "WbSchema.h"
#include "WbConfig.h"
#include "WbKeyValues.h"
#include "WbConfigFileReader.h"
#include "WbConfigFileWriter.h"

WbSchemaKeyGroup::WbSchemaKeyGroup(const KeyName& name,
                                    const Multiplicity::Type& multiplicity)
    :
    WbSchemaElement(name, multiplicity) {}

void WbSchemaKeyGroup::AddKey(const WbSchemaValueKey& key)
{
    m_keys.push_back(key);
}

WbSchemaKeyGroup* const WbSchemaKeyGroup::Clone() const
{
    return new WbSchemaKeyGroup(*this);
}

void WbSchemaKeyGroup::ReadFrom(WbConfigFileReader& reader, WbConfig& config) const
{
    const KeyName groupName(GetKeyName());
    WbKeyValues keyValues;

    for (size_t i = 0; i < m_keys.size(); ++i)
    {
        reader.ReadKeyValuesFromGroup(m_keys[ i ].GetKeyName(), groupName, keyValues);
    }

    config.SetKeyValues(keyValues);
}

bool WbSchemaKeyGroup::WriteTo(WbConfigFileWriter& writer, const WbConfig& config) const
{
    std::map< KeyId, std::vector< KeyNameValuePair > > keysByGroup;

    for (size_t i = 0; i < m_keys.size(); ++i)
    {
        const KeyName subKeyName(m_keys[ i ].GetKeyName());
        WbKeyValues::ValueIdPairList keyValueIdPairs = config.GetKeyValues(subKeyName);

        for (size_t j = 0; j < keyValueIdPairs.size(); ++j)
        {
            KeyNameValuePair nameValuePair = { subKeyName, keyValueIdPairs[ j ].value };
            keysByGroup[ keyValueIdPairs[ j ].id ].push_back(nameValuePair);
        }
    }

    for (std::map< KeyId, std::vector< KeyNameValuePair > >::const_iterator itr = keysByGroup.begin();
            itr != keysByGroup.end();
            ++itr)
    {
        const KeyId thisGroupId = itr->first;

        writer.StartGroup(GetKeyName(), thisGroupId);
        const std::vector< KeyNameValuePair >& thisGroupKeys = itr->second;

        for (size_t i = 0; i < thisGroupKeys.size(); ++i)
        {
            writer.WriteKey(thisGroupKeys[ i ].name, thisGroupKeys[ i ].value, thisGroupId);
        }

        writer.EndGroup(GetKeyName(), thisGroupId);
    }
    return true;
}

void WbSchemaKeyGroup::SetDefaultTo(WbConfig& config) const
{
    for (size_t i = 0; i < m_keys.size(); ++i)
    {
        m_keys[ i ].SetDefaultTo(config);
    }

}

void WbSchemaKeyGroup::PrintOn(std::ostream& os, const std::string& indent) const
{
    os << indent << "Start Group: " << GetKeyName() << std::endl;

    for (size_t i = 0; i < m_keys.size(); ++i)
    {
        m_keys[ i ].PrintOn(os, indent + " ");
    }

    os << indent << "End Group: " << GetKeyName() << std::endl;
}

