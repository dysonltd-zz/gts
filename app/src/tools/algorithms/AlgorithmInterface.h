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

#ifndef ALGORITHMINTERFACE_H
#define ALGORITHMINTERFACE_H

/**
  @brief Exit status identifiers to be used when running algorithms.
  Identifies if algorithm has experienced errors.
 **/
namespace ExitStatus
{
    enum
    {
        OK_TO_CONTINUE  = 0,
        ERRORS_OCCURRED = 1,
        HELP_DISPLAYED  = 2
    };

    typedef int Flags;
}

#endif // ALGORITHMINTERFACE_H
