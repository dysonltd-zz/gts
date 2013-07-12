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

#include "FileUtilities.h"

#include <QtCore/QFileInfo>
#include <QtCore/QFile>
#include <QtCore/QDir>

namespace FileUtilities
{
    const QString GetUniqueFileName( const QString& fileNameFormat )
    {
        QString newFileName;
        size_t counter = 0;

        do
        {
            newFileName = fileNameFormat.arg( counter );
            counter++;
        }
        while ( QFileInfo( newFileName ).exists() );

        return newFileName;
    }

    bool DeleteDirectory( const QString& dirName )
    {
        bool ok = true;
        QDir dir(dirName);

        if (dir.exists(dirName))
        {
            Q_FOREACH(QFileInfo info,
                      dir.entryInfoList(QDir::NoDotAndDotDot |
                                        QDir::System |
                                        QDir::Hidden  |
                                        QDir::AllDirs |
                                        QDir::Files, QDir::DirsFirst))
            {
                if (info.isDir())
                {
                    ok = DeleteDirectory( info.absoluteFilePath() );
                }
                else
                {
                    ok = QFile::remove( info.absoluteFilePath() );
                }

                if (!ok)
                {
                    return ok;
                }
            }

            ok = dir.rmdir(dirName);
        }

        return ok;
    }

    bool FileIsExternal( const QString& fileName, const WbConfig& config )
    {
        const QString absoluteFileName(
                config.GetAbsoluteFileNameFor( fileName ) );

        const WbConfig topLevelCfg( config.FindRootAncestor() );
        const QDir topLevelDir( topLevelCfg.GetAbsoluteFileInfo().absoluteDir() );

        return topLevelDir.relativeFilePath( absoluteFileName ).startsWith( ".." );
    }

    void LineSkip(FILE* f)
    {
        char c;

        do
        {
            c = (char)( fgetc(f) );
        }
        while (c!='\n' && c!=EOF);
    }

    int LineCount(FILE* fp)
    {
        int n = 0;
        char c;

        do
        {
            c = (char)( fgetc(fp) );
            if (c=='\n') n++;
        }
        while (c!=EOF);

        //fseek( fp, 0, SEEK_SET ); // rewind file
        rewind(fp);

        return n;
    }

    bool FileExists( const char* file )
    {
        FILE* fp = fopen( file, "r" );

        if (fp)
        {
            fclose( fp );
            return true;
        }

        return false;
    }
}
