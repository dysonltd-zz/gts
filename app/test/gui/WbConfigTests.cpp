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
#include "WbConfig.h"
#include "WbSchema.h"
#include "TestHelpers.h"

namespace
{
    struct TestSetup
    {
        TestSetup()
        {
            parentSchemaMultivaluedKey = KeyName( "multivaluedKey" );

            parentSchema          = WbSchema( KeyName( "parentSchema" ) );
            childSchema           = WbSchema( KeyName( "childSchema" ) );
            siblingChildSchema    = WbSchema( KeyName( "siblingChildSchema" ) );
            grandChildSchema      = WbSchema( KeyName( "gcSchema" ) );
            greatGrandChildSchema = WbSchema( KeyName( "gtgcSchema" ) );

            grandChildSchema.AddSubSchema ( greatGrandChildSchema,
                                            WbSchemaElement::Multiplicity::One );
            childSchema.AddSubSchema      ( grandChildSchema,
                                            WbSchemaElement::Multiplicity::One );
            parentSchema.AddSubSchema     ( childSchema,
                                            WbSchemaElement::Multiplicity::One );
            parentSchema.AddSubSchema     ( childSchema,
                                            WbSchemaElement::Multiplicity::One );
            parentSchema.AddSingleValueKey( parentSchemaMultivaluedKey,
                                            WbSchemaElement::Multiplicity::Many );

            parentConfig          = WbConfig( parentSchema, QFileInfo() );
            childConfig           = WbConfig( parentConfig
                                              .CreateSubConfig( childSchema.Name(),
                                                                "anyFile" ) );
            siblingChildConfig    = WbConfig( parentConfig
                                              .CreateSubConfig( childSchema.Name(),
                                                                "anyFile" ) );
            grandChildConfig      = WbConfig( childConfig
                                              .CreateSubConfig( grandChildSchema.Name(),
                                                                "anyFile" ) );
            greatGrandChildConfig = WbConfig( childConfig
                                              .CreateSubConfig( grandChildSchema.Name(),
                                                                "anyFile" ) );
        }

        KeyName parentSchemaMultivaluedKey;

        WbSchema parentSchema;
        WbSchema childSchema;
        WbSchema siblingChildSchema;
        WbSchema grandChildSchema;
        WbSchema greatGrandChildSchema;

        WbConfig parentConfig;
        WbConfig childConfig;
        WbConfig siblingChildConfig;
        WbConfig grandChildConfig;
        WbConfig greatGrandChildConfig;
    };
}

TEST(WbConfigTests, SharedData)
{
    WbSchema schema( KeyName( "testSchema" ) );
    WbConfig config( schema, QFileInfo() );

    const KeyName  testKeyName ( "keyName" );
    const KeyValue testKeyValue( KeyValue::from( "keyValue" ) );

    config.SetKeyValue( testKeyName, testKeyValue );

    EXPECT_EQ( testKeyValue , config.GetKeyValue( testKeyName ) ) <<  "Retrieving a value we just set works";

    WbConfig copiedConfig( config );

    EXPECT_EQ( testKeyValue , copiedConfig.GetKeyValue( testKeyName ) ) <<  "Retrieving a value from a copy of the config works";

    const KeyValue testDifferentKeyValue( KeyValue::from( "testDifferentKeyValue" ) );
    config.SetKeyValue( testKeyName, testDifferentKeyValue );

    EXPECT_EQ(testDifferentKeyValue , copiedConfig.GetKeyValue( testKeyName )) << "After changing the value in the original config, the change is reflected in the copied config";

}

TEST(WbConfigTests, DescendantFunction)
{
    TestSetup test;

    EXPECT_TRUE(!WbConfig().SchemaIsDescendantOf( test.parentConfig.GetSchemaName() ) ) << "Null Config is not a descendant of any schema";
    EXPECT_TRUE(test.childConfig.SchemaIsDescendantOf( test.parentConfig.GetSchemaName() ) ) << "Direct child is a descendant of a schema";

    EXPECT_TRUE(test.grandChildConfig.SchemaIsDescendantOf( test.parentConfig.GetSchemaName() ) ) << "Grandchild config is a descendant of the schema";
    EXPECT_TRUE(test.parentConfig.SchemaIsDescendantOf( test.parentConfig.GetSchemaName() ) ) << "Config is a descendant of its own schema";
}

