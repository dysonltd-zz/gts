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

#include "WorkbenchUi.h"

#include "ui_WorkbenchUi.h"

#include "PositionCollectionTool.h"
#include "TargetCollectionTool.h"
#include "CameraCollectionTool.h"
#include "RoomCollectionTool.h"
#include "RobotCollectionTool.h"
#include "RunCollectionTool.h"
#include "AnalysisTool.h"

#include "CameraHardware.h"
#include "Workbench.h"
#include "WbDefaultKeys.h"
#include "Message.h"
#include "HardwareAbstraction.h"
#include "MainWindow.h"
#include "ToolTabsContainerWidget.h"
#include "ScopedQtSignalsBlocker.h"
#include "XmlConfigFileReader.h"

#include "Version.h"
#include "Debugging.h"

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QSettings>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QDialogButtonBox>

namespace
{
    const bool DONT_UPDATE_WORKBENCH_TREE = false;

    /** @brief Functor to collect current active WbPath information
     * from the active tools
     */
    struct ActivePathCollector : public ToolFunction
    {
        ActivePathCollector( const WbSchema& workbenchSchema ) :
            m_activePath   (),
            m_workbenchSchema( workbenchSchema )
        {
        }

        /** @brief override the current idea of path to lowest active tool with
         * path to current tool.
         *
         * @param tool the current too to consider.
         */
        virtual void operator() ( ToolInterface& tool )
        {
            m_activePath =
                 m_workbenchSchema.FindPathToSchema( tool.GetMostSpecificSubSchema() );
        }

        /** @brief Retrieve the active path
         *
         * @return The path to the lowest active tool
         */
        const WbPath GetActivePath() const
        {
            return m_activePath;
        }

    private:
         WbPath   m_activePath;
         WbSchema m_workbenchSchema;
    };

    /** @brief Functor to update the tools with new configs, and the UI based
     * on the active Tools.
     */
    struct ToolUpdater : public ToolFunction
    {
        ToolUpdater( QMenu* const toolMenu,
                     const WbConfig& config ) :
            m_toolMenu   ( toolMenu ),
            m_config     ( config )
        {
            if ( m_toolMenu ) m_toolMenu->clear();
        }

        ~ToolUpdater()
        {
            if ( m_toolMenu )
            {
                const bool shouldEnable = !m_toolMenu->isEmpty() && !m_config.IsNull();
                m_toolMenu->setEnabled( shouldEnable );
                foreach( QAction* action,  m_toolMenu->actions() )
                {
                    action->setEnabled( shouldEnable );
                }
            }
        }

        virtual void operator() ( ToolInterface& tool )
        {
            tool.Reload( m_config );

            if ( m_toolMenu ) tool.UpdateToolMenu( *m_toolMenu );
        }

     private:
         QMenu*   m_toolMenu;
         WbConfig m_config;
     };

    /** @brief Functor to update the Workbench tree widget based on the currently
     * active tools
     */
    struct WorkbenchTreeUpdater : public ToolFunction
    {
        WorkbenchTreeUpdater( QTreeWidget& treeWidget ) :
            m_treeWidget( treeWidget )
        {
        }

        virtual void operator() ( ToolInterface& tool )
        {
            ScopedQtSignalsBlocker blockItemChangedSignals( m_treeWidget );
            const WbConfig activeConfig = tool.GetCurrentConfig();
            if ( !activeConfig.IsNull() )
            {
                QTreeWidgetItem* const linkedTreeWidgetItem = activeConfig.GetLinkedTreeItem();
                m_treeWidget.setCurrentItem( linkedTreeWidgetItem );
                if ( linkedTreeWidgetItem )
                {
                    linkedTreeWidgetItem->setSelected( true );
                    linkedTreeWidgetItem->setExpanded( true );
                }
            }
        }

    private:
        QTreeWidget& m_treeWidget;
    };

}

