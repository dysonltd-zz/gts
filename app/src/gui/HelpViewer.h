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

#include <QtGui/QWidget>
#include <QtGui/QToolButton>

#include <QObject>
#include <QProcess>

namespace Ui
{
    class HelpViewer;
}

enum HelpBookmark
{
    HELP_GENERAL,
    HELP_CREATING_A_WORKBENCH,
    HELP_CONFIGURING_YOUR_ROBOT,
    HELP_CALIBRATING_THE_CAMERA,
    HELP_SETTING_UP_THE_CAMERA_POSITIONS,
    HELP_CREATING_ROOMS,
    HELP_RECORDING_A_RUN,
    HELP_TRACKING_THE_ROBOT,
    HELP_ANALYSING_COVERAGE,
    HELP_INVALID
};

const size_t NumHelpBookmarks = static_cast<size_t>( HELP_INVALID );

const QString helpGeneral                  ( "gts_userguide.htm" );
const QString helpCreatingAWorkbench       ( "gts_userguide.htm#CREATING_A_WORKBENCH" );
const QString helpConfiguringARobot        ( "gts_userguide.htm#CONFIGURING_YOUR_ROBOT" );
const QString helpCalibratingACamera       ( "gts_userguide::htm#CALIBRATING_THE_CAMERA" );
const QString helpSettingUpCameraPositions ( "gts_userguide.htm#SETTING_UP_THE_CAMERA_POSITIONS" );
const QString helpCreatingRooms            ( "gts_userguide.htm#CREATING_ROOMS" );
const QString helpRecodingARun             ( "gts_userguide.htm#RECORDING_A_RUN" );
const QString helpTrackingTheRobot         ( "gts_userguide.htm#TRACKING_THE_ROBOT" );
const QString helpAnalysingCoverage        ( "gts_userguide.htm#ANALYSING_COVERAGE" );

const QString helpBookmark[ NumHelpBookmarks ] =
{
    helpGeneral,
    helpCreatingAWorkbench,
    helpConfiguringARobot,
    helpCalibratingACamera,
    helpSettingUpCameraPositions,
    helpCreatingRooms,
    helpRecodingARun,
    helpTrackingTheRobot,
    helpAnalysingCoverage
};

/**
 *  @todo Write help text for each Tool.
 */
class HelpViewer : public QWidget
{
    Q_OBJECT

public:
    explicit HelpViewer( QWidget* parent = 0 );
    ~HelpViewer();

    void SetUiWidget( QWidget* const widget );

    QToolButton* const GetShowHelpBtn() const;

    void showPage( const QString &page );

	void preparePage( HelpBookmark bookmark );
    void requestPage( HelpBookmark bookmark );

public:
    void Show();
	void OnHelpChange();
    void Close();

public slots:
	void OnStartedHelp();
	void OnEndHelp( int , QProcess::ExitStatus );

private slots:
    void ShowHelpClicked();

private:
    void showPage( HelpBookmark bookmark );

private:
	bool             m_help_exec;

    Ui::HelpViewer*  m_ui;

    QToolButton*     m_showHelpBtn;

    HelpBookmark     m_pendingBookmark;

    QProcess*        m_process_help;
};

#endif // HELPVIEWER_H
