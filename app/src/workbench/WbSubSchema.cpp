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

#include "WbSubSchema.h"

#include <iostream>

#include "WbSchema.h"
#include "WbConfig.h"
#include "WbConfigFileReader.h"
#include "WbConfigFileWriter.h"

WbSubSchema::WbSubSchema( const WbSchema& subSchema,
                          const Multiplicity::Type& multiplicity,
                          const QString& defaultFileName )
    :
    WbSchemaElement( subSchema.Name(), multiplicity ),
    m_schema( subSchema ),
    m_defaultFileName( defaultFileName )
{}

WbSubSchema* const WbSubSchema::Clone() const
{
    return new WbSubSchema( *this );
}

void WbSubSchema::ReadFrom( WbConfigFileReader& reader, WbConfig& config ) const
{
    SchemaLocationsList locations;
    reader.ReadSubSchemaLocations( GetKeyName(), locations );

    for ( size_t i = 0; i < locations.size(); ++i )
    {
        WbSchemaLocationIdPair& locationIdPair( locations.at( i ) );

        WbConfig subConfig( config.CreateSubConfig( GetKeyName(),
                                                    locationIdPair.location,
                                                    locationIdPair.id ) );

        std::auto_ptr<WbConfigFileReader> readerClone( reader.Clone() );
        subConfig.ReadUsing( *readerClone );
    }
}

void WbSubSchema::SetDefaultTo( WbConfig& config ) const
{
    if ( !m_defaultFileName.isEmpty() )
    {
        config.CreateSubConfig( GetKeyName(), m_defaultFileName ); //defaults always have no id currently
    }
}

bool WbSubSchema::WriteTo( WbConfigFileWriter& writer, const WbConfig& config ) const
{
    WbConfig::SubConfigs::ValueIdPairList values( config.GetSubConfigs( GetKeyName() ) );

    for ( size_t i = 0; i < values.size(); ++i )
    {
        WbConfig::SubConfigs::ValueIdPair thisPair( values.at( i ) );
        writer.WriteSubConfig( GetKeyName(),
                               thisPair.value.GetPossiblyRelativeFileInfo(),
                               thisPair.id );

        std::auto_ptr<WbConfigFileWriter> writerClone( writer.Clone() );
        const bool successful = thisPair.value.WriteUsing( *writerClone );
        if ( !successful )
        {
            return false;
        }
    }
    return true;
}

void WbSubSchema::PrintOn( std::ostream& os, const std::string& indent ) const
{
    os << indent << "Start SubSchema: " << GetKeyName() << std::endl;
    m_schema.PrintOn( os, indent + " " );
    os << indent << "End SubSchema: " << GetKeyName() << std::endl;
}

WbSchema& WbSubSchema::ModifiableSchema()
{
    return m_schema;
}
