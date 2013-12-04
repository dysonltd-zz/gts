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

#include "Collection.h"

#include "WbDefaultKeys.h"

#include <QtCore/QStringBuilder>

Collection::Collection(const KeyName&  collectionSchemaName,
                        const KeyName&  elementSchemaName) :
    m_collectionSchemaName(collectionSchemaName),
    m_elementSchemaName   (elementSchemaName),
    m_collectionConfig    (),
    m_status              (Status_CollectionNotSet),
    m_elementKeyIdFormat  (elementSchemaName.ToQString() % "%1")
{
}

const Collection::StatusType Collection::SetConfig(const WbConfig& config)
{
    FindCollection(config);

    return Status();
}

const WbConfig Collection::CollectionConfig() const
{
    if(Status() == Status_Ok)
    {
        return m_collectionConfig;
    }

    return WbConfig();
}

WbConfig Collection::AddNewElement(const KeyValue& name)
{
    WbConfig newElement;

    if (Status() == Status_Ok)
    {
        newElement = m_collectionConfig.AddSubConfig(m_elementSchemaName,
                                                      m_elementKeyIdFormat);

        newElement.SetKeyValue(WbDefaultKeys::displayNameKey, name);
    }

    return newElement;
}

void Collection::DeleteElement(const KeyId& keyId)
{
    std::vector< KeyId > idsToRemove;

    idsToRemove.push_back(keyId);

    m_collectionConfig.RemoveSubConfigs(m_elementSchemaName, idsToRemove);
}

bool Collection::AnyElementHas(const KeyId& keyId) const
{
    return !ElementById(keyId).IsNull();
}

bool Collection::AnyElementHas(const KeyName&  keyName,
                                const KeyValue& keyValue,
                                const Qt::CaseSensitivity& caseSensitivity) const
{
    for (size_t i = 0; i < NumElements(); ++i)
    {
        if (ElementAt(i).value.GetKeyValue(keyName).IsEqualTo(keyValue,
                                                                    caseSensitivity))
        {
            return true;
        }
    }

    return false;
}

const Collection::StatusType Collection::Status() const
{
    return m_status;
}

const size_t Collection::NumElements() const
{
    if(Status() == Status_Ok)
    {
        WbConfig::SubConfigs::ValueIdPairList elementConfigs =
                        m_collectionConfig.GetSubConfigs(m_elementSchemaName);

        return elementConfigs.size();
    }

    return 0;
}

const WbConfig::SubConfigs::ValueIdPair Collection::ElementAt(const size_t index) const
{
    if (Status() == Status_Ok)
    {
        WbConfig::SubConfigs::ValueIdPairList elementConfigs =
                        m_collectionConfig.GetSubConfigs(m_elementSchemaName);

        return elementConfigs.at(index);
    }

    return WbConfig::SubConfigs::ValueIdPair();
}

const WbConfig Collection::ElementById(const KeyId& id) const
{
    return m_collectionConfig.GetSubConfig(m_elementSchemaName, id);
}

void Collection::FindCollection(const WbConfig & config)
{
    const WbConfig workbenchConfig(config.FindRootAncestor());
    m_collectionConfig = workbenchConfig.GetSubConfig(m_collectionSchemaName);

    m_status = (m_collectionConfig.IsNull()) ? Status_CollectionNotFound : Status_Ok;
}
