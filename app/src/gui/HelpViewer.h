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

#ifndef HELPVIEWER_H
#define HELPVIEWER_H

#include <QObject>
#include <QProcess>

/**
  @brief Manages the Help Viewer - loads relevant file and handles open/exit
  of the new process that is launched
 */
class HelpViewer : public QObject
{
    Q_OBJECT

public slots:
    /**
      @brief Handles graceful exit of Help Window
      params match signal signature
     */
    void OnEndHelp(int, QProcess::ExitStatus);

public:
    HelpViewer();
    ~HelpViewer();

    void Show();
    void Close();

private slots:
    void ShowHelp();

private:
    QProcess*   m_helpProcess;
    bool        m_helpLaunched;
};

#endif // HELPVIEWER_H
