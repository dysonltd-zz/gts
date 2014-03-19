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

#include "KeyValue.h"

#include <opencv/cv.h>

const QString KeyValue::trueString ( "true" );
const QString KeyValue::falseString( "false" );

KeyValue::KeyValue() :
  m_elementList()
{
}

KeyValue::KeyValue( const ElementType& firstElement ) :
  m_elementList( firstElement )
{
}

KeyValue& KeyValue::operator <<( const ElementType& newElement )
{
    m_elementList << newElement;
    return *this;
}

const QString KeyValue::ToQString() const
{
    QString qstring;

    for ( int i = 0; i < m_elementList.size(); ++i )
    {
        qstring.append( m_elementList.at( i ) );
    }
    return qstring;
}

const KeyId KeyValue::ToKeyId() const
{
    return KeyId( ToQString() );
}

const QStringList KeyValue::ToQStringList() const
{
    return m_elementList;
}

double KeyValue::ToDouble() const
{
    return ToQString().toDouble();
}

int KeyValue::ToInt() const
{
    return ToQString().toInt();
}

bool KeyValue::ToBool() const
{
    return ( ToQString() == trueString );
}

void KeyValue::AddTo( QDomElement& element, QDomDocument& domDocument ) const
{
    if ( IsMultiValued() )
    {
        for ( int i = 0; i < m_elementList.size(); ++i )
        {
            QDomElement newChildElement = domDocument.createElement( QString( "_%1" ).arg( i ) );
            newChildElement.appendChild( domDocument.createTextNode( m_elementList.at( i ) ) );
            element.appendChild( newChildElement );
        }
    }
    else
    {
        element.appendChild( domDocument.createTextNode( ToQString() ) );
    }
}

bool KeyValue::IsNull() const
{
    return m_elementList.isEmpty();
}

bool KeyValue::IsEqualTo( const KeyValue& other,
                          const Qt::CaseSensitivity& caseSensitive ) const
{
    if ( m_elementList.size() != other.m_elementList.size() )
    {
        return false;
    }

    for ( int i = 0; i < other.m_elementList.size(); ++i )
    {
        if ( m_elementList.at( i )
                    .compare( other.m_elementList.at( i ), caseSensitive ) != 0 )
        {
            return false;
        }
    }

    return true;
}

bool KeyValue::IsMultiValued() const
{
    return m_elementList.size() > 1;
}

const std::wstring KeyValue::ToWString() const
{
    return ToQString().toStdWString();
}

bool KeyValue::TocvMat( cv::Mat& matrix ) const
{
    CvMat mat = static_cast<CvMat>( matrix );

    return ToCvMat( mat );
}

bool KeyValue::ToCvMat( CvMat& matrix ) const
{
    if ( matrix.rows*matrix.cols != m_elementList.size() )
    {
        return false;
    }

    KeyValue newValue;

    for ( int r = 0; r < matrix.rows; ++r )
    {
        for ( int c = 0; c < matrix.cols; ++c )
        {
            cvmSet( &matrix, r, c, m_elementList.at( r*matrix.cols+c ).toDouble() );
        }
    }

    return true;
}

/**
 * End value of vector is undefined if can't convert
 * @param vector
 * @return
 */
bool KeyValue::ToStdVectorOfCvPoint2f( std::vector< cv::Point2f >& vector ) const
{
    vector.clear();
    const int numDimensions = 2;

    if ( m_elementList.size()%numDimensions != 0 )
    {
        return false;
    }

    const int vectorLength = m_elementList.size()/numDimensions;

    for ( int i = 0; i < vectorLength; ++i )
    {
        const int thisOffset = i*numDimensions;
        bool xOk, yOk;
        const float x = m_elementList.at( thisOffset+0 ).toFloat( &xOk );
        const float y = m_elementList.at( thisOffset+1 ).toFloat( &yOk );
        if ( !xOk || !yOk ) return false;

        vector.push_back( cv::Point2f( x, y ) );
    }

    return true;
}

#if defined(__MINGW32__) || defined(__GNUC__)

const KeyValue KeyValue::from( const char* firstElement )
{
    return KeyValue( QString( "%1" ).arg( firstElement ) );
}

const KeyValue KeyValue::from( const QString& firstElement )
{
    return KeyValue( QString( "%1" ).arg( firstElement ) );
}

const KeyValue KeyValue::from( const int& firstElement )
{
    return KeyValue( QString( "%1" ).arg( firstElement ) );
}

const KeyValue::ElementType
KeyValue::elemFromFloatingPoint( const double& floatingPointValue )
{
    return QString( "%1" ).arg( floatingPointValue,
                                0,
                                'g',
                                maxFloatingPointPrecision );
}

const KeyValue::ElementType
KeyValue::elemFromFloatingPoint( const long double& floatingPointValue )
{
    return QString( "%1" ).arg( floatingPointValue,
                                0,
                                'g',
                                maxFloatingPointPrecision );
}

const KeyValue::ElementType
KeyValue::elemFromFloatingPoint( const float& floatingPointValue )
{
    return QString( "%1" ).arg( floatingPointValue,
                                0,
                                'g',
                                maxFloatingPointPrecision );
}

const KeyValue KeyValue::from( const double& firstElement )
{
    return KeyValue( elemFromFloatingPoint( firstElement ) );
}

const KeyValue KeyValue::from( const long double& firstElement )
{
    return KeyValue( elemFromFloatingPoint( firstElement ) );
}

const KeyValue KeyValue::from( const float& firstElement )
{
    return KeyValue( elemFromFloatingPoint( firstElement ) );
}

const KeyValue KeyValue::from( const std::wstring& firstElement )
{
    return KeyValue( QString::fromStdWString( firstElement ) );
}

const KeyValue KeyValue::from( const bool& firstElement )
{
    return KeyValue( firstElement ? trueString : falseString );
}

const KeyValue KeyValue::from( const CvMat& matrix )
{
    KeyValue newValue;

    for ( int r = 0; r < matrix.rows; ++r )
    {
        for ( int c = 0; c < matrix.cols; ++c )
        {
            newValue << elemFromFloatingPoint( cvmGet( &matrix, r, c ) );
        }
    }

    return newValue;
}

const KeyValue KeyValue::from( const cv::Mat& matrix )
{
    return from( static_cast< CvMat >( matrix ) );
}

const KeyValue KeyValue::from( const std::vector< cv::Point2f >& vec )
{
    KeyValue newKeyValue;

    for ( size_t i = 0; i < vec.size(); ++i )
    {
        const cv::Point2f thisPoint( vec.at( i ) );
        newKeyValue << elemFromFloatingPoint( thisPoint.x );
        newKeyValue << elemFromFloatingPoint( thisPoint.y );
    }

    return newKeyValue;
}

#endif

const bool operator == ( const KeyValue& lhs, const KeyValue& rhs )
{
    return lhs.m_elementList == rhs.m_elementList;
}

const bool operator != ( const KeyValue& lhs, const KeyValue& rhs )
{
    return !( lhs == rhs );
}
