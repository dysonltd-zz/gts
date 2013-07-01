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

#ifndef KEYNAME_H
#define KEYNAME_H

#include <QtCore/qstring.h>
#include <QtCore/QList>

/** @brief Type to represent unique key names
 */
class KeyName
{
public:
    explicit KeyName( const QString& name = QString() )
    :
        m_name( name )
    {
    }

    const QString ToQString() const
    {
        return m_name;
    }

    bool IsNull() const
    {
        return m_name.isEmpty();
    }

private:
    QString m_name;
};

inline std::ostream& operator << ( std::ostream& os, const KeyName& keyName )
{
    return ( os << keyName.ToQString().toStdString() );
}

inline const bool operator < ( const KeyName& lhs, const KeyName& rhs )
{
    return ( lhs.ToQString() < rhs.ToQString() );
}

inline const bool operator == ( const KeyName& lhs, const KeyName& rhs )
{
    return ( lhs.ToQString() == rhs.ToQString() );
}

inline const bool operator != ( const KeyName& lhs, const KeyName& rhs )
{
    return !( lhs == rhs );
}

/** @brief Type to represent lists of unique key names
 *
 *  They can be constructed as <tt>KeyNameList() << uniqueKey1 << uniqueKey2;</tt>
 */
typedef QList< KeyName > KeyNameList;

#endif // KEYNAME_H