WorkbenchUi::WorkbenchUi( MainWindow& mainWindow ) :
    QWidget              ( &mainWindow ),
    m_ui                 ( new Ui::WorkbenchUiClass ),
    m_workbench          (),
    m_cameraHardware     ( HardwareAbstraction::CreateCameraHardware() ),
    m_toolMenu           ( 0 ),
    m_toolTabs           ( 0 ),
    m_itemToSwitchBackTo ( 0 ),
    m_activePath         (),
    m_mainWindow         ( mainWindow )
{
    m_ui->setupUi(this);

    CreateToolTabs( mainWindow );
    CreateTools( mainWindow );
    SetupSplitter();
    SetupWorkbench();
    ConnectSignals();
}

WorkbenchUi::~WorkbenchUi()
{
}

void WorkbenchUi::SetupSplitter()
{
    const int workbenchPane = 0;
    const int toolsPane = 1;
    m_ui->m_splitter->setCollapsible( workbenchPane, true );
    m_ui->m_splitter->setCollapsible( toolsPane, false );
}

void WorkbenchUi::SetupWorkbench()
{
    m_workbench.reset( new Workbench( *m_ui->m_workbenchTree, *m_toolTabs ) );

    SetUpWorkbenchSchema();

    const QString workbenchToOpen( GetWorkbenchToOpenAtStartup() );
    m_activePath = GetActivePathFromTools();

    if ( !workbenchToOpen.isEmpty() )
    {
        OpenWorkbench( workbenchToOpen );
    }
}

void WorkbenchUi::ConnectSignals()
{
    QObject::connect( m_ui->m_workbenchTree,
                      SIGNAL( currentItemChanged( QTreeWidgetItem*,
                                                  QTreeWidgetItem* ) ),
                      this,
                      SLOT( TreeCurrentItemChanged( QTreeWidgetItem*,
                                                    QTreeWidgetItem* ) ) );

    QObject::connect( m_ui->m_workbenchTree,
                      SIGNAL( itemSelectionChanged() ),
                      this,
                      SLOT( TreeSelectionChanged() ) );

    QObject::connect( m_workbench->GetConfigListener(),
                      SIGNAL( changed() ),
                      this,
                      SLOT( ConfigChanged() ) );
}

const QString WorkbenchUi::GetWorkbenchToOpenAtStartup() const
{
    QString workbenchToOpen( TryToGetWorkbenchFromCmdLineArgs() );

    if ( workbenchToOpen.isEmpty() )
    {
        QSettings settings;

        workbenchToOpen = settings.value( "wb/lastOpenWorkbench", QString() ).toString();
    }

    return workbenchToOpen;
}

const QString WorkbenchUi::TryToGetWorkbenchFromCmdLineArgs() const
{
    QString workbenchInArgs;
    const QStringList cmdLineArgs( QApplication::instance()->arguments() );
    const QRegExp workbenchOptions( tr( "^(?:-w=|--workbench=)(.*)$" ) );

    const QStringList workbenchArgs( cmdLineArgs.filter( workbenchOptions ) );

    if ( workbenchArgs.size() > 0 )
    {
        const int pos = workbenchOptions.indexIn( workbenchArgs.at( 0 ) );
        assert( pos != -1 );
        if ( pos != -1 )
        {
            workbenchInArgs = workbenchOptions.cap( 1 );
        }
    }

    return workbenchInArgs;
}

const WbPath WorkbenchUi::GetActivePathFromTools()
{
    ActivePathCollector pathCollector( m_workbench->Schema() );
    m_toolTabs->CallOnActiveTools( pathCollector );

    const WbPath activePathFromTools( pathCollector.GetActivePath() );
    return activePathFromTools;
}

bool WorkbenchUi::HasOpenModifiedWorkbench() const
{
    if ( !m_workbench.get() )
    {
        return false;
    }

    if ( !m_workbench->GetCurrentConfig().IsModified() )
    {
        return false;
    }

    return true;
}

