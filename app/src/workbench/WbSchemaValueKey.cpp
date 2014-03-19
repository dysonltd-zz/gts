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

#include "WbSchemaValueKey.h"

#include <iostream>

#include "WbConfig.h"
#include "WbSchema.h"
#include "WbConfigFileReader.h"
#include "WbConfigFileWriter.h"

WbSchemaValueKey::WbSchemaValueKey( const KeyName& name,
                                    const Multiplicity::Type& multiplicity,
                                    const KeyValue& defaultValue )
    :
    WbSchemaElement( name, multiplicity ),
    m_defaultValue( defaultValue )
{}

WbSchemaValueKey* const WbSchemaValueKey::Clone() const
{
    return new WbSchemaValueKey( *this );
}

void WbSchemaValueKey::ReadFrom( WbConfigFileReader& reader, WbConfig& config ) const
{
    WbKeyValues keyValues;
    reader.ReadKeyValues( GetKeyName(), keyValues );
    config.SetKeyValues( keyValues );
}

bool WbSchemaValueKey::WriteTo( WbConfigFileWriter& writer, const WbConfig& config ) const
{
    WbKeyValues::ValueIdPairList values( config.GetKeyValues( GetKeyName() ) );

    for ( size_t i = 0; i < values.size(); ++i )
    {
        writer.WriteKey( GetKeyName(), values[ i ].value, values[ i ].id );
    }
    return true;
}

void WbSchemaValueKey::SetDefaultTo( WbConfig& config ) const
{
    if ( !m_defaultValue.IsNull() )
    {
        config.SetKeyValue( GetKeyName(), m_defaultValue ); //defaults always have no id currently
    }
}

void WbSchemaValueKey::PrintOn( std::ostream& os, const std::string& indent ) const
{
    os << indent << "Key: " << GetKeyName() << std::endl;
}

