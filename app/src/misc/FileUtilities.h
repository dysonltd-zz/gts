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

/**
  Utilities to support different file/directory related tasks
 */
namespace FileUtilities
{
    /**
      @brief GetUniqueFileName
      @param fileNameFormat
      @return
     */
    const QString GetUniqueFileName(const QString& fileNameFormat);

    /**
      @brief Delete a directory
      @param dirName Path to directory
      @return True if successfully deleted
     */
    bool DeleteDirectory(const QString& dirName);

    /**
      @brief Check if a file is external to the workbench (i.e. the workbench config)
      @param fileName Path to file
      @param config The workbench config
      @return True if it is external
     */
    bool FileIsExternal(const QString& fileName, const WbConfig& config);

    /**
      @brief Check if a file exists by opening it. Then close it.
      @param file File path
      @return True If file exists
     */
    bool FileExists(const char* filePath);

    /**
      @brief Open a directory in the OS's specific window explorer
      @param dirName Director to be opened
     */
    void ShowInGraphicalShell(const QString &dirName);

    /**
      @brief Skip to the next line (look for '\n') of file as
      long as its not the end of file
      @param f Pointer to file
     */
    void LineSkip(FILE* fp);

    /**
      @brief Go through the entire file counting the number of
      newline ('n') characters
      @param f Pointer to file
     */
	int LineCount(FILE* fp);
}

#endif // FILEUTILITIES_H

