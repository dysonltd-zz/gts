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

#include "ToolTabsContainerWidget.h"
#include "WbSchema.h"
#include "ScopedQtSignalsBlocker.h"
#include "MainWindow.h"
#include "Message.h"

#include <QtGui/QToolButton>
#include <QtGui/QMouseEvent>

#include <cassert>

class ModifiedToolTabs : public QTabBar
{
public:
    ModifiedToolTabs( ToolTabsContainerWidget& tabWidget )
    :
        m_tabWidget( tabWidget )
    {
    }

protected:
    virtual void mousePressEvent( QMouseEvent* event )
    {
        if ( !m_tabWidget.ActiveToolCanClose() )
        {
            m_tabWidget.ShowCannotCloseMessage();
            event->accept();
        }
        else
        {
            QTabBar::mousePressEvent( event );
        }

    }

private:
    ToolTabsContainerWidget& m_tabWidget;
};

ToolTabsContainerWidget::ToolTabsContainerWidget( MainWindow* const mainWindow, QWidget* parent ) :
    QTabWidget( parent ),
    m_mainWindow( mainWindow ),
    m_tools()
{
    setTabBar( new ModifiedToolTabs( *this ) );

    QObject::connect( this,
                      SIGNAL( currentChanged( int ) ),
                      this,
                      SLOT( CurrentTabChanged( const int ) ) );
}

ToolTabsContainerWidget::~ToolTabsContainerWidget()
{
    ScopedQtSignalsBlocker blockChangePageSignalsDuringClear( *this );
    // Make sure tab widget doesn't delete the tab page widgets
    // -- that's the Tool's responsibility
    clear();
}

void ToolTabsContainerWidget::AddTool( ToolInterface* const tool )
{
    addTab( tool->Widget(), tool->Name() );
    m_tools.push_back( std::shared_ptr<ToolInterface>(tool) );

    tabBar()->setHidden( count() == 1 );

}

void ToolTabsContainerWidget::ShowCannotCloseMessage()
{
    Message::Show( this,
                   tr( "Stop!" ),
                   ActiveToolCannotCloseReason(),
                   Message::Severity_NonBlockingInfo );
}

void ToolTabsContainerWidget::ActivateTabWithoutInvokingSignals( const size_t tabIndex )
{
    ScopedQtSignalsBlocker blockChangePageSignals( *this );
    this->setCurrentIndex( static_cast<int>( tabIndex ) );
}

bool ToolTabsContainerWidget::TryToOpenTool( const WbConfig& config )
{
    bool openedAnyTool = false;
    size_t index = 0;
    for (auto tool  = m_tools.begin(); tool != m_tools.end(); ++tool)
    {
        const bool openedThisTool = (*tool)->TryToOpenTool( config );

        if ( !openedAnyTool && openedThisTool )
        {
            ActivateTabWithoutInvokingSignals(index);
            openedAnyTool = true;
        }
        ++index;
    }

    return openedAnyTool;
}

void ToolTabsContainerWidget::CallOnActiveTools( ToolFunction& func )
{
    const ToolPtr activeTab = FindActiveTool();
    if ( activeTab != 0 )
    {
        activeTab->CallOnSelfAndActiveSubTools( func );
    }
}

const ToolTabsContainerWidget::ToolPtr ToolTabsContainerWidget::FindActiveTool() const
{
    const int activeTab = currentIndex();
    if ( ( activeTab >= 0 ) && ( activeTab < (int)m_tools.size() ) )
    {
        return m_tools.at( activeTab );
    }
    return ToolPtr();
}

void ToolTabsContainerWidget::AddToolsFullWorkbenchSchemaSubTreeTo( WbSchema& parentSchema,
                                                                    const KeyName& schemaToAddTo )
{
    for (auto tool = m_tools.begin(); tool != m_tools.end(); ++tool)
    {
        (*tool)->AddFullWorkbenchSchemaSubTreeTo( parentSchema, schemaToAddTo );
    }
}

void ToolTabsContainerWidget::AddToolsFullWorkbenchSchemaSubTreeTo( WbSchema& parentSchema )
{
    AddToolsFullWorkbenchSchemaSubTreeTo( parentSchema, parentSchema.Name() );
}

void ToolTabsContainerWidget::CurrentTabChanged( const int newTabIndex )
{
    Q_UNUSED(newTabIndex);

    if ( m_mainWindow )
    {
        m_mainWindow->Reload();
    }

    const ToolPtr activeTab = FindActiveTool();
    if ( activeTab != 0 )
    {
        activeTab->Activated();
    }
}

bool ToolTabsContainerWidget::ActiveToolCanClose() const
{
    const ToolPtr activeTab = FindActiveTool();
    if ( activeTab != 0 )
    {
        return activeTab->CanClose();
    }
    return true;
}

const QString ToolTabsContainerWidget::ActiveToolCannotCloseReason() const
{
    const ToolPtr activeTab = FindActiveTool();
    if ( activeTab != 0 )
    {
        return activeTab->CannotCloseReason();
    }
    return QString();
}
