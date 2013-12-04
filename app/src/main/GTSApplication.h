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

#include <QApplication>

/**
  @brief Subclass of QApplication to allow us to catch exceptions by overriding notify function
 */
class GTSApplication : public QApplication
{
public:
    /**
      @brief Initialise base class by direct pass of parameters from command line
      @param argc Passed directly to base class
      @param argv Passed directly to base class
     */
    GTSApplication(int& argc, char* argv[]);

    /**
      @brief Re-implementation of notify function in QApplication to allow
      catching of exceptions in app.
      @param receiver
      @param event
      @return whether or not exception caught
     */
    virtual bool notify(QObject* receiver, QEvent* event);
};
