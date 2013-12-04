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

#include "XmlConfigFileReader.h"
#include "WbKeyValues.h"

XmlConfigFileReader::XmlConfigFileReader()
    :
    m_domDocument()
{
}

XmlConfigFileReader* const XmlConfigFileReader::Clone() const
{
    return new XmlConfigFileReader;
}

bool XmlConfigFileReader::ReadFrom(QIODevice& ioDevice)
{
    bool successful = ioDevice.open(QIODevice::ReadOnly);

    if (successful)
    {
        successful = m_domDocument.setContent(&ioDevice);
    }

    return successful;
}

const KeyName XmlConfigFileReader::GetSchemaName() const
{
    return KeyName(m_domDocument.documentElement().tagName());
}

void XmlConfigFileReader::ReadKeyValues(const KeyName& keyName, WbKeyValues& keyValues)
{
    QDomNodeList keyNodes = m_domDocument.elementsByTagName(keyName.ToQString());

    for (int i = 0; i < keyNodes.size(); ++i)
    {
        QDomElement keyElement = keyNodes.at(i).toElement();

        if (!keyElement.isNull())
        {
            QString id = keyElement.attribute(XmlTools::xmlIdAttribute, QString());
            KeyValue keyValue = GetValueFromElement(keyElement);
            keyValues.SetKeyValue(keyName, keyValue, id);
        }
    }
}

void XmlConfigFileReader::ReadKeyValuesFromGroup(const KeyName& keyName, const KeyName& groupName, WbKeyValues& keyValues)
{
    QDomNodeList groupNodes = m_domDocument.elementsByTagName(groupName.ToQString());

    for (int i = 0; i < groupNodes.size(); ++i)
    {
        QDomElement groupElement = groupNodes.at(i).toElement();

        if (!groupElement.isNull())
        {
            QString id = groupElement.attribute(XmlTools::xmlIdAttribute, QString());
            QDomNodeList keyNodes = groupElement.elementsByTagName(keyName.ToQString());

            for (int i = 0; i < keyNodes.size(); ++i)
            {
                QDomElement keyElement = keyNodes.at(i).toElement();

                if (!keyElement.isNull())
                {

                    keyValues.SetKeyValue(keyName, GetValueFromElement(keyElement), id);
                }
            }
        }
    }
}

void XmlConfigFileReader::ReadSubSchemaLocations(const KeyName& keyName,
                                                  SchemaLocationsList& locationsList)
{
    locationsList.clear();

    const KeyName configFileNameKey(XmlTools::xmlConfigFileNameTag);
    // treat SubSchema key name as a group
    WbKeyValues configFileLocations;
    ReadKeyValuesFromGroup(configFileNameKey, keyName, configFileLocations);

    WbKeyValues::ValueIdPairList configLocationsList =
        configFileLocations.GetKeyValues(configFileNameKey);

    for (size_t i = 0; i < configLocationsList.size(); ++i)
    {
        WbKeyValues::ValueIdPair thisPair(configLocationsList.at(i));

        if (!thisPair.value.IsNull())
        {
            WbSchemaLocationIdPair locationIdPair;
            locationIdPair.location = thisPair.value.ToQString();
            locationIdPair.id       = thisPair.id;
            locationsList.push_back(locationIdPair);
        }
    }
}

const KeyValue XmlConfigFileReader::GetValueFromElement(QDomElement& keyElement)
{
    QDomElement childElement = keyElement.firstChildElement();

    KeyValue keyValue;

    if (childElement.isNull())
    {
        keyValue << keyElement.text();
    }
    else
    {
        for (; !childElement.isNull(); childElement = childElement.nextSiblingElement())
        {
            keyValue << childElement.text();
        }
    }

    return keyValue;
}
