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

#include "MainWindow.h"

#include "ui_MainWindow.h"

#include "WorkbenchUi.h"
#include "HelpViewer.h"
#include "AboutDialog.h"

#include <QtGui/QToolButton>
#include <QtGui/QMessageBox>
#include <QtGui/qevent.h>

#include <cassert>

#if LEAK_DETECTION
#include "vld.h"
#endif

namespace
{
    const QMessageBox::StandardButton AskUserToSave( QWidget* const dialogParent )
    {
        return QMessageBox::question( dialogParent,
                                      QObject::tr( "Save work?", "MainWindow" ),
                                      QObject::tr( "You are about to quit. "
                                                   "Would you like to save the "
                                                   "changes to your workbench?" ),
                                      QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel,
                                      QMessageBox::Cancel );
    }
}

MainWindow::MainWindow( QWidget* parent ) :
    QMainWindow   ( parent ),
    m_ui          ( new Ui::MainWindow ),
    m_helpViewer  ( new HelpViewer ),
    m_workbenchUi ()
{
    m_ui->setupUi( this );
}

void MainWindow::Start()
{
#if LEAK_DETECTION
    VLDEnable();
#endif

    m_workbenchUi = new WorkbenchUi( *this );
    m_workbenchUi->SetToolMenu( *m_ui->m_toolMenu );

    m_cornerButton = new QToolButton();
    m_cornerButton->setIcon( QIcon( ":/save.png" ) );
    m_cornerButton->setPopupMode(QToolButton::InstantPopup);

    m_workbenchUi->SetCornerWidget( m_cornerButton );

    QLayout* centralLayout = m_ui->m_centralwidget->layout();
    assert( centralLayout );
    if ( centralLayout )
    {
        centralLayout->addWidget( m_workbenchUi );
    }

    QObject::connect( m_ui->actionE_xit,
                      SIGNAL( triggered() ),
                      qApp,
                      SLOT( closeAllWindows() ) );
    QObject::connect( m_ui->m_newWorkbenchAction,
                      SIGNAL( triggered() ),
                      m_workbenchUi,
                      SLOT( NewWorkbench() ) );
    QObject::connect( m_ui->m_openWorkbenchAction,
                      SIGNAL( triggered() ),
                      m_workbenchUi,
                      SLOT( OpenWorkbench() ) );
    QObject::connect( m_ui->m_saveWorkbenchAction,
                      SIGNAL( triggered() ),
                      m_workbenchUi,
                      SLOT( SaveWorkbench() ) );

    QObject::connect( m_ui->m_helpAction,
                      SIGNAL( triggered() ),
                      this,
                      SLOT( ShowHelp() ) );
    QObject::connect( m_ui->m_aboutAction,
                      SIGNAL( triggered() ),
                      this,
                      SLOT( ShowAboutGTS() ) );
    QObject::connect( m_ui->m_aboutQtAction,
                      SIGNAL( triggered() ),
                      this,
                      SLOT( ShowAboutQt() ) );

    QObject::connect( m_cornerButton,
	                  SIGNAL( clicked() ),
                      m_workbenchUi,
	                  SLOT( SaveWorkbench() ) );

    m_workbenchUi->Reload();

    show();
}

MainWindow::~MainWindow()
{
    delete m_helpViewer;
    delete m_cornerButton;

    delete m_workbenchUi;

    delete m_ui;
}

void MainWindow::ShowHelp()
{
    m_helpViewer->Show();
}

void MainWindow::ShowAboutGTS()
{
    AboutDialog(this).exec();
}

void MainWindow::ShowAboutQt()
{
    QMessageBox::aboutQt(this);
}

void MainWindow::Reload()
{
    if ( this ) m_workbenchUi->Reload();
}

void MainWindow::MergeWithActivePath( const WbPath& desiredPath )
{
    if ( this ) m_workbenchUi->MergeWithActivePath( desiredPath );
}

void MainWindow::closeEvent( QCloseEvent* event )
{
    if ( m_workbenchUi && m_workbenchUi->HasOpenModifiedWorkbench() )
    {
        const QMessageBox::StandardButton choice = AskUserToSave( this );
        switch ( choice )
        {
            case QMessageBox::Yes:
                m_workbenchUi->SaveWorkbench();
                event->accept();
                break;
            case QMessageBox::No:
                event->accept();
                break;
            case QMessageBox::Cancel:
                event->ignore();
                break;
            default:
                break;
        }
    }
    else
    {
        event->accept();
    }

    m_helpViewer->Close();

#if LEAK_DETECTION
    VLDReportLeaks();
#endif
}
