#ifndef TESTHELPERS_H_
#define TESTHELPERS_H_
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include "KeyValue.h"

namespace
{
    std::ostream& operator << ( std::ostream& os, const QString& qstring )
    {
        os << qstring.toStdString();
        return os;
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

#endif /* TESTHELPERS_H_ */