void WorkbenchUi::SetUpWorkbenchSchema()
{
    WbSchema schema( KeyName( "workbench" ) );
    schema.AddSingleValueKey(WbDefaultKeys::displayNameKey,
                             WbSchemaElement::Multiplicity::One,
                             KeyValue::from(tr("Workbench")));

    m_toolTabs->AddToolsFullWorkbenchSchemaSubTreeTo(schema);
    m_workbench->SetSchema(schema);
}

void WorkbenchUi::CreateToolTabs( MainWindow& mainWindow )
{
    m_toolTabs = new ToolTabsContainerWidget( &mainWindow, m_ui->m_splitter );
    m_toolTabs->setObjectName( QString::fromUtf8("m_toolTabs") );

    QSizePolicy tabsSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding);
    tabsSizePolicy.setHorizontalStretch( 5 );
    tabsSizePolicy.setVerticalStretch  ( 0 );
    m_toolTabs->setSizePolicy( tabsSizePolicy );

    m_ui->m_splitter->addWidget( m_toolTabs );
}

void WorkbenchUi::CreateTools( MainWindow& mainWindow )
{
    m_toolTabs->AddTool( new TargetCollectionTool ( this, mainWindow ) );
    m_toolTabs->AddTool( new RobotCollectionTool  ( this, mainWindow ) );
    m_toolTabs->AddTool( new CameraCollectionTool( *m_cameraHardware, this, mainWindow ) );
    m_toolTabs->AddTool( new PositionCollectionTool( *m_cameraHardware,
                                                     this,
                                                     mainWindow ) );
    m_toolTabs->AddTool( new RoomCollectionTool ( *m_cameraHardware, this, mainWindow ) );
    m_toolTabs->AddTool( new RunCollectionTool ( *m_cameraHardware, this, mainWindow ) );
    m_toolTabs->AddTool( new AnalysisTool ( this, mainWindow ) );
}

void WorkbenchUi::Reload( const bool updateWorkbench )
{
    if ( this && m_ui.get() )
    {
        MergeWithActivePath( GetActivePathFromTools() );

        ToolUpdater toolUpdater( m_toolMenu,
                                 m_workbench->GetCurrentConfig()
                                                 .GetFromPath( m_activePath ) );

        m_toolTabs->CallOnActiveTools( toolUpdater );

        if ( updateWorkbench )
        {
            m_workbench->ReloadConfig();
            WorkbenchTreeUpdater workbenchTreeUpdater( *m_ui->m_workbenchTree );
            m_toolTabs->CallOnActiveTools( workbenchTreeUpdater );
        }
    }
}

void WorkbenchUi::OpenWorkbench()
{
    QString initialDir;

    QSettings settings;
    const QString lastWb = settings.value( "wb/lastOpenWorkbench", QString() ).toString();

    if (lastWb.isEmpty())
    {
        initialDir = QApplication::applicationDirPath();
    }
    else
    {
        initialDir = QFileInfo(lastWb).absolutePath();
    }

    const QString workbenchConfigFileName = QFileDialog::getOpenFileName( this,
                                                                          tr( "Open Workbench" ),
                                                                          initialDir,
                                                                          tr("XML workbench files (*.xml )") );

    OpenWorkbench( workbenchConfigFileName );
}

void WorkbenchUi::OpenWorkbench( const QString& workbenchConfigFileName )
{
    if ( !workbenchConfigFileName.isEmpty() )
    {
        if ( m_workbench->Open( QFileInfo( workbenchConfigFileName ) ) )
        {
            Reload();

            QSettings settings;
            settings.setValue( "wb/lastOpenWorkbench", workbenchConfigFileName );

            QString title = QString("Ground Truth System (v%1) - %2").arg(GTS_BUILD_REVN).arg(workbenchConfigFileName);
            m_mainWindow.setWindowTitle(title);
        }
        else
        {
            QMessageBox question( QMessageBox::Information,
                                  tr("Create New Workbench or Open Existing Workbench?"),
                                  tr("Create New Workbench or Open Existing Workbench?"),
                                  QMessageBox::Open | QMessageBox::Close,
                                  this );
            question.addButton("New", QMessageBox::ActionRole);
            question.setDefaultButton(QMessageBox::Close);

            const int response = question.exec();

            switch (response)
            {
                case QMessageBox::Open:
                    OpenWorkbench();
                    break;

                case QMessageBox::Close:
                    // use std::exit as qApp->closeAllWindows() would not work yet
                    // since the app has not started running!
                    std::exit(0);
                    break;

                // Create new workbench
                default:
                    NewWorkbench();
                    break;
            }
        }
    }
}

