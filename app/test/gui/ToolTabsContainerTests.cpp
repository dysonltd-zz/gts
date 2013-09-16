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


#include <gtest/gtest.h>
#include "Tool.h"
#include "HelpViewer.h"
#include "TestHelpers.h"
#include "MockQWidget.h"
#include "ToolTabsContainerWidget.h"

namespace
{
    class MockTool : public ToolInterface
    {
    public:
        MockTool()
        :
            m_openThisTool( false ),
            m_tryToOpenToolCalled( false ),
            m_mockQWidget()
        {}

        virtual const QString Name() const { return QObject::tr( "Name" ); }
        virtual QWidget* Widget() { return &m_mockQWidget; }

        virtual void Reload( const WbConfig& ) {}

        virtual bool TryToOpenTool( const WbConfig& config )
        {
            (void) config;
            m_tryToOpenToolCalled = true;
            return m_openThisTool;
        }

        virtual bool CanClose() const
        {
            return true;
        }

        virtual void Activated()
        {

        }

        virtual const QString CannotCloseReason() const
        {
            return QString();
        }

        virtual void CallOnSelfAndActiveSubTools( ToolFunction& func )
        {
            (void) func;
        }

        virtual void UpdateToolMenu( QMenu& toolMenu )
        {
            (void) toolMenu;
        }

        virtual void AddFullWorkbenchSchemaSubTreeTo(WbSchema&, const KeyName&) const
        {
        }

        virtual const WbConfig GetCurrentConfig() const
        {
            return WbConfig();
        }

        virtual const WbSchema GetMostSpecificSubSchema() const
        {
            return WbSchema();
        }

        bool m_openThisTool;
        bool m_tryToOpenToolCalled;

        WbSchema m_schema;
        mutable WbSchema m_parentSchemaToAddTo;

    private:
        MockQWidget m_mockQWidget;
    };

}


TEST(ToolTabsContainerTests, TryToOpen)
{
    ToolTabsContainerWidget toolTabsContainer( 0 );
    MockTool* testTool1 = new MockTool;
    MockTool* testTool2 = new MockTool;

    toolTabsContainer.AddTool( testTool1 );
    toolTabsContainer.AddTool( testTool2 );

    WbSchema testSchema( KeyName( "testSchema" ) );
    WbConfig testConfig( testSchema, QFileInfo() );

    EXPECT_FALSE(toolTabsContainer.TryToOpenTool( testConfig ) ) << "If none of the tools handle this schema then the TryToOpen reports failure";

    EXPECT_TRUE(testTool1->m_tryToOpenToolCalled && testTool2->m_tryToOpenToolCalled)
            << "Both of the tools have their TryToOpen functions called (we try them all as possibilities)";

    testTool1->m_tryToOpenToolCalled = false;
    testTool1->m_openThisTool = true;
    testTool2->m_tryToOpenToolCalled = false;

    EXPECT_TRUE(toolTabsContainer.TryToOpenTool( testConfig ) ) << "If one of the tools handle this schema then the TryToOpen reports success";

    EXPECT_TRUE(testTool1->m_tryToOpenToolCalled ) << "The tool that handles the schema has its TryToOpen function called";

    EXPECT_TRUE(testTool2->m_tryToOpenToolCalled ) << "The other tool also has its TryToOpen function called";
}

