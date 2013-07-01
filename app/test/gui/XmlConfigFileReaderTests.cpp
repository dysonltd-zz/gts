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
#include <QtCore/qiodevice.h>
#include <QtCore/qbuffer.h>
#include "XmlConfigFileReader.h"
#include <Container.h>
#include "MockIoDevice.h"

#include "TestHelpers.h"
#include "WbKeyValues.h"

using namespace XmlTools;

namespace
{
    const QString XmlOpenTag( const QString tagName )
    {
        return QString( "<" ).append( tagName ).append( ">" );
    }
    const QString XmlOpenTagAttr( const QString tagName, const QString idAttr )
    {
        return QString( "<" ).append( tagName ).append( " id = \"" ).append( idAttr ).append( "\" >" );
    }
    const QString XmlCloseTag( const QString tagName )
    {
        return XmlOpenTag( QString( "/" ).append( tagName ) );
    }

    const KeyName schemaName                        ( "testSchema" );
    const KeyName testKeyNameNoId                   ( "testKeyNameNoId" );
    const KeyName testKeyNameWithId                 ( "testKeyNameWithId" );
    const QString testKeyNoIdValueString            ( "testKeyNoIdValue" );
    const QString testKeyWithIdValue1String         ( "testKeyWithIdValue1" );
    const QString testKeyWithIdValue2String         ( "testKeyWithIdValue2" );
    const KeyId testKeyWithIdId1                    ( "testKeyWithIdId1" );
    const KeyId testKeyWithIdId2                    ( "testKeyWithIdId2" );

    const KeyName testMultiValuedKeyName            ( "testMatrixKey" );
    const QString testMultiValuedKeyValueString1    ( "A" );
    const QString testMultiValuedKeyValueString2    ( "B" );
    const QString testMultiValuedKeyValueString3    ( "C" );

    const KeyName testGroupWithIdName               ( "testGroupWithId" );
    const KeyName testSubKeyName                    ( "testSubKeyName" );
    const QString testSubKeyValueString1            ( "testSubKeyValueString1" );
    const QString testSubKeyValueString2            ( "testSubKeyValueString2" );
    const KeyId   testGroupId1                      ( "testSubKeyId1" );
    const KeyId   testGroupId2                      ( "testSubKeyId2" );

    const KeyName testMultiValuedSubKeyName         ( "testMatrixSubKey" );
    const QString testMultiValuedSubKeyValueString1 ( "X" );
    const QString testMultiValuedSubKeyValueString2 ( "Z" );

}

TEST(XmlConfigFileReaderTests, IoDeviceInteraction)
{
    MockIoDevice mockIoDevice;

    XmlConfigFileReader reader;

    mockIoDevice.m_openSuccessful = false;

    EXPECT_FALSE(reader.ReadFrom( mockIoDevice ) ) << "ReadFrom returns false if the open fails";

    EXPECT_EQ( QIODevice::ReadOnly , mockIoDevice.m_openModeRequested ) <<  "The reader requested read only access to the IoDevice";
}

TEST(XmlConfigFileReaderTests, InvalidXml)
{
    QString invalidXml( "InvalidXml <>" );

    QByteArray invalidXmlByteArray( invalidXml.toAscii() );
    QBuffer buffer( &invalidXmlByteArray );

    XmlConfigFileReader reader;

    EXPECT_FALSE(reader.ReadFrom( buffer ) ) << "ReadFrom returns false if Xml is not valid";

}

TEST(XmlConfigFileReaderTests, ReadingXmlSpecialCharacters)
{
    QString xmlConfigFile;
    xmlConfigFile.append( XmlOpenTag( schemaName.ToQString() ) );

    xmlConfigFile.append( XmlOpenTag( testSubKeyName.ToQString() ) );

    const QString textWithEncodedXmlCharacters( "This is an ampersand &amp;" );
    const QString textWithDecodedXmlCharacters( "This is an ampersand &" );
    xmlConfigFile.append( textWithEncodedXmlCharacters );

    xmlConfigFile.append( XmlCloseTag( testSubKeyName.ToQString() ) );

    xmlConfigFile.append( XmlCloseTag( schemaName.ToQString() ) );

    QByteArray invalidXmlByteArray( xmlConfigFile.toAscii() );
    QBuffer buffer( &invalidXmlByteArray );
    XmlConfigFileReader reader;

    reader.ReadFrom( buffer );

    WbKeyValues values;
    reader.ReadKeyValues( testSubKeyName, values );

    EXPECT_EQ(textWithDecodedXmlCharacters , values.GetKeyValue( testSubKeyName ).ToQString()) << "The XML special characters are correctly decoded";
}

