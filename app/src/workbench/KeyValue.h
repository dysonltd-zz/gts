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

#ifndef KEYVALUE_H
#define KEYVALUE_H

#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtXml/qdom.h>
#include <string>
#include <opencv/cv.h>
#include "KeyId.h"

/** @brief Type to represent the (possibly multi-part) values of the keys.
 *
 *  They can be constructed as <tt>KeyValue( singlePartValue );</tt> or as
 *  <tt>%KeyValue() << multiPartValuePart1 << multiPartValuePart2;</tt>
 */
class KeyValue
{
public:
    typedef QString ElementType;

    KeyValue();

#if defined(__MINGW32__) || defined(__GNUC__)
    static const KeyValue from( const char* firstElement );
    static const KeyValue from( const QString& firstElement );
    static const KeyValue from( const int& firstElement );

    static const KeyValue from( const double& firstElement );
    static const KeyValue from( const long double& firstElement );
    static const KeyValue from( const float& firstElement );
    static const KeyValue from( const bool& firstElement );
    static const KeyValue from( const std::wstring& firstElement );
    static const KeyValue from( const CvMat& firstElement );
    static const KeyValue from( const cv::Mat& firstElement );
    static const KeyValue from( const std::vector< cv::Point2f >& matrix );
#else
    template < class Type >
    static const KeyValue from( const Type& firstElement );
	template <>
    static const KeyValue from( const double& firstElement );
    template <>
    static const KeyValue from( const long double& firstElement );
    template <>
    static const KeyValue from( const float& firstElement );
    template <>
    static const KeyValue from( const bool& firstElement );
    template <>
    static const KeyValue from( const std::wstring& firstElement );
    template <>
    static const KeyValue from( const CvMat& firstElement );
    template <>
    static const KeyValue from( const cv::Mat& firstElement );
    template <>
    static const KeyValue from( const std::vector< cv::Point2f >& matrix );
#endif

    KeyValue& operator << ( const ElementType& newElement );

    const KeyId        ToKeyId       () const;
    const QString      ToQString     () const;
    const std::wstring ToWString     () const;
    const QStringList  ToQStringList () const;
    double       ToDouble      () const;
    int          ToInt         () const;
    bool         ToBool        () const;
    bool         ToCvMat       ( CvMat& matrix ) const;
    bool         TocvMat       ( cv::Mat& matrix ) const;
    bool         ToStdVectorOfCvPoint2f( std::vector< cv::Point2f >& vector ) const;

    void AddTo( QDomElement& element, QDomDocument& domDocument ) const;

    bool IsNull() const;
    bool IsEqualTo( const KeyValue& other,
                          const Qt::CaseSensitivity& caseSensitive =
                                                              Qt::CaseSensitive ) const;

private:
    explicit KeyValue( const ElementType&  firstElement );

    bool IsMultiValued() const;

#ifdef __GNUC__
    static const ElementType elemFromFloatingPoint(
                            const double& floatingPointValue );
    static const ElementType elemFromFloatingPoint(
                            const long double& floatingPointValue );
    static const ElementType elemFromFloatingPoint(
                            const float& floatingPointValue );
#else
    template <class FloatingPointType>
    static const ElementType elemFromFloatingPoint(
                            const FloatingPointType& floatingPointValue );
#endif

    QStringList m_elementList;

    friend const bool operator == ( const KeyValue& lhs, const KeyValue& rhs );

    static const QString trueString;
    static const QString falseString;
    static const int maxFloatingPointPrecision = 100;
};

const bool operator == ( const KeyValue& lhs, const KeyValue& rhs );
const bool operator != ( const KeyValue& lhs, const KeyValue& rhs );

#if !defined(__MINGW32__) && !defined(__GNUC__)

template <class Type>
const KeyValue KeyValue::from( const Type& firstElement )
{
    return KeyValue( QString( "%1" ).arg( firstElement ) );
}

template <class FloatingPointType>
const KeyValue::ElementType
KeyValue::elemFromFloatingPoint( const FloatingPointType& floatingPointValue )
{
    return QString( "%1" ).arg( floatingPointValue,
                                0,
                                'g',
                                maxFloatingPointPrecision );
}

template <>
const KeyValue KeyValue::from<double>( const double& firstElement )
{
    return KeyValue( elemFromFloatingPoint( firstElement ) );
}

template <>
const KeyValue KeyValue::from<long double>( const long double& firstElement )
{
    return KeyValue( elemFromFloatingPoint( firstElement ) );
}

template <>
const KeyValue KeyValue::from<float>( const float& firstElement )
{
    return KeyValue( elemFromFloatingPoint( firstElement ) );
}

template<>
const KeyValue KeyValue::from<std::wstring>( const std::wstring& firstElement )
{
    return KeyValue( QString::fromStdWString( firstElement ) );
}

template<>
const KeyValue KeyValue::from<bool>( const bool& firstElement )
{
    return KeyValue( firstElement ? trueString : falseString );
}

template<>
const KeyValue KeyValue::from<CvMat>( const CvMat& matrix )
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

template<>
const KeyValue KeyValue::from<cv::Mat>( const cv::Mat& matrix )
{
    return from( static_cast< CvMat >( matrix ) );
}

template<>
const KeyValue KeyValue::from< std::vector< cv::Point2f > >(
                                const std::vector< cv::Point2f >& vec )
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

#endif
