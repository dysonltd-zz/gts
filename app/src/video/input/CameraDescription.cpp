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

#include "CameraDescription.h"
#include <QtCore/qcoreapplication.h>
#include <QtGui/QTableWidgetItem>
#include <memory>
#include "VideoSequence.h"

/** @brief Create a null CameraDescription.
 *
 *  Used as an invalid return type.
 */
CameraDescription::CameraDescription()
    :
    m_isOfflineFileOnly(false),
    m_api        ( 0 ),
    m_name       (),
    m_description(),
    m_uniqueId   (),
    m_apiIndex   ( static_cast< size_t >( -1 ) )
{
}

/** @brief Create a valid CameraDescription.
 *
 *  @param api The API in which the camera was enumerated.
 */
CameraDescription::CameraDescription( const CameraApi& api )
    :
    m_isOfflineFileOnly(false),
    m_api        ( &api ),
    m_name       (),
    m_description(),
    m_uniqueId   (),
    m_apiIndex   ( static_cast< size_t >( -1 ) )
{
}

CameraDescription::CameraDescription(const bool isOffline)
    :
    m_isOfflineFileOnly(isOffline),
    m_api        ( 0 ),
    m_name       (),
    m_description(),
    m_uniqueId   (),
    m_apiIndex   ( static_cast< size_t >( -1 ) )
{
}

const CameraDescription CameraDescription::CreateOffline()
{
    return CameraDescription(true).WithDescription(QObject::tr("*** OFFLINE CAMERA ***").toStdWString());
}

/** @brief Is this a valid camera?
 *
 *  @return @a true if the camera is a valid real camera, @a false
 *  if it's a null one.
 */
bool CameraDescription::IsValid() const
{
    return m_isOfflineFileOnly || (m_api != 0);
}


/** @brief The friendly name of the camera. */
const std::wstring& CameraDescription::Name() const
{
    return m_name;
}

/** @brief The friendly description of the camera (more detailed than name). */
const std::wstring& CameraDescription::Description () const
{
    return m_description;
}

/** @brief The unique id the camera. */
const std::wstring& CameraDescription::UniqueId() const
{
    return m_uniqueId;
}

/** @brief Create a VideoSequence for this camera.
 *
 *  Create a camera using the API that this camera was enumerated from.
 *
 *  @param fps The frame rate to set on the camera.  If @a fps <= 0.0 then
 *  a default of 7.5 FPS is set.
 *
 *  @return A pointer to a newly-allocated VideoSequence getting images from
 *  this camera. If this is a null description a null pointer is returned. The
 *  caller takes ownership if the pointer is valid.
 */
VideoSequence* const CameraDescription::CreateVideoSequence( double fps ) const
{
    VideoSequence* newSequence = 0;

    if ( m_api ) { newSequence = m_api->CreateVideoSequenceForCamera( *this ); }

    if ( newSequence )
    {
        double newFrameRate = m_frameRate;
        if ( fps > 0.0 ) { newFrameRate = fps; }

        newSequence->SetFrameRate( newFrameRate );
    }

    return newSequence;
}

/** Make a copy of this description with the friendly name set to @a name.
 *
 *  @param name The name to set.
 *  @return A CameraDescription which is identical to this one except that it has
 *  its name set to @a name.
 */
const CameraDescription CameraDescription::WithName( const std::wstring& name ) const
{
    CameraDescription newDescription( *this );
    newDescription.m_name = name;
    return newDescription;
}

/** Make a copy of this description with the friendly description set to @a description.
 *
 *  @param description The description to set.
 *  @return A CameraDescription which is identical to this one except that it has
 *  its description set to @a description.
 */
const CameraDescription CameraDescription::WithDescription( const std::wstring& description ) const
{
    CameraDescription newDescription( *this );
    newDescription.m_description = description;
    return newDescription;
}

/** Make a copy of this description with the unique id set to @a uniqueId.
 *
 *  @param uniqueId The unique id to set.
 *  @return A CameraDescription which is identical to this one except that it has
 *  its unique id set to @a uniqueId.
 */
const CameraDescription CameraDescription::WithUniqueId( const std::wstring& uniqueId ) const
{
    CameraDescription newDescription( *this );
    newDescription.m_uniqueId = uniqueId;
    return newDescription;
}

const QString CameraDescription::ToPlainText() const
{
    const QString plainText( QString( "%1\n%2\n%3" )
                            .arg( QString::fromStdWString( Name() ) )
                            .arg( QString::fromStdWString( Description() ) )
                            .arg( QString::fromStdWString( UniqueId() ) ) );
    return plainText;
}

const QString CameraDescription::ToRichText() const
{
    const QString richText( QCoreApplication::translate( "CameraDescription",
                              "<head/>\n"
                              "<body>\n"
                                  "<p><strong>Name:</strong> %1</p>\n"
                                  "<p><strong>Description:</strong> %2</p>\n"
                                  "<p><strong>Unique ID:</strong> %3</p>\n"
                              "</body>" )
                                .arg( QString::fromStdWString( Name() ) )
                                .arg( QString::fromStdWString( Description() ) )
                                .arg( QString::fromStdWString( UniqueId() ) ) );
    return richText;
}

/** @brief Create a QTableWidgetItem to represent the camera.
 *
 * @return The new QTableWidgetItem.  The caller takes ownership.
 */
QTableWidgetItem* const CameraDescription::CreateTableItem() const
{
    QTableWidgetItem* newItem = new QTableWidgetItem(QString::fromStdWString(Name()));
    newItem->setToolTip(ToRichText());
    return newItem;
}

const QSize CameraDescription::GetImageSize() const
{
     /// @todo Add some error checking to video sequence creation?
    std::auto_ptr< VideoSequence > videoSequence( CreateVideoSequence( -1.0 ) );

    const int width  = videoSequence->GetFrameWidth();
    const int height = videoSequence->GetFrameHeight();

    return QSize( width, height );
}
