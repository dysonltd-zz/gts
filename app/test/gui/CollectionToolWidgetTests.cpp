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
#include "TestHelpers.h"
#include "ToolTabsContainerWidget.h"
#include "WbDefaultKeys.h"
#include "CollectionToolWidget.h"
#include "TestCollectionToolWidget.h"
#include "TestTool.h"

TEST(CollectionToolWidgetTests, AddingSubSchemaToCollection)
{
    WbSchema testTool1Schema( KeyName( "testTool1Schema" ) );
    WbSchema testTool2Schema( KeyName( "testTool2Schema" ) );

    WbSchema testCollectionSchema( KeyName( "testCollectionSchema" ) );
    WbSchema testElementSchema   ( KeyName( "testElementSchema" ) );

    TestCollectionToolWidget toolWithSubTools( testCollectionSchema,
                                               testElementSchema );
    toolWithSubTools.TestAddSubTool( new TestTool( testTool1Schema ) );
    toolWithSubTools.TestAddSubTool( new TestTool( testTool2Schema ) );

    WbSchema testWorkbenchSchema( KeyName( "testWbSchema" ) );

    toolWithSubTools.AddFullWorkbenchSchemaSubTreeTo( testWorkbenchSchema,
                                                      testWorkbenchSchema.Name() );

    EXPECT_EQ(2 , testWorkbenchSchema.FindSubSchema(testCollectionSchema.Name())
                                     .FindSubSchema(testElementSchema.Name()).GetNumSubSchemas())
            << "We added the two sub-schema for the sub-tools to the element schema";
}

