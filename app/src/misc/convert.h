#ifndef CONVERSION_H
#define CONVERSION_H

#include <sstream>
#include <string>
#include <iomanip>

/// namespace for generally useful conversion functions
namespace Convert
{
    /**    @brief %Convert a \c float to an \c int by rounding to nearest.

        x.5f goes further away from zero.

        @param floatValue A \c float value to be converted.
        @return The value as rounded to an \c int.
    **/
    inline int ToIntByRounding( const float floatValue )
    {
        if ( floatValue >= 0.f )
        {
            return (int)( floatValue + 0.5f );
        }
        else
        {
            return (int)( floatValue - 0.5f );
        }
    }

    /** @brief %Convert a \c std::string to an specified Type in a type-safe manner.

        Types must have a <tt>std::istream& operator >> (std::istream&, Type)</tt> defined
        (as all built-in types do).

        @param stringValue A string represention of the value to be converted.
        @param success     A @c bool which is set to @c true if the conversion is successful @c false otherwise.
        @tparam Type       A template parameter specifying the type to convert to.
        @return            The value as the appropriate type, if the conversion is possible.
    **/
    template <class Type>
    const Type To(const std::string& stringValue, bool& success, const Type& defaultVal = Type())
    {
        std::istringstream iss(stringValue);
        Type return_value(defaultVal);
        iss >> return_value;
        success = static_cast<bool>(iss);
        return return_value;
    }

    /** @brief %Convert a \c std::string to an specified Type in a type-safe manner.

        Types must have a <tt>std::istream& operator >> (std::istream&, Type)</tt> defined
        (as all built-in types do).

        @param stringValue A string represention of the value to be converted.
        @tparam Type       A template parameter specifying the type to convert to.
        @return            The value as the appropriate type.
    **/
    template <class Type>
    const Type To(const std::string& stringValue, const Type& defaultVal = Type())
    {
        bool ignored;
        return To<Type>(stringValue, ignored, defaultVal);
    }

    /** @brief %Convert an arbitrary \c Type to \c std::string in a type-safe manner.

        Types must have an <tt>std::ostream& operator << (std::ostream&, Type)</tt> defined
        (as all built-in types do).

        @param otherType A string representation of the value to be converted.
        @return The value as a default-formatted \c std::string.
    **/
    template <class Type>
    const std::string ToStdString( const Type& otherType )
    {
        std::ostringstream oss;
        oss << std::boolalpha << otherType;
        return oss.str();
    }

    /** @brief %Convert an arbitrary Type to <tt>std::wstring</tt> in a type-safe manner.

        Types must have an <tt>std::wostream& operator << (std::wostream&, Type)</tt> defined
        (as all built-in types do).

        @param otherType A string representation of the value to be converted.
        @return The value as a default-formatted \c std::wstring.
    **/
    template< class Type >
    const std::wstring ToStdWString( const Type& otherType )
    {
        std::wostringstream oss;
        oss << otherType;

        return oss.str();
    }

}

#endif

