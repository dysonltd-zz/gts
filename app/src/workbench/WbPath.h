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

#ifndef WBCONFIGPATH_H_
#define WBCONFIGPATH_H_

#include "WbKeyValues.h"
#include <iostream>
#include <deque>
#include "WbSchemaElement.h"

class WbConfig;

class WbPathElement
{
public:
    enum ElementType
    {
        Type_Unique,
        Type_UnknownId,
        Type_KnownId
    };

    static WbPathElement Unique   ( const KeyName& name );
    static WbPathElement UnknownId( const KeyName& name );
    static WbPathElement KnownId  ( const KeyName& name, const KeyId& id );
    static WbPathElement FromMultiplicity( const KeyName& name,
                                           const WbSchemaElement::Multiplicity::Type& multiplicity );
    const KeyName     Name() const    { return m_name; }
    const ElementType Type() const    { return m_type; }
    const KeyId       KnownId() const { return m_knownId; }

    bool CompatibleWith( const WbPathElement& other ) const;

private:
    WbPathElement( const KeyName name,
                   const ElementType& type,
                   const KeyId knownId = KeyId() );

    KeyName     m_name;
    ElementType m_type;
    KeyId       m_knownId;
};

const bool operator == ( const WbPathElement& lhs,
                         const WbPathElement& rhs );
const bool operator != ( const WbPathElement& lhs,
                         const WbPathElement& rhs );


std::ostream& operator << ( std::ostream& os,
                            const WbPathElement& element );

class WbPath
{
public:
    static WbPath FromWbConfig( const WbConfig& config );

    WbPath();

    virtual ~WbPath();

    const WbPath BestFitWith( const WbPath& desiredPath ) const;

    WbPath& operator << ( const WbPathElement& element );
    WbPath& operator << ( const WbPath& otherPath );

    bool IsEmpty() const { return m_elements.empty(); }
    const WbPathElement PopFront()
    {
        WbPathElement front( m_elements.front() );
        m_elements.pop_front();
        return front;
    }

private:
    friend std::ostream& operator << ( std::ostream& os, const WbPath& path );
    friend const bool operator == ( const WbPath& lhs,
                                    const WbPath& rhs );

    typedef std::deque< WbPathElement > ElementContainer;
    ElementContainer m_elements;
};


std::ostream& operator << ( std::ostream& os, const WbPath& path );
const bool operator == ( const WbPath& lhs, const WbPath& rhs );
const bool operator != ( const WbPath& lhs, const WbPath& rhs );

#endif // WBCONFIGPATH_H_
