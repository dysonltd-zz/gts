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

#include "Debugging.h"

#ifndef NDEBUG
namespace Debugging
{
    std::unique_ptr< DestructorPrinter > Print( const QString& file,
                             const int line,
                             const QString& function,
                             const QString& msg )
    {
        qDebug() << "At: " << file << ":" << line << " in function: " << function << "\n\t";
        return std::unique_ptr< DestructorPrinter >(new DestructorPrinter(msg));
    }
}
#endif
