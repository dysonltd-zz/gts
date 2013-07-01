#ifndef MOCKIODEVICE_H
#define MOCKIODEVICE_H

#include <QtCore/qiodevice.h>

class MockIoDevice : public QIODevice
{
    Q_OBJECT

public:
    MockIoDevice()
        :
        m_openSuccessful( false )
    {
    }

    bool m_openSuccessful;
    OpenMode m_openModeRequested;

    bool open( OpenMode mode )
    {
        m_openModeRequested = mode;
        return m_openSuccessful;
    }

protected:
    qint64 readData( char* data, qint64 maxSize )
    {
        (void) data;
        (void) maxSize;
        return 0;
    }

    qint64 writeData( const char* data, qint64 maxSize )
    {
        (void) data;
        (void) maxSize;
        return 0;
    }
};

#endif
