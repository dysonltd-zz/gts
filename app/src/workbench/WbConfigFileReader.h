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

#ifndef WBCONFIGFILEREADER_H
#define WBCONFIGFILEREADER_H

#include "WbKeyValues.h"

#include <QtCore/QIODevice>
#include <QtCore/QString>

struct WbSchemaLocationIdPair
{
    QString location;
    KeyId   id;
};

inline const bool operator == ( const WbSchemaLocationIdPair& lhs, const WbSchemaLocationIdPair& rhs )
{
    return ( lhs.location == rhs.location ) && ( lhs.id == rhs.id );
}

typedef std::vector< WbSchemaLocationIdPair > SchemaLocationsList;

class WbConfigFileReader
{
public:
    virtual ~WbConfigFileReader() {}

    virtual bool ReadFrom( QIODevice& ioDevice ) = 0;

    virtual WbConfigFileReader* const Clone() const = 0;

    virtual const KeyName GetSchemaName() const = 0;
    virtual void ReadKeyValues( const KeyName& keyName, WbKeyValues& keyValues ) = 0;
    virtual void ReadKeyValuesFromGroup( const KeyName& keyName, const KeyName& groupName, WbKeyValues& keyValues ) = 0;
    virtual void ReadSubSchemaLocations( const KeyName& keyName, SchemaLocationsList& locationsList ) = 0;
};

#endif // WBCONFIGFILEREADER_H
