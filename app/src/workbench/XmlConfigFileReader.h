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

#ifndef XMLCONFIGFILEREADER_H
#define XMLCONFIGFILEREADER_H

#include <QtCore/qiodevice.h>
#include <QtXml/qdom.h>
#include <memory>
#include "XmlTools.h"
#include "WbConfigFileReader.h"

class WbSchema;

class XmlConfigFileReader : public WbConfigFileReader
{
public:
    XmlConfigFileReader();

    virtual XmlConfigFileReader* const Clone() const;

    virtual bool ReadFrom( QIODevice& ioDevice );

    virtual const KeyName GetSchemaName() const;

    virtual void ReadKeyValues( const KeyName& keyName, WbKeyValues& keyValues );
    virtual void ReadKeyValuesFromGroup( const KeyName& keyName, const KeyName& groupName, WbKeyValues& keyValues );
    virtual void ReadSubSchemaLocations( const KeyName& keyName, SchemaLocationsList& locationsList );

private:
    const KeyValue GetValueFromElement( QDomElement& keyElement );
    QDomDocument m_domDocument;
};

#endif
