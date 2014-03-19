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

#ifndef  FILEUTILITIES_H
#define  FILEUTILITIES_H

#include "WbConfig.h"

#include <QtCore/QString>

#include <stdio.h>

namespace FileUtilities
{
    const QString GetUniqueFileName( const QString& fileNameFormat );

    bool DeleteDirectory( const QString& dirName );
    bool FileIsExternal( const QString& fileName, const WbConfig& config );
    bool FileExists( const char* file );
    void ShowInGraphicalShell( const QString &dirName );
    void LineSkip( FILE* f );
	int LineCount( FILE* fp );
}

#endif // FILEUTILITIES_H