TEST(WbConfigTests, CreateSubConfig)
{
    TestSetup test;
    const KeyId subConfigId( "someId" );

    const KeyName subConfigKeyName( test.childSchema.Name() );
    const size_t numSubConfigsBeforeCreatingThisOne = test.parentConfig.GetSubConfigs( subConfigKeyName ).size();
    WbConfig aSubConfig( test.parentConfig.CreateSubConfig( subConfigKeyName, "anyFile", subConfigId ) );

    EXPECT_EQ(numSubConfigsBeforeCreatingThisOne + 1 , test.parentConfig.GetSubConfigs( subConfigKeyName ).size()) << "New SubConfig is added to the list of sub configs";
    EXPECT_TRUE(test.parentConfig.GetSubConfig( subConfigKeyName, subConfigId ).IsTheSameAs( aSubConfig ) ) << "We Can get the sub-config back";
    EXPECT_EQ( test.childSchema.Name() , aSubConfig.GetSchemaName() ) <<  "SubConfig is created with correct schema";
}

TEST(WbConfigTests, AddKeyValue)
{
    TestSetup test;
    const KeyValue testValue ( KeyValue::from( "testValue" ) );
    const KeyValue testValue2( KeyValue::from( "testValue2" ) );
    const KeyId firstAddedId =
        test.parentConfig.AddKeyValue( test.parentSchemaMultivaluedKey, testValue );
    const KeyId secondAddedId =
        test.parentConfig.AddKeyValue( test.parentSchemaMultivaluedKey, testValue2 );

    EXPECT_FALSE(firstAddedId.isEmpty() ) << "The returned KeyIds from AddKeyValue are valid";

    const KeyValue addedValue(
        test.parentConfig.GetKeyValue( test.parentSchemaMultivaluedKey, firstAddedId ) );
    EXPECT_EQ(testValue , addedValue) << "The value is added to the config under the returned KeyId and the specified KeyName";

    EXPECT_NE(secondAddedId , firstAddedId) << "The returned KeyIds from multiple calls to AddKeyValue are different";
}

TEST(WbConfigTests, RemoveOldKeyValue)
{
    TestSetup test;
    const KeyValue testValue ( KeyValue::from( "testValue" ) );
    const KeyValue testValue2( KeyValue::from( "testValue2" ) );
    const KeyId firstAddedId =
        test.parentConfig.AddKeyValue( test.parentSchemaMultivaluedKey, testValue );
    const KeyId secondAddedId =
        test.parentConfig.AddKeyValue( test.parentSchemaMultivaluedKey, testValue2 );
    const KeyId thirdAddedId =
        test.parentConfig.AddKeyValue( test.parentSchemaMultivaluedKey, testValue2 );
    const KeyId fourthAddedId =
        test.parentConfig.AddKeyValue( test.parentSchemaMultivaluedKey, testValue2 );

    std::vector< KeyId > idsToKeep;
    idsToKeep.push_back( secondAddedId );
    idsToKeep.push_back( fourthAddedId );

    test.parentConfig.RemoveOldKeys( test.parentSchemaMultivaluedKey, idsToKeep );

    EXPECT_FALSE(test.parentConfig.GetKeyValue( test.parentSchemaMultivaluedKey, idsToKeep.at( 0 ) ).IsNull() ) << "The first key we required to keep is still present";
    EXPECT_FALSE(test.parentConfig.GetKeyValue( test.parentSchemaMultivaluedKey, idsToKeep.at( 1 ) ).IsNull() ) << "The second key we required to keep is still present";

    EXPECT_TRUE(test.parentConfig.GetKeyValue( test.parentSchemaMultivaluedKey, firstAddedId ).IsNull() ) << "The first unspecified key is removed";
    EXPECT_TRUE(test.parentConfig.GetKeyValue( test.parentSchemaMultivaluedKey, thirdAddedId ).IsNull() ) << "The second unspecified key is removed";
}

