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

#ifndef WBSCHEMAVALUEKEY_H
#define WBSCHEMAVALUEKEY_H

#include "WbSchemaElement.h"
#include "WbKeyValues.h"

/// class WbSchemaValueKey
class WbSchemaValueKey : public WbSchemaElement
{
public:
    WbSchemaValueKey( const KeyName& name,
                      const Multiplicity::Type& multiplicity,
                      const KeyValue& defaultValue =  KeyValue() );

    virtual WbSchemaValueKey* const Clone() const;

    void ReadFrom( WbConfigFileReader& reader, WbConfig& config ) const;
    bool WriteTo ( WbConfigFileWriter& writer, const WbConfig& config ) const;
    virtual void PrintOn( std::ostream& os, const std::string& indent ) const;
    virtual void SetDefaultTo( WbConfig& config ) const;

private:
    KeyValue m_defaultValue;
};

#endif
