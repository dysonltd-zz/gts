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

#ifndef DEBUGGING_H
#define DEBUGGING_H

#include <cstdio>

#include <QtCore/QDebug>
#include <QtCore/QTime>
#include <QtCore/QString>

#ifndef NDEBUG
#include <iostream>
#include <string>
#include <cstdarg>
#include <cassert>
#include <memory>
#endif

#ifdef _MSC_VER
#define __func__ __FUNCTION__
#endif

#define START_TIMING() QTime TIMING_MACRO_stopwatch; TIMING_MACRO_stopwatch.start();
#define TIME_IT()      std::printf( "Took %d msec\n", TIMING_MACRO_stopwatch.restart() );

#ifndef NDEBUG

#define PRINT( msg ) ( *Debugging::Print( __FILE__, __LINE__, __func__, msg ) )
#define PRINT_VAR( var ) PRINT( QString( #var " = %1" ).arg( var ) )
#define PRINT_VAR_MESSAGE( var, msg ) \
    PRINT( QString( #var " = %1 ( %2 )" ).arg( var ).arg( msg ) )

#define ASSERT( expression ) assert( expression )
#define ASSERT_COMPARISON( value1, value2, op ) do { \
                                                    if ( !( value1 op value2 ) ) \
                                                    { \
                                                        PRINT_VAR( value1 ); \
                                                        PRINT_VAR( value2 ); \
                                                    } \
                                                    assert( value1 op value2 ); \
                                                } while( false )

#define ASSERT_EQUAL( v1, v2 ) ASSERT_COMPARISON( v1, v2, == )
#define ASSERT_LESS_THAN( v1, v2 ) ASSERT_COMPARISON( v1, v2, < )
#define ASSERT_LESS_THAN_OR_EQUAL( v1, v2 ) ASSERT_COMPARISON( v1, v2, <= )
#define ASSERT_GREATER_THAN( v1, v2 ) ASSERT_COMPARISON( v1, v2, > )
#define ASSERT_GREATER_THAN_OR_EQUAL( v1, v2 ) ASSERT_COMPARISON( v1, v2, >= )


namespace Debugging
{
    class DestructorPrinter
    {
    public:
        explicit DestructorPrinter( const QString& qstring ) :
            m_string( qstring )
        {
        }

        ~DestructorPrinter()
        {
            qDebug() << m_string << "\n\n";
        }

        template< class Type >
        DestructorPrinter& With( const Type& value )
        {
            m_string.arg( value );
            return *this;
        }

    private:
        QString m_string;
    };

    std::unique_ptr< DestructorPrinter> Print( const QString& file,
                                    const int line,
                                    const QString& function,
                                    const QString& msg );
}

#else

#define PRINT( msg )
#define PRINT_VAR( msg )
#define PRINT_VAR_MESSAGE( var, msg )

#define ASSERT( expression )
#define ASSERT_COMPARISON( value1, value2, op )
#define ASSERT_EQUAL( v1, v2 )
#define ASSERT_LESS_THAN( v1, v2 )
#define ASSERT_LESS_THAN_OR_EQUAL( v1, v2 )
#define ASSERT_GREATER_THAN( v1, v2 )
#define ASSERT_GREATER_THAN_OR_EQUAL( v1, v2 )

#endif

#endif // DEBUGGING_H
