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

#ifndef XMLCONFIGWRITER_H
#define XMLCONFIGWRITER_H

#include "WbSchema.h"

#include "WbConfigFileWriter.h"

#include <QtCore/qfileinfo.h>
#include <QtXml/qdom.h>
#include <cassert>
#include "XmlTools.h"

class XmlConfigFileWriter : public WbConfigFileWriter
{
public:
    XmlConfigFileWriter()
        :
        m_domDocument(),
        m_currentElement()
    {
    }

    virtual XmlConfigFileWriter* const Clone() const
    {
        return new XmlConfigFileWriter;
    }

    virtual void StartConfigFile( const KeyName& name )
    {
        QDomElement rootElement = m_domDocument.createElement( name.ToQString() );
        m_currentElement = rootElement;
        m_domDocument.appendChild( rootElement );
    }

    virtual void EndConfigFile( const KeyName& name )
    {
        Q_UNUSED(name);
    }

    virtual bool WriteTo( QIODevice& ioDevice )
    {
        bool successful = ioDevice.open( QIODevice::WriteOnly );

        if ( successful )
        {
            qint64 bytesWritten = ioDevice.write( m_domDocument.toByteArray() );
            successful = ( bytesWritten != -1 );
        }

        ioDevice.close();
        return successful;
    }

    virtual void WriteKey( const KeyName& name, const KeyValue& value, const KeyId& id )
    {
        if ( DocumentStarted() )
        {
            QDomElement newKeyElement = m_domDocument.createElement( name.ToQString() );
            QString idToWrite( id );

            if ( m_currentElement != m_domDocument.documentElement() )
            {
                idToWrite.clear();
            }

            if ( !idToWrite.isEmpty() )
            {
                newKeyElement.setAttribute( XmlTools::xmlIdAttribute, idToWrite );
            }

            value.AddTo( newKeyElement, m_domDocument );
            m_currentElement.appendChild( newKeyElement );
        }
    }

    virtual void StartGroup( const KeyName& name, const KeyId& id )
    {
        if ( DocumentStarted() )
        {
            QDomElement newKeyGroup = m_domDocument.createElement( name.ToQString() );

            if ( !id.isEmpty() )
            {
                newKeyGroup.setAttribute( XmlTools::xmlIdAttribute, id );
            }

            m_currentElement.appendChild( newKeyGroup );
            m_currentElement = newKeyGroup;
        }
    }

    virtual void EndGroup( const KeyName& name, const KeyId& id )
    {
        Q_UNUSED(name);
        Q_UNUSED(id);

        m_currentElement = m_currentElement.parentNode().toElement();
    }

    virtual void WriteSubConfig( const KeyName& name,
                                 const QFileInfo& configFileLocation,
                                 const KeyId& id )
    {
        if ( DocumentStarted() )
        {
            StartGroup( name, id );
            WriteKey( KeyName( XmlTools::xmlConfigFileNameTag ),
                      KeyValue::from( configFileLocation.filePath() ),
                      KeyId() );
            EndGroup( name, id );
        }
    }

private:
    bool DocumentStarted() const
    {
        if ( m_currentElement.isNull() )
        {
            return false;
        }

        return true;
    }

    QDomDocument m_domDocument;
    QDomElement m_currentElement;
};

#endif