TEST(XmlConfigFileReaderTests, ValidEmptyXmlConfig)
{
    QString validEmptyConfigXml;
    validEmptyConfigXml.append( XmlOpenTag( schemaName.ToQString() ) );
    validEmptyConfigXml.append( XmlCloseTag( schemaName.ToQString() ) );

    QByteArray validXmlByteArray( validEmptyConfigXml.toAscii() );
    QBuffer buffer( &validXmlByteArray );

    XmlConfigFileReader reader;

    EXPECT_TRUE(reader.ReadFrom( buffer ) ) << "ReadFrom returns true if Xml is valid";

    EXPECT_EQ( schemaName , reader.GetSchemaName() ) <<  "The correct schema name is returned";

    WbKeyValues keyValues;
    reader.ReadKeyValues( KeyName( "anyKey" ), keyValues );
    EXPECT_TRUE(keyValues.GetKeyValues( KeyName( "anyKey" ) ).empty() ) << "Since the key does not exist, reading a key value does not put anything in the WbKeyValues structure";

    reader.ReadKeyValuesFromGroup( KeyName( "anyGroupKey" ), KeyName( "anyGroup" ), keyValues );
    EXPECT_TRUE(keyValues.GetKeyValues( KeyName( "anyGroupKey" ) ).empty() ) << "Since the key does not exist, reading a key value from a group does not put anything in the WbKeyValues structure";

}

