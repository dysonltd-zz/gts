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

#include "WbPath.h"
#include <cassert>
#include "WbConfig.h"

WbPathElement::WbPathElement( const KeyName name,
                              const ElementType& type,
                              const KeyId knownId )
:
m_name   ( name ),
m_type   ( type ),
m_knownId( knownId )
{
    assert( ( m_type == Type_KnownId ) || m_knownId.isEmpty() );
}


WbPathElement WbPathElement::Unique( const KeyName& name )
{
    return WbPathElement( name, Type_Unique );
}

WbPathElement WbPathElement::UnknownId( const KeyName& name )
{
    return WbPathElement( name, Type_UnknownId );
}

WbPathElement WbPathElement::KnownId  ( const KeyName& name, const KeyId& id )
{
    return WbPathElement( name, Type_KnownId, id );
}

WbPathElement WbPathElement::FromMultiplicity( const KeyName& name,
                                               const WbSchemaElement::Multiplicity::Type& multiplicity )
{
    switch ( multiplicity )
    {
        case WbSchemaElement::Multiplicity::One:
            return WbPathElement::Unique( name );
        case WbSchemaElement::Multiplicity::Many:
            return WbPathElement::UnknownId( name );
        default:
            assert( !"Unhandled multiplicity" );
            return WbPathElement::Unique( KeyName( "<ERROR Unhandled Multiplicity>" ) );
    }
}

bool WbPathElement::CompatibleWith( const WbPathElement& other ) const
{
    if ( *this == other )
    {
        return true;
    }
    if ( Name() == other.Name() )
    {
        if ( ( Type() == Type_KnownId ) && other.Type() == Type_UnknownId )
        {
            return true;
        }
    }
    return false;
}

/** @brief Debug only
 *
 * @param os
 * @return
 */
std::ostream& operator << ( std::ostream& os,
                            const WbPathElement& element )
{
    os << element.Name();
    switch ( element.Type() )
    {
        case WbPathElement::Type_Unique:
            break;
        case WbPathElement::Type_UnknownId:
            os << ": ????";
            break;
        case WbPathElement::Type_KnownId:
            os << ": " << element.KnownId().toStdString();
            break;
        default:
            assert( !"Unhandled element type" );
            break;
    }
    return os;
}

const bool operator == ( const WbPathElement& lhs,
                         const WbPathElement& rhs )
{
    if ( lhs.Name() != rhs.Name() )
    {
        return false;
    }
    if ( lhs.Type() != rhs.Type() )
    {
        return false;
    }
    if ( lhs.Type() == WbPathElement::Type_KnownId )
    {
        if ( lhs.KnownId() != rhs.KnownId() )
        {
            return false;
        }
    }
    return true;
}
const bool operator != ( const WbPathElement& lhs, const WbPathElement& rhs )
{
    return !( lhs == rhs );
}



//=================================================================================================================

/** @brief Get the WbPath from the root config to the config specified
 *
 * @param config The config to get the path for
 * @return The path from the root config
 */
WbPath WbPath::FromWbConfig( const WbConfig& config )
{
    WbPath path;
    WbPathElement element( WbPathElement::Unique( config.GetSchemaName() ) );

    if ( !config.GetParent().IsNull() )
    {
        path = FromWbConfig( config.GetParent() );
        KeyId id = config.GetParent().FindSubConfigId( config );
        if ( !id.isEmpty() )
        {
            element = WbPathElement::KnownId( config.GetSchemaName(), id );
        }
    }
    path << element;
    return path;
}

WbPath::WbPath()
:
    m_elements()
{
}

WbPath::~WbPath()
{
}

WbPath& WbPath::operator << ( const WbPathElement& element )
{
    m_elements.push_back( element );
    return *this;
}

WbPath& WbPath::operator << ( const WbPath& otherPath )
{
    assert( &otherPath != this );
    if ( &otherPath != this )
    {
        for ( size_t i = 0; i < otherPath.m_elements.size(); ++i )
        {
            (*this) << otherPath.m_elements.at( i );
        }
    }
    return ( *this );
}


std::ostream& operator << ( std::ostream& os, const WbPath& path )
{
    for ( size_t i = 0; i < path.m_elements.size(); ++i )
    {
        os << path.m_elements.at( i );
        if ( i < (path.m_elements.size()-1) ) os << ", ";
    }
    return os;
}

const bool operator == ( const WbPath& lhs, const WbPath& rhs )
{
    if ( lhs.m_elements.size() != rhs.m_elements.size() )
    {
        return false;
    }

    for ( size_t i = 0; i < lhs.m_elements.size(); ++i )
    {
        if ( lhs.m_elements.at( i ) != rhs.m_elements.at( i ) )
        {
            return false;
        }
    }
    return true;
}

const bool operator != ( const WbPath& lhs, const WbPath& rhs )
{
    return !( lhs == rhs );
}

const WbPath WbPath::BestFitWith( const WbPath& desiredPath ) const
{
    WbPath bestFitPath;

    typedef ElementContainer::const_iterator ElementIterator;
    ElementIterator thisItr = m_elements.begin();
    const ElementIterator thisEnd = m_elements.end();
    ElementIterator desiredItr = desiredPath.m_elements.begin();
    const ElementIterator desiredEnd = desiredPath.m_elements.end();
    while ( ( thisItr != thisEnd ) &&
            ( desiredItr != desiredEnd ) &&
              thisItr->CompatibleWith( *desiredItr ) )
    {
        bestFitPath << *thisItr;
        ++thisItr;
        ++desiredItr;
    }

    if ( desiredItr == desiredEnd )
    {
        while ( ( thisItr != thisEnd ) && ( thisItr->Type() == WbPathElement::Type_Unique ) )
        {
            bestFitPath << *thisItr;
            ++thisItr;
        }
    }
    else
    {
        while ( desiredItr != desiredEnd )
        {
            bestFitPath << *desiredItr;
            ++desiredItr;
        }
    }

    return bestFitPath;
}


