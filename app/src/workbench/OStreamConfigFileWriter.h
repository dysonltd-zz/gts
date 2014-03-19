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

#ifndef OSTREAMCONFIGWRITER_H
#define OSTREAMCONFIGWRITER_H

#include "WbSchema.h"

#include "WbConfigFileWriter.h"

#include <iostream>
#include <cassert>
#include "NullConfigFileWriter.h"

namespace
{
    std::ostream& operator << ( std::ostream& os, const QString& qString )
    {
        return ( os << qString.toStdString() );
    }

    std::ostream& operator << ( std::ostream& os, const QStringList& qStringList )
    {
        if ( qStringList.size() == 1 )
        {
            return ( os << qStringList.at( 0 ).toStdString() );
        }

        for ( int i = 0; i < qStringList.size(); ++i )
        {
            os << " [" << i << "]" << qStringList.at( i ).toStdString();
        }

        return os;
    }

    std::ostream& operator << ( std::ostream& os, const KeyValue& keyValue )
    {
        return ( os << keyValue.ToQStringList() );
    }
}

/** @brief A WbConfigFileWriter which outputs to a @a std::ostream
 *
 *  It can be Recursive (printing a config descends into sub-configs), or Non-recursive.
 */
class OStreamConfigFileWriter : public WbConfigFileWriter
{
public:
    enum Recursiveness
    {
        NonRecursive,
        Recursive
    };

    OStreamConfigFileWriter( std::ostream& os = std::cout,
                             const Recursiveness& recursiveness = Recursive )
    :
        m_os( os ),
        m_indent( 0 ),
        m_recursiveness( recursiveness )
    {
    }

    virtual WbConfigFileWriter* const Clone() const
    {
        if ( m_recursiveness == Recursive )
        {
            return new OStreamConfigFileWriter( *this );
        }
        else
        {
            return new NullConfigFileWriter;
        }
    }

    virtual bool WriteTo( QIODevice& ioDevice )
    {
        Q_UNUSED(ioDevice);

        return true;
    }

    virtual void StartConfigFile( const KeyName& name )
    {
        Q_UNUSED(name);

        IncreaseIndent();
    }

    virtual void EndConfigFile( const KeyName& name )
    {
        Q_UNUSED(name);

        DecreaseIndent();
    }

    virtual void WriteKey( const KeyName& name, const KeyValue& value, const KeyId& id = KeyId() )
    {
        m_os << Indent() << "Key: " << name << IdOrEmpty( id ) << ", Value: " << value << std::endl;
    }

    virtual void StartGroup( const KeyName& name, const KeyId& id )
    {
        m_os << Indent()
             << "Start Group: "
             << name
             << IdOrEmpty( id )
             << " -------------"
             << std::endl;

        IncreaseIndent();
    }

    virtual void EndGroup( const KeyName& name, const KeyId& id )
    {
        Q_UNUSED(name);
        Q_UNUSED(id);

        DecreaseIndent();
    }

    virtual void WriteSubConfig( const KeyName& name, const QFileInfo& configFileLocation, const KeyId& id )
    {
        m_os << Indent()
             << "SubConfig: "
             << name
             << IdOrEmpty( id )
             << ", at: "
             << configFileLocation.filePath()
             << std::endl;
    }

private:
    const QString IdOrEmpty( const KeyId& id ) const
    {
        QString idString;

        if ( !id.isEmpty() )
        {
            idString.append( " (id: " );
            idString.append( id );
            idString.append( ")" );
        }
        return idString;
    }

    const std::string Indent() const
    {
        std::string indent;
        for ( size_t i = 0; i < m_indent; ++i )
        {
            indent += "  ";
        }
        return indent;
    }
    void IncreaseIndent()
    {
        m_indent++;
    }
    void DecreaseIndent()
    {
        assert( m_indent > 0 );
        m_indent--;
    }

    std::ostream& m_os;
    size_t m_indent;
    Recursiveness m_recursiveness;
};

#endif // OSTREAMCONFIGWRITER_H