TEST(XmlConfigFileReaderTests, ValidPopulatedXmlConfig)
{
    QString validPopulatedConfigXml;
    validPopulatedConfigXml.append( XmlOpenTag( schemaName.ToQString() ) );

    validPopulatedConfigXml.append( XmlOpenTag( testKeyNameNoId.ToQString() ) );
    validPopulatedConfigXml.append( testKeyNoIdValueString );
    validPopulatedConfigXml.append( XmlCloseTag( testKeyNameNoId.ToQString() ) );

    validPopulatedConfigXml.append( XmlOpenTagAttr( testKeyNameWithId.ToQString(), testKeyWithIdId1 ) );
    validPopulatedConfigXml.append( testKeyWithIdValue1String );
    validPopulatedConfigXml.append( XmlCloseTag( testKeyNameWithId.ToQString() ) );

    validPopulatedConfigXml.append( XmlOpenTagAttr( testKeyNameWithId.ToQString(), testKeyWithIdId2 ) );
    validPopulatedConfigXml.append( testKeyWithIdValue2String );
    validPopulatedConfigXml.append( XmlCloseTag( testKeyNameWithId.ToQString() ) );

    validPopulatedConfigXml.append( XmlOpenTag( testMultiValuedKeyName.ToQString() ) );
    {
        validPopulatedConfigXml.append( XmlOpenTag( "_0" ) );
        validPopulatedConfigXml.append( testMultiValuedKeyValueString1 );
        validPopulatedConfigXml.append( XmlCloseTag( "_0" ) );

        validPopulatedConfigXml.append( XmlOpenTag( "_1" ) );
        validPopulatedConfigXml.append( testMultiValuedKeyValueString2 );
        validPopulatedConfigXml.append( XmlCloseTag( "_1" ) );

        validPopulatedConfigXml.append( XmlOpenTag( "_2" ) );
        validPopulatedConfigXml.append( testMultiValuedKeyValueString3 );
        validPopulatedConfigXml.append( XmlCloseTag( "_2" ) );
    }
    validPopulatedConfigXml.append( XmlCloseTag( testMultiValuedKeyName.ToQString() ) );

    validPopulatedConfigXml.append( XmlOpenTagAttr( testGroupWithIdName.ToQString(), testGroupId1 ) );
    {
        validPopulatedConfigXml.append( XmlOpenTag( testSubKeyName.ToQString() ) );
        validPopulatedConfigXml.append( testSubKeyValueString1 );
        validPopulatedConfigXml.append( XmlCloseTag( testSubKeyName.ToQString() ) );

        validPopulatedConfigXml.append( XmlOpenTag( testMultiValuedSubKeyName.ToQString() ) );
        {
            validPopulatedConfigXml.append( XmlOpenTag( "_0" ) );
            validPopulatedConfigXml.append( testMultiValuedSubKeyValueString1 );
            validPopulatedConfigXml.append( XmlCloseTag( "_0" ) );

            validPopulatedConfigXml.append( XmlOpenTag( "_1" ) );
            validPopulatedConfigXml.append( testMultiValuedSubKeyValueString2 );
            validPopulatedConfigXml.append( XmlCloseTag( "_1" ) );
        }
        validPopulatedConfigXml.append( XmlCloseTag( testMultiValuedSubKeyName.ToQString() ) );
    }
    validPopulatedConfigXml.append( XmlCloseTag( testGroupWithIdName.ToQString() ) );
    validPopulatedConfigXml.append( XmlOpenTagAttr( testGroupWithIdName.ToQString(), testGroupId2 ) );
    {
        validPopulatedConfigXml.append( XmlOpenTag( testSubKeyName.ToQString() ) );
        validPopulatedConfigXml.append( testSubKeyValueString2 );
        validPopulatedConfigXml.append( XmlCloseTag( testSubKeyName.ToQString() ) );
    }
    validPopulatedConfigXml.append( XmlCloseTag( testGroupWithIdName.ToQString() ) );

    validPopulatedConfigXml.append( XmlCloseTag( schemaName.ToQString() ) );

    QByteArray validXmlByteArray( validPopulatedConfigXml.toAscii() );
    QBuffer buffer( &validXmlByteArray );

    XmlConfigFileReader reader;

    EXPECT_TRUE(reader.ReadFrom( buffer ) ) << "ReadFrom returns true if Xml is valid";
    WbKeyValues keyValues;
    reader.ReadKeyValues( testKeyNameNoId, keyValues );
    EXPECT_EQ(1 , keyValues.GetKeyValues( testKeyNameNoId ).size()) << "Since the key exists, reading the key's value puts it in the WbKeyValues structure once";

    EXPECT_EQ(KeyValue::from( testKeyNoIdValueString ) , keyValues.GetKeyValue( testKeyNameNoId )) << "Since the key exists, reading the key's value puts it in the WbKeyValues structure";

    reader.ReadKeyValues( testKeyNameWithId, keyValues );
    EXPECT_EQ(2 , keyValues.GetKeyValues( testKeyNameWithId ).size()) << "Since the key exists, reading the key's value puts it in the WbKeyValues structure twice (once per id)";

    EXPECT_EQ(KeyValue::from( testKeyWithIdValue1String ) , keyValues.GetKeyValue( testKeyNameWithId, testKeyWithIdId1 )) << "Since the key exists, reading the key's value puts it in the WbKeyValues structure with correct id (id 1)";

    EXPECT_EQ(KeyValue::from( testKeyWithIdValue2String ) , keyValues.GetKeyValue( testKeyNameWithId, testKeyWithIdId2 )) << "Since the key exists, reading the key's value puts it in the WbKeyValues structure with correct id (id 2)";

    reader.ReadKeyValues( testMultiValuedKeyName, keyValues );
    EXPECT_EQ(1 , keyValues.GetKeyValues( testMultiValuedKeyName ).size()) << "Since the key exists, reading the multi-valued key's value puts it in the WbKeyValues structure once";

    EXPECT_EQ(KeyValue() << testMultiValuedKeyValueString1 << testMultiValuedKeyValueString2 << testMultiValuedKeyValueString3 , keyValues.GetKeyValue( testMultiValuedKeyName )) << "Since the key exists, reading the multi-valued key's value puts it in the WbKeyValues structure as one value, and it is retrieved as a list";

    reader.ReadKeyValuesFromGroup( testSubKeyName, testGroupWithIdName, keyValues );
    reader.ReadKeyValuesFromGroup( testMultiValuedSubKeyName, testGroupWithIdName, keyValues );

    EXPECT_EQ(0 , keyValues.GetKeyValues( testGroupWithIdName ).size()) << "The group's name is not added as a key";

    EXPECT_EQ(2 , keyValues.GetKeyValues( testSubKeyName ).size()) << "Since the key exists, reading the group subkey's value puts it in the WbKeyValues structure twice (once per id)";

    EXPECT_EQ(KeyValue::from( testSubKeyValueString1 ) , keyValues.GetKeyValue( testSubKeyName, testGroupId1 )) << "Since the key exists, reading the group subkey's value puts it in the WbKeyValues structure with correct id (from the group - id 1)";

    EXPECT_EQ(KeyValue() << testMultiValuedSubKeyValueString1 << testMultiValuedSubKeyValueString2 , keyValues.GetKeyValue( testMultiValuedSubKeyName, testGroupId1 )) << "Since the key exists, reading the multi-valued group subkey's value puts it in the WbKeyValues structure with correct id (from the group - id 1)";

    EXPECT_EQ(KeyValue::from( testSubKeyValueString2 ) , keyValues.GetKeyValue( testSubKeyName, testGroupId2 )) << "Since the key exists, reading the group subkey's value puts it in the WbKeyValues structure with correct id (from the group - id 2)";


}

