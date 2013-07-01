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
#include "WbKeyValues.h"
#include "TestHelpers.h"
#include <Container.h>

TEST(WbKeyValueTests, SetAndRetrieveNoId)
{
    const KeyName  testKeyName          ( "testKeyName" );
    const KeyValue testKeyValue         ( KeyValue::from( "testKeyValue" ) );
    const KeyValue testDifferentKeyValue( KeyValue::from( "testDifferentKeyValue" ) );

    WbKeyValues keyValues;
    keyValues.SetKeyValue( testKeyName, testKeyValue );
    EXPECT_EQ(testKeyValue , keyValues.GetKeyValue( testKeyName )) << "Setting and retrieving a key value for a key with no id works";

    keyValues.SetKeyValue( testKeyName, testDifferentKeyValue );
    EXPECT_EQ(testDifferentKeyValue , keyValues.GetKeyValue( testKeyName )) << "Setting a different key value for the same key with no id overwrites the previous value";
}

TEST(WbKeyValueTests, SetAndRetrieveDifferentKeysNoId)
{
    const KeyName  testKeyName          ( "testKeyName" );
    const KeyName  testDifferentKeyName ( "testDifferentKeyName" );
    const KeyValue testKeyValue         ( KeyValue::from( "testKeyValue" ) );
    const KeyValue testDifferentKeyValue( KeyValue::from( "testDifferentKeyValue" ) );

    WbKeyValues keyValues;
    keyValues.SetKeyValue( testKeyName,          testKeyValue );
    keyValues.SetKeyValue( testDifferentKeyName, testDifferentKeyValue );
    EXPECT_EQ(testKeyValue , keyValues.GetKeyValue( testKeyName )) << "Setting and retrieving a key value for a second key with no id does not overwrite the first keys's value";
    EXPECT_EQ(testDifferentKeyValue , keyValues.GetKeyValue( testDifferentKeyName )) << "Setting and retrieving a key value for a second key with no id returns correc tvalue for second key";
}

TEST(WbKeyValueTests, SetAndRetrieveWithId)
{
    const KeyName  testKeyName          ( "testKeyName" );
    const KeyId    testKeyId            ( "testKeyId" );
    const KeyId    testDifferentKeyId   ( "testDifferentKeyId" );
    const KeyValue testKeyValue         ( KeyValue::from( "testKeyValue" ) );
    const KeyValue testDifferentKeyValue( KeyValue::from( "testDifferentKeyValue" ) );

    WbKeyValues keyValues;
    keyValues.SetKeyValue( testKeyName, testKeyValue, testKeyId );
    EXPECT_EQ(testKeyValue , keyValues.GetKeyValue( testKeyName, testKeyId )) << "Setting and retrieving a key value for a key with an id works";

    keyValues.SetKeyValue( testKeyName, testDifferentKeyValue, testDifferentKeyId );
    EXPECT_EQ(testKeyValue , keyValues.GetKeyValue( testKeyName, testKeyId )) << "Setting a different key value for the same key with a different id does not overwrites the previous value";
    EXPECT_EQ(testDifferentKeyValue , keyValues.GetKeyValue( testKeyName, testDifferentKeyId )) << "Setting a different key value for the same key with a different id returns correct value if the different id is requested";

    keyValues.SetKeyValue( testKeyName, testDifferentKeyValue, testKeyId );
    EXPECT_EQ(testDifferentKeyValue , keyValues.GetKeyValue( testKeyName, testKeyId )) << "Setting the different key value for the key with the original id overwrites the original value";


}

TEST(WbKeyValueTests, SetRetrieveAndRemoveWithId)
{
    const KeyName  testKeyName          ( "testKeyName" );
    const KeyId    testKeyId            ( "testKeyId" );
    const KeyId    testDifferentKeyId   ( "testDifferentKeyId" );
    const KeyValue testKeyValue         ( KeyValue::from( "testKeyValue" ) );
    const KeyValue testDifferentKeyValue( KeyValue::from( "testDifferentKeyValue" ) );

    WbKeyValues keyValues;
    keyValues.SetKeyValue( testKeyName, testKeyValue, testKeyId );
    keyValues.SetKeyValue( testKeyName, testDifferentKeyValue, testDifferentKeyId );

    keyValues.RemoveKeyValue( testKeyName, testKeyId );

    EXPECT_TRUE(keyValues.GetKeyValue( testKeyName, testKeyId ).IsNull() ) << "The key we asked to remove is gone";
    EXPECT_EQ(testDifferentKeyValue , keyValues.GetKeyValue( testKeyName, testDifferentKeyId )) << "The key we did not ask to remove is still there";
}

TEST(WbKeyValueTests, RetrieveKeyValuesForAllIds)
{
    const KeyName  testKeyName          ( "testKeyName" );
    const KeyId    testKeyId            ( "testKeyId" );
    const KeyId    testDifferentKeyId   ( "testDifferentKeyId" );
    const KeyValue testKeyValue         ( KeyValue::from( "testKeyValue" ) );
    const KeyValue testDifferentKeyValue( KeyValue::from( "testDifferentKeyValue" ) );

    WbKeyValues::ValueIdPair testFirstPair;
    testFirstPair.id    = testKeyId;
    testFirstPair.value = testKeyValue;

    WbKeyValues::ValueIdPair testSecondPair;
    testSecondPair.id    = testKeyId;
    testSecondPair.value = testKeyValue;

    WbKeyValues keyValues;
    keyValues.SetKeyValue( testKeyName, testKeyValue, testKeyId );
    keyValues.SetKeyValue( testKeyName, testDifferentKeyValue, testDifferentKeyId );

    const WbKeyValues::ValueIdPairList valueIdPairList = keyValues.GetKeyValues( testKeyName );

    EXPECT_TRUE(container( valueIdPairList ).contains( testFirstPair ) ) << "The Value-Id Pair List for a key contains the first set value";
    EXPECT_TRUE(container( valueIdPairList ).contains( testSecondPair ) ) << "The Value-Id Pair List for a key contains the first set value";
}

