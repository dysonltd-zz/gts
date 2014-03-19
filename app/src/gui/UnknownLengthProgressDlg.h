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

#ifndef UNKNOWNLENGTHPROGRESSDLG_H_
#define UNKNOWNLENGTHPROGRESSDLG_H_

#include <QtGui/QProgressBar>
#include <QtGui/QWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QCloseEvent>

class UnknownLengthProgressDlg : public QWidget
{
    Q_OBJECT

public:
    UnknownLengthProgressDlg( QWidget* const parent = 0 );

    void Start( const QString& title, const QString& message );
    void Complete( const QString& title, const QString& message, const QString& filePath = "");
    void ForceClose();

protected:
    virtual void closeEvent( QCloseEvent* event );

private slots:
    void ShowInGraphicalShell(const QString &dirPath);

private:
    void SetLabelText( const QString& message );
    void AdjustGeometry();

    QProgressBar* m_bar;
    QGridLayout*  m_layout;
    QLabel*       m_label;
    bool          m_allowClose;
    QString*      m_filePath;
};


#endif

