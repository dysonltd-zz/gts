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
#include "TestHelpers.h"
#include "MockQWidget.h"
#include "ToolTabsContainerWidget.h"
#include "WbDefaultKeys.h"
#include "TestTool.h"

TEST(ToolTests, AddSubSchemaWithNoFullSubSchemaOverride)
{
    WbSchema testToolSchema( KeyName( "testToolSchema" ) );
    TestTool defaultTool( testToolSchema );

    WbSchema testSchema( KeyName( "testSchema" ) );

    defaultTool.AddFullWorkbenchSchemaSubTreeTo( testSchema, testSchema.Name() );

    EXPECT_EQ( 1 , testSchema.GetNumSubSchemas() ) <<  "By default a tool will add its schema as a subschema";
    EXPECT_EQ(testToolSchema.Name(), testSchema.FindSubSchema( testToolSchema.Name() ).Name()) <<
                "The subschema will be added under a key matching its schema name";
}

TEST(ToolTests, AddSubSchemaWithDefaultFileNameToSpecifiedSchema)
{
    WbSchema testToolSchema( KeyName( "testToolSchema" ) );
    TestToolWithDefaultSubSchemaFileName testTool( testToolSchema );

    testTool.m_defaultSubSchemaFileName = "test/file/default.xml";

    WbSchema testSchema( KeyName( "testSchema" ) );

    testTool.AddFullWorkbenchSchemaSubTreeTo( testSchema, testSchema.Name() );

    WbConfig defaultedConfigFromSchema( testSchema, QFileInfo() );

    EXPECT_EQ(testTool.m_defaultSubSchemaFileName,
              defaultedConfigFromSchema.GetSubConfig(testToolSchema.Name()).GetPossiblyRelativeFileInfo().filePath())
              << "The sub-schema will be added under a key matching its schema name with the default fileName";
}

TEST(ToolTests, AddSubSchemaWithFullSubSchemaOverride)
{
    WbSchema testToolSchema( KeyName( "testToolSchema" ) );
        TestToolImplementingFullWorkbenchSchemaSubTree testToolWithFullSchemaSubTree( testToolSchema );
    const KeyName testFullSchemaSubTreeName( "testFullSchemaSubTreeName" );
    testToolWithFullSchemaSubTree.m_fullWorkBenchSchemaSubTree = WbSchema( testFullSchemaSubTreeName );

    WbSchema testSchema( KeyName( "testSchema" ) );

    testToolWithFullSchemaSubTree.AddFullWorkbenchSchemaSubTreeTo( testSchema, testSchema.Name() );

    EXPECT_EQ(1, testSchema.GetNumSubSchemas())
            << "A tool will add a subschema returned by GetFullWorkbenchSchemaSubTree in the derived class";

    EXPECT_EQ(testFullSchemaSubTreeName, testSchema.FindSubSchema( testFullSchemaSubTreeName ).Name())
            << "The subschema will be added under a key matching its schema name";
}

TEST(ToolTests, CreateWorkbenchSubSchema)
{
    TestTool testTool;

    const QString testDefaultName( "testDefaultName" );

    const KeyName testSchemaName( "testSchemaName" );

    const WbSchema testSchema =
        testTool.TestCreateWorkbenchSubSchema( testSchemaName, testDefaultName );

    EXPECT_EQ(testSchemaName, testSchema.Name()) << "The sub-schema has the correct schema name";
    EXPECT_TRUE(testSchema.ContainsKey(WbDefaultKeys::displayNameKey))
            << "The sub-schema created as a workbench sub-schema will have a key added for its display name ";

    WbConfig testConfig( testSchema, QFileInfo() );
    testSchema.SetDefaultsTo( testConfig );

    EXPECT_EQ(KeyValue::from(testDefaultName), testConfig.GetKeyValue(WbDefaultKeys::displayNameKey))
            << "The default value for the name is the value supplied to the create function";
}

TEST(ToolTests, TryToOpen)
{
    const KeyName testFirstSchemaName( "testFirstSchemaName" );
    const KeyName testSecondSchemaName( "testSecondSchemaName" );
    const KeyName testSchemaNameNotInTool( "testSchemaNameNotInTool" );

    WbSchema firstSchema( testFirstSchemaName );
    const WbSchema secondSchema( testSecondSchemaName );

    firstSchema.AddSubSchema( secondSchema, WbSchemaElement::Multiplicity::One );

    const WbSchema schemaNotInTool( testSchemaNameNotInTool );

    TestTool testTool( firstSchema );

    const WbConfig firstConfig( firstSchema, QFileInfo() );

    EXPECT_TRUE(testTool.TryToOpenTool(firstConfig))
            << "By default we return true from try to open if any of the schemas contained in the tool's schema "
               "match the schema in the config";
    EXPECT_TRUE(testTool.TryToOpenTool(WbConfig(secondSchema, QFileInfo())))
            << "By default we return true from try to open if any of the schemas created with "
               "CreateWorkbenchSubSchema match the schema in the config";
    EXPECT_FALSE(testTool.TryToOpenTool(WbConfig(schemaNotInTool, QFileInfo())))
            << "By default we return false from try to open if none of the schemas created with "
               "CreateWorkbenchSubSchema match the schema in the config";
}

