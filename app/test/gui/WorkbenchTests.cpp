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

#include "Workbench.h"
#include "TestHelpers.h"
#include "Tool.h"
#include "ToolTabsContainerWidget.h"
#include "WbConfig.h"
#include "WbSchema.h"

namespace
{
    class DummyToolContainer : public ToolContainer
    {
    public:
        virtual void AddTool( ToolInterface* const tool )
        {
            (void) tool;
        }

        virtual bool TryToOpenTool( const WbConfig& config )
        {
            (void) config;
            return false;
        }

    };

    class DummyTool : public ToolInterface
    {
    public:
        virtual const QString Name() const { return QObject::tr( "" ); }
        virtual QWidget* Widget() { return 0; }
        virtual bool TryToOpenTool( const WbConfig& config )
        {
            (void) config;
            return false;
        }

    };
}

TEST(WorkbenchTests, OpenWorkbenchFile)
{
//    WbSchema parentSchema( KeyName( "parentSchema" ) );
//    parentSchema.AddSingleValueKey( "name" );
//
//    WbSchema child1Schema( KeyName( "child1Schema" ) );
//    child1Schema.AddSingleValueKey( "name" );
//
//    WbSchema child1ChildSchema( KeyName( "child1ChildSchema" ) );
//    child1ChildSchema.AddSingleValueKey( "name" );
//
//    child1Schema.AddSubSchema( child1ChildSchema.GetName(), child1ChildSchema );
//    parentSchema.AddSubSchema( child1Schema.GetName(), child1Schema );
//
//    WbSchema child2Schema( KeyName( "child2Schema" ) );
//    child2Schema.AddSingleValueKey( "name" );
//
//    parentSchema.AddSubSchema( child2Schema.GetName(), child2Schema );
//
//    WbConfig parentConfig( parentSchema );
//

//    child1Config->SetDisplayName( "Child1" );
//    child1Config->SetCheckParsing();
//    parentConfig.AddChildConfigFile( child1Config );
//
//    DummyConfigFile* const child2Config = new DummyConfigFile;
//    child2Config->SetDisplayName( "Child2" );
//    child2Config->SetCheckParsing();
//    parentConfig.AddChildConfigFile( child2Config );
//
//    DummyConfigFile* const child1ChildConfig = new DummyConfigFile;
//    child1ChildConfig->SetDisplayName( "Child Child" );
//    child1ChildConfig->SetCheckParsing();
//    child1Config->AddChildConfigFile( child1ChildConfig );
//
//    QTreeWidget treeWidget( 0 );
//
//    DummyToolContainer toolContainer;
//    Workbench workbench( treeWidget, toolContainer );
//
//    workbench.Open( parentConfig );
//
//    EXPECT_EQ( 1 , treeWidget.topLevelItemCount() ) <<  "There is one top level widget in the tree";
//
//    QTreeWidgetItem* const topLevelWidget = treeWidget.topLevelItem( 0 );
//
//    ASSERT(topLevelWidget ) << "The top level widget is not null";
//
//    EXPECT_EQ( parentConfig.GetDisplayName(false) , topLevelWidget->text(0) ) <<  "The top level widget is the parent one";
//
//    EXPECT_EQ( 2 ,  topLevelWidget->childCount() ) <<  "The top level widget has two children";
//
//    QTreeWidgetItem* const firstChildWidget  = topLevelWidget->child( 0 );
//    QTreeWidgetItem* const secondChildWidget = topLevelWidget->child( 1 );
//
//    ASSERT(firstChildWidget ) << "The first child widget is not null";
//    ASSERT(secondChildWidget ) << "The second child widget is not null";
//
//    EXPECT_EQ(  child1Config->GetDisplayName(false) ,  firstChildWidget->text(0) ) <<  "The first child is Child1";
//    EXPECT_EQ( child2Config->GetDisplayName(false) , secondChildWidget->text(0) ) <<  "The second child is Child2";
//
//    EXPECT_EQ( 1 ,    firstChildWidget->childCount() ) <<  "The first child widget has one child";
//    EXPECT_EQ( 0 , secondChildWidget->childCount() ) <<  "The second child widget has no children";
//
//    QTreeWidgetItem* const firstChildChildWidget  = firstChildWidget->child( 0 );
//
//    ASSERT(firstChildChildWidget ) << "The first child child widget is not null";
//
//    EXPECT_EQ( child1ChildConfig->GetDisplayName(false) , firstChildChildWidget->text(0)) << "The first child's child is is Child1's child";

}

