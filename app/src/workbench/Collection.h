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

#ifndef COLLECTION_H_
#define COLLECTION_H_

#include "KeyName.h"
#include "WbConfig.h"

class WbConfig;

class Collection
{
public:
    enum StatusType
    {
        Status_Ok,
        Status_CollectionNotSet,
        Status_CollectionNotFound
    };

    Collection(const KeyName&   collectionSchemaName,
                const KeyName&   elementSchemaName);

    const StatusType SetConfig(const WbConfig& config);
    const size_t NumElements() const;

    const WbConfig::SubConfigs::ValueIdPair ElementAt(const size_t index) const;
    const WbConfig ElementById(const KeyId& id) const;
    const WbConfig CollectionConfig() const;

    WbConfig AddNewElement(const KeyValue& name);
    void DeleteElement(const KeyId& keyId);

    bool AnyElementHas(const KeyId& keyId) const;
    bool AnyElementHas(const KeyName& keyName,
                              const KeyValue& keyValue,
                              const Qt::CaseSensitivity& caseSensitivity =
                                                              Qt::CaseSensitive) const;

private:
    void FindCollection(const WbConfig& config);
    const StatusType Status() const;

    //const QString GetSubConfigFileName(const KeyValue& name) const;

    KeyName    m_collectionSchemaName;
    KeyName    m_elementSchemaName;
    WbConfig   m_collectionConfig;
    StatusType m_status;
    QString    m_elementKeyIdFormat;
};

#endif // COLLECTION_H_
