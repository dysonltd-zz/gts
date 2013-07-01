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

#ifndef CAMERADESCRIPTION_H_
#define CAMERADESCRIPTION_H_

#include <vector>
#include <set>
#include <string>
#include <cassert>

#include "VideoSequence.h"
#include "CameraApi.h"

#include <QtCore/qstring.h>
#include <QtCore/QVariant>
#include <QtCore/qsize.h>

class QTableWidgetItem;

/** @brief Class to describe a camera from a give API.
 *
 *  This class follows a named-parameter pattern to simplify inline
 *  object creation.  Create a description like this:
 *
 *
 */
class CameraDescription
{
public:
    struct Resolution
    {
        int width;
        int height;

        bool operator < (const Resolution& other) const
        {
            const int myRes = width * height;
            const int otherRes = other.width * other.height;

            return myRes < otherRes;
        }

        Resolution(int w, int h) : width(w), height(h) {}
        Resolution() : width(0), height(0) {}
    };

    typedef std::set<Resolution> Resolutions;
    
    CameraDescription();

    explicit CameraDescription( const CameraApi& api );
    static const CameraDescription CreateOffline();

    bool IsValid() const;

    const std::wstring& Name        () const;
    const std::wstring& Description () const;
    const std::wstring& UniqueId    () const;

    VideoSequence* const CreateVideoSequence( const double fps = -1.0 ) const;

    const CameraDescription WithName        ( const std::wstring& name ) const;
    const CameraDescription WithDescription ( const std::wstring& description ) const;
    const CameraDescription WithUniqueId    ( const std::wstring& uniqueId ) const;

    const QString ToPlainText() const;
    const QString ToRichText() const;

    const QSize GetImageSize() const;

    QTableWidgetItem *const CreateTableItem() const;

    void SetResolutions(const Resolutions& resolutions)
    {
        m_resolutions = resolutions;
    }

    const Resolutions& GetResolutions() const
    {
        return m_resolutions;
    }
    
private:
    CameraDescription(const bool isOffline);

    bool m_isOfflineFileOnly; ///< @brief Whether the camera is a real connected camera, or an offline, file-only one.

    const CameraApi* m_api;         ///< @brief The API which enumerated the camera, or 0 if this is a null description.
    std::wstring     m_name;        ///< @copybrief Name()
    std::wstring     m_description; ///< @copybrief Description()
    std::wstring     m_uniqueId;    ///< @copybrief UniqueId()
    size_t           m_apiIndex;    ///< @copybrief ApiIndex()
    Resolutions      m_resolutions; ///< @brief the set of available resolutions
};

Q_DECLARE_METATYPE( CameraDescription )

#endif /* CAMERADESCRIPTIONH_ */