void WorkbenchUi::NewWorkbench()
{
    QString workbenchConfigFolderName = QFileDialog::getExistingDirectory(this,
                                                                          tr("Create Workbench"),
                                                                          getenv("HOME"),
                                                                          QFileDialog::ShowDirsOnly);

    QString workbenchConfigFileName = QString("%1/%2").arg(workbenchConfigFolderName).
                                                        arg("workbench.xml");

    if ( m_workbench->New( QFileInfo( workbenchConfigFileName ) ) )
    {
        QSettings settings;
        settings.setValue( "wb/lastOpenWorkbench", workbenchConfigFileName );

        Reload();

        QString title = QString("Ground Truth System (v%1) - %2").arg(GTS_BUILD_REVN).arg(workbenchConfigFileName);
        m_mainWindow.setWindowTitle(title);
        SaveWorkbench();
    }
    else
    {
        Message::Show( this,
                       tr( "New Workbench" ),
                       tr( "Error - Failed to create workbench!" ),
                       Message::Severity_Critical );
    }
}

void WorkbenchUi::SaveWorkbench()
{
    if ( m_workbench->Save() )
    {
        Reload();

        GetCornerWidget()->setVisible( false );
    }
    else
    {
        Message::Show( this,
                       tr( "Save Workbench" ),
                       tr( "Error - Failed to save workbench!" ),
                       Message::Severity_Critical );
    }
}

void WorkbenchUi::ConfigChanged()
{
    GetCornerWidget()->setVisible( true );
}

void WorkbenchUi::MergeWithActivePath( const WbPath& desiredPath )
{
    m_activePath = m_activePath.BestFitWith( desiredPath );
}

void WorkbenchUi::SwitchBackToItemAfterSelectionChanges( QTreeWidgetItem* const item )
{
    // Qt may deliver this selectionChanged signal before or after currentItemChanged
    // So we need to call TreeSelectionChanged() twice -- once here, once when the
    // selection actually changes
    m_itemToSwitchBackTo = item;
    TreeSelectionChanged();
    m_itemToSwitchBackTo = item;
}

void WorkbenchUi::TreeSelectionChanged()
{
    if ( m_itemToSwitchBackTo )
    {
        ScopedQtSignalsBlocker blockAnotherItemChangeSignal( *m_ui->m_workbenchTree );
        m_ui->m_workbenchTree->setCurrentItem( m_itemToSwitchBackTo );
        m_itemToSwitchBackTo = 0;
    }
}

void WorkbenchUi::TreeCurrentItemChanged( QTreeWidgetItem* current,
                                          QTreeWidgetItem* previous )
{
    m_itemToSwitchBackTo = 0;
    if ( current && ( current != previous ) )
    {
        if ( m_workbench->ActivateToolFor( *current ) )
        {
            MergeWithActivePath( WbPath::FromWbConfig(
                                            WbConfig::FromTreeItem( *current ) ) );
            Reload( DONT_UPDATE_WORKBENCH_TREE );
        }
        else
        {
            SwitchBackToItemAfterSelectionChanges( previous );
        }
    }
}

void WorkbenchUi::SetToolMenu( QMenu& toolMenu )
{
    m_toolMenu = &toolMenu;
}

void WorkbenchUi::SetCornerWidget( QWidget* const widget )
{
    m_toolTabs->setCornerWidget( widget );
    m_toolTabs->cornerWidget()->setVisible( false );
}

QWidget* WorkbenchUi::GetCornerWidget()
{
    return m_toolTabs->cornerWidget();
}