TEST(XmlConfigFileReaderTests, SubSchemaLocations)
{
    const QString subSchemaFileName( "path/to/subschema/file" );
    const KeyName schemaTag( "schemaTag" );
    const KeyName subSchemaTagNoId( "subSchemaTagNoId" );

    const QString subSchemaIdFileName1( "path/to/subschemaId/file1" );
    const QString subSchemaIdFileName2( "path/to/subschemaId/file2" );
    const KeyName subSchemaTagWithIds( "subSchemaTagWithIds" );
    const KeyId   subSchemaId1( "subSchemaId1" );
    const KeyId   subSchemaId2( "subSchemaId2" );

    QString xml;
    xml.append( XmlOpenTag( schemaTag.ToQString() ) );

    xml.append( XmlOpenTag( subSchemaTagNoId.ToQString() ) );
    xml.append( XmlOpenTag( xmlConfigFileNameTag ) );
    xml.append( subSchemaFileName );
    xml.append( XmlCloseTag( xmlConfigFileNameTag ) );
    xml.append( XmlCloseTag( subSchemaTagNoId.ToQString() ) );

    xml.append( XmlOpenTagAttr( subSchemaTagWithIds.ToQString(), subSchemaId1 ) );
    xml.append( XmlOpenTag( xmlConfigFileNameTag ) );
    xml.append( subSchemaIdFileName1 );
    xml.append( XmlCloseTag( xmlConfigFileNameTag ) );
    xml.append( XmlCloseTag( subSchemaTagWithIds.ToQString() ) );

    xml.append( XmlOpenTagAttr( subSchemaTagWithIds.ToQString(), subSchemaId2 ) );
    xml.append( XmlOpenTag( xmlConfigFileNameTag ) );
    xml.append( subSchemaIdFileName2 );
    xml.append( XmlCloseTag( xmlConfigFileNameTag ) );
    xml.append( XmlCloseTag( subSchemaTagWithIds.ToQString() ) );

    xml.append( XmlCloseTag( schemaTag.ToQString() ) );

    QByteArray xmlByteArray( xml.toAscii() );
    QBuffer buffer( &xmlByteArray );

    XmlConfigFileReader reader;

    reader.ReadFrom( buffer );

    SchemaLocationsList locations;

    reader.ReadSubSchemaLocations( subSchemaTagNoId, locations );

    EXPECT_EQ( 1 , locations.size() ) <<  "There is exactly no id sub schema";
    EXPECT_EQ( subSchemaFileName , locations.at( 0 ).location ) <<  "The no id sub schema location is correct";

    reader.ReadSubSchemaLocations( subSchemaTagWithIds, locations );

    EXPECT_EQ( 2 , locations.size() ) <<  "There are exactly two sub schemas with id";

    WbSchemaLocationIdPair subSchemaWithId1;
    subSchemaWithId1.location = subSchemaIdFileName1;
    subSchemaWithId1.id = subSchemaId1;

    EXPECT_TRUE(container( locations ).contains( subSchemaWithId1 ) ) << "First sub schema with id is correct";

    WbSchemaLocationIdPair subSchemaWithId2;
    subSchemaWithId2.location = subSchemaIdFileName2;
    subSchemaWithId2.id = subSchemaId2;

    EXPECT_TRUE(container( locations ).contains( subSchemaWithId2 ) ) << "Second sub schema with id is correct";

}

