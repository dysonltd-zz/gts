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

#include <vector>
#include <string>
#include <iostream>

#include <QtGui/qapplication.h>
#include <QtCore/qdir.h>

#include <gtest/gtest.h>
#include <Container.h>

#include "XmlConfigFileReader.h"
#include "WbSchema.h"
#include "WbKeyValues.h"
#include "WbConfigFileWriter.h"
#include "WbConfigFileReader.h"
#include "WbConfig.h"
#include "TestHelpers.h"

namespace
{
    struct TestSetup
    {
        TestSetup()
            :
            parentSchemaName    ( "parentSchemaName" ),
            childSchemaName     ( "childSchemaName" ),
            grandChildSchemaName( "grandChildSchemaName" ),
            parentSchema    ( parentSchemaName ),
            childSchema     ( childSchemaName ),
            grandChildSchema( grandChildSchemaName )
        {
            childSchema.AddSubSchema( grandChildSchema, WbSchemaElement::Multiplicity::One );
            parentSchema.AddSubSchema( childSchema, WbSchemaElement::Multiplicity::One );
        }

        KeyName parentSchemaName;
        KeyName childSchemaName;
        KeyName grandChildSchemaName;
        WbSchema parentSchema;
        WbSchema childSchema;
        WbSchema grandChildSchema;
    };

    const WbSchema newSchema( KeyName( "newSchema" ) );

}

TEST(WbSchemaTests, AddSubSchemaToSchemaWhenSchemaIsTopLevel)
{
    TestSetup test;

    const bool addToTopLevelSchemaResult =
        test.parentSchema.AddSubSchemaToSchema( newSchema,
                                                test.parentSchemaName,
                                                WbSchemaElement::Multiplicity::One );

    EXPECT_TRUE(addToTopLevelSchemaResult) << "The addition of the sub-schema was successful";

    EXPECT_EQ( 2 , test.parentSchema.GetNumSubSchemas() ) <<  "We added a sub-schema to the top-level one";
    EXPECT_EQ(newSchema.Name(), test.parentSchema.FindSubSchema( newSchema.Name() ).Name()) << "The added schema has the correct name";
}

TEST(WbSchemaTests, AddSubSchemaToNonExistentSchema)
{
    TestSetup test;

    const bool addToNonExistentSchemaResult =
        test.parentSchema.AddSubSchemaToSchema( newSchema,
                                                KeyName( "NonExistentSchemaName" ),
                                                WbSchemaElement::Multiplicity::One );

    EXPECT_FALSE(addToNonExistentSchemaResult) << "Adding to a non-existent schema fails";
    EXPECT_EQ( 1 , test.parentSchema.GetNumSubSchemas() ) <<  "No sub-schema added to the top-level one";

}

TEST(WbSchemaTests, AddSubSchemaToSchemaWhenSchemaIsDirectDescendant)
{
    TestSetup test;

    const bool result =
        test.parentSchema.AddSubSchemaToSchema( newSchema,
                                                test.childSchemaName,
                                                WbSchemaElement::Multiplicity::One );

    EXPECT_TRUE(result) << "The addition of the sub-schema was successful";

    const WbSchema newChildSchema( test.parentSchema.FindSubSchema( test.childSchemaName ) );
    EXPECT_EQ( 2 , newChildSchema.GetNumSubSchemas() ) <<  "We added a sub-schema to the first child level";
    EXPECT_EQ(newSchema.Name(), newChildSchema.FindSubSchema( newSchema.Name() ).Name()) << "The added schema has the correct name";
}

TEST(WbSchemaTests, AddSubSchemaToSchemaWhenSchemaIsInDirectDescendant)
{
    TestSetup test;

    const bool result =
        test.parentSchema.AddSubSchemaToSchema( newSchema,
                                                test.grandChildSchemaName,
                                                WbSchemaElement::Multiplicity::One );

    EXPECT_TRUE(result) << "The addition of the sub-schema was successful";

    const WbSchema newGrandChildSchema( test
                                        .parentSchema
                                        .FindSubSchema( test.childSchemaName )
                                        .FindSubSchema( test.grandChildSchemaName ) );
    EXPECT_EQ(1, newGrandChildSchema.GetNumSubSchemas()) << "We added a sub-schema to the first child level";
    EXPECT_EQ(newSchema.Name(), newGrandChildSchema.FindSubSchema( newSchema.Name() ).Name()) << "The added schema has the correct name";
}

TEST(WbSchemaTests, SchemaAddToSubSchemaBug)
{
    TestSetup test;
    test.parentSchema.AddSubSchemaToSchema( newSchema,
                                            test.childSchema.Name(),
                                            WbSchemaElement::Multiplicity::One );

    EXPECT_TRUE(test.parentSchema.FindSubSchema( test.childSchemaName ) .ContainsSubSchema( newSchema.Name() )) << "Can find a schema added to the child schema AFTER child was added to parent";
}
