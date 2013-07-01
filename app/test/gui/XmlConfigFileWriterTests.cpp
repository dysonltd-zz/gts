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
#include "XmlConfigFileWriter.h"
#include "MockIoDevice.h"

#include "TestHelpers.h"
#include "WbKeyValues.h"
#include <QtXml/qdom.h>

using namespace XmlTools;

namespace
{
    const KeyName schemaName( "testSchema" );
    const KeyName testKeyNameNoId( "testKeyNameNoId" );
    const KeyName testKeyNameWithId( "testKeyNameWithId" );
    const QString testKeyNoIdValueString( "testKeyNoIdValue" );
    const QString testKeyWithIdValue1String( "testKeyWithIdValue1" );
    const QString testKeyWithIdValue2String( "testKeyWithIdValue2" );
    const KeyId testKeyWithIdId1( "testKeyWithIdId1" );
    const KeyId testKeyWithIdId2( "testKeyWithIdId2" );

    const KeyName testMultiValuedKeyName( "testMatrixKey" );
    const KeyId   testMultiValuedKeyId( "testMultiValKeyId" );
    const QString testMultiValuedKeyValueString1( "A" );
    const QString testMultiValuedKeyValueString2( "B" );
    const QString testMultiValuedKeyValueString3( "C" );

    const KeyName testGroupWithIdName( "testGroupWithId" );
    const KeyName testSubKeyName( "testSubKeyName" );
    const QString testSubKeyValueString1( "testSubKeyValueString1" );
    const QString testSubKeyValueString2( "testSubKeyValueString2" );
    const KeyId   testGroupId1( "testSubKeyId1" );
    const KeyId   testGroupId2( "testSubKeyId2" );

    const KeyName testMultiValuedSubKeyName( "testMatrixSubKey" );
    const QString testMultiValuedSubKeyValueString1( "X" );
    const QString testMultiValuedSubKeyValueString2( "Z" );

    QDomElement AddTagPair( const QString& tagName,
                            QDomElement& currentElement,
                            QDomDocument& document )
    {
        QDomElement newElement = document.createElement( tagName );
        currentElement.appendChild( newElement );
        return newElement;
    }
    QDomElement AddTagPairWithContents( const QString& tagName,
                                        const QString& contents,
                                        QDomElement& currentElement,
                                        QDomDocument& document )
    {
        QDomElement newElement = AddTagPair( tagName, currentElement, document );
        newElement.appendChild( document.createTextNode( contents ) );
        return newElement;
    }

    QDomElement AddTagPairWithIdAttribute( const QString& tagName,
                                           const QString& idAttr,
                                           QDomElement& currentElement,
                                           QDomDocument& document )
    {
        QDomElement newElement = AddTagPair( tagName, currentElement, document );
        newElement.setAttribute( xmlIdAttribute, idAttr );
        return newElement;
    }

    QDomElement AddTagPairWithContentsAndIdAttribute( const QString& tagName,
                                                      const QString& idAttr,
                                                      const QString& contents,
                                                      QDomElement& currentElement,
                                                      QDomDocument& document )
    {
        QDomElement newElement = AddTagPairWithContents( tagName,
                                                         contents,
                                                         currentElement,
                                                         document );
        newElement.setAttribute( xmlIdAttribute, idAttr );
        return newElement;
    }
}

TEST(XmlConfigFileWriterTests, IoDeviceInteraction)
{
    MockIoDevice mockIoDevice;

    XmlConfigFileWriter writer;

    mockIoDevice.m_openSuccessful = false;

    EXPECT_FALSE(writer.WriteTo( mockIoDevice ) ) << "WriteTo returns false if the open fails";

    EXPECT_EQ(QIODevice::WriteOnly , mockIoDevice.m_openModeRequested) << "The reader requested WriteOnly access to the IoDevice";
}

TEST(XmlConfigFileWriterTests, FullConfigWriteTest)
{
    QDomDocument expectedDomDocument;

    XmlConfigFileWriter writer;
    writer.StartConfigFile( schemaName );

    QDomElement root = expectedDomDocument.createElement( schemaName.ToQString() );
    expectedDomDocument.appendChild( root );

    writer.WriteKey( testKeyNameNoId, KeyValue::from( testKeyNoIdValueString ), QString() );
    AddTagPairWithContents( testKeyNameNoId.ToQString(), testKeyNoIdValueString, root, expectedDomDocument );

    writer.WriteKey( testKeyNameWithId, KeyValue::from( testKeyWithIdValue1String ), testKeyWithIdId1 );
    AddTagPairWithContentsAndIdAttribute( testKeyNameWithId.ToQString(), testKeyWithIdId1, testKeyWithIdValue1String, root, expectedDomDocument );
    writer.WriteKey( testKeyNameWithId, KeyValue::from( testKeyWithIdValue2String ), testKeyWithIdId2 );
    AddTagPairWithContentsAndIdAttribute( testKeyNameWithId.ToQString(), testKeyWithIdId2, testKeyWithIdValue2String, root, expectedDomDocument );

    writer.StartGroup( testGroupWithIdName, testGroupId1 );
    writer.WriteKey( testSubKeyName, KeyValue::from( testSubKeyValueString1 ), testGroupId1 );
    writer.WriteKey( testMultiValuedSubKeyName,  KeyValue() << testMultiValuedSubKeyValueString1 << testMultiValuedSubKeyValueString2,
                     testGroupId1 );
    writer.EndGroup( testGroupWithIdName, testGroupId1 );

    QDomElement groupElement = AddTagPairWithIdAttribute( testGroupWithIdName.ToQString(), testGroupId1, root, expectedDomDocument );
    AddTagPairWithContents( testSubKeyName.ToQString(), testSubKeyValueString1, groupElement, expectedDomDocument ); //NB: no id attr on group members
    QDomElement multiValSubKeyElement = AddTagPair( testMultiValuedSubKeyName.ToQString(), groupElement, expectedDomDocument ); //NB: no id attr on group members
    AddTagPairWithContents( "_0", testMultiValuedSubKeyValueString1, multiValSubKeyElement, expectedDomDocument );
    AddTagPairWithContents( "_1", testMultiValuedSubKeyValueString2, multiValSubKeyElement, expectedDomDocument );

    writer.StartGroup( testGroupWithIdName, testGroupId2 );
    writer.WriteKey( testSubKeyName, KeyValue::from( testSubKeyValueString2 ), testGroupId2 );
    writer.EndGroup( testGroupWithIdName, testGroupId2 );

    QDomElement groupElement2 = AddTagPairWithIdAttribute( testGroupWithIdName.ToQString(), testGroupId2, root, expectedDomDocument );
    AddTagPairWithContents( testSubKeyName.ToQString(), testSubKeyValueString2, groupElement2, expectedDomDocument ); //NB: no id attr on group members

    writer.WriteKey( testMultiValuedKeyName,
                      KeyValue() << testMultiValuedKeyValueString1 << testMultiValuedKeyValueString2 << testMultiValuedKeyValueString3,
                     testMultiValuedKeyId );
    QDomElement multiValElement = AddTagPairWithIdAttribute( testMultiValuedKeyName.ToQString(), testMultiValuedKeyId, root, expectedDomDocument );
    AddTagPairWithContents( "_0", testMultiValuedKeyValueString1, multiValElement, expectedDomDocument );
    AddTagPairWithContents( "_1", testMultiValuedKeyValueString2, multiValElement, expectedDomDocument );
    AddTagPairWithContents( "_2", testMultiValuedKeyValueString3, multiValElement, expectedDomDocument );

    writer.EndConfigFile( schemaName );

    QBuffer buffer;

    writer.WriteTo( buffer );

    QDomDocument actualDomDocument;
    actualDomDocument.setContent( &buffer ); //create a new DOM-document from the writer output to nullify formatting variations, etc.

    EXPECT_EQ( actualDomDocument.toString() , expectedDomDocument.toString() ) <<  "Xml output is the same as the expected DOM document";
}

TEST(XmlConfigFileWriterTests, SubConfigWriteTest)
{
    XmlConfigFileWriter writer;
    writer.StartConfigFile( schemaName );
    QDomDocument expectedDomDocument;

    QDomElement root = expectedDomDocument.createElement( schemaName.ToQString() );
    expectedDomDocument.appendChild( root );

    const KeyName subConfigKeyName( "subConfigKeyName" );
    const QString subConfigFileName( "path/to/subConfig/file" );

    {
        writer.WriteSubConfig( subConfigKeyName, QFileInfo( subConfigFileName ), KeyId() );
        QDomElement subConfigElement = AddTagPair( subConfigKeyName.ToQString(), root, expectedDomDocument );
        AddTagPairWithContents( xmlConfigFileNameTag, subConfigFileName, subConfigElement, expectedDomDocument );
    }

    const KeyName subConfigWithIdKeyName( "subConfigWithIdKeyName" );
    const QString subConfigWithIdFileName1( "path/to/subConfigWithId/file1" );
    const QString subConfigWithIdFileName2( "path/to/subConfig/with/id/file2" );
    const KeyId subConfigId1( "id1" );
    const KeyId subConfigId2( "id2" );

    {
        writer.WriteSubConfig( subConfigWithIdKeyName, QFileInfo( subConfigWithIdFileName1 ), subConfigId1 );
        QDomElement subConfigElement = AddTagPairWithIdAttribute( subConfigWithIdKeyName.ToQString(), subConfigId1, root, expectedDomDocument );
        AddTagPairWithContents( xmlConfigFileNameTag, subConfigWithIdFileName1, subConfigElement, expectedDomDocument );
    }
    {
        writer.WriteSubConfig( subConfigWithIdKeyName, QFileInfo( subConfigWithIdFileName2 ), subConfigId2 );
        QDomElement subConfigElement = AddTagPairWithIdAttribute( subConfigWithIdKeyName.ToQString(), subConfigId2, root, expectedDomDocument );
        AddTagPairWithContents( xmlConfigFileNameTag, subConfigWithIdFileName2, subConfigElement, expectedDomDocument );
    }
    writer.EndConfigFile( schemaName );

    QBuffer buffer;

    writer.WriteTo( buffer );

    QDomDocument actualDomDocument;
    actualDomDocument.setContent( &buffer ); //create a new DOM-document from the writer output to nullify formatting variations, etc.

    EXPECT_EQ(expectedDomDocument.toString(), actualDomDocument.toString())
            << "Xml output is the same as the expected DOM document for subconfigs with and without ids";
}

TEST(XmlConfigFileWriterTests, XmlSpecialCharacters)
{
    XmlConfigFileWriter writer;
    writer.StartConfigFile( schemaName );

    QDomDocument expectedDomDocument;
    QDomElement root = expectedDomDocument.createElement( schemaName.ToQString() );
    expectedDomDocument.appendChild( root );

    const KeyName  subKeyName( "subKeyName" );
    const KeyValue subKeyValueWithSpecialCharacters(
        KeyValue::from( "<need to & escape some of these > characters" ) );

    const QString placeholderString( "AAAAA" );
    const QString subKeyValueWithSpecialCharactersEscaped( "&lt;need to &amp; escape some "
                                                           "of these > characters" );

    AddTagPairWithContents( subKeyName.ToQString(),
                            placeholderString,
                            root,
                            expectedDomDocument );
    writer.WriteKey( subKeyName, subKeyValueWithSpecialCharacters, KeyId() );
    writer.EndConfigFile( schemaName );

    QBuffer buffer;
    writer.WriteTo( buffer );

    QDomDocument actualDomDocument;
    actualDomDocument.setContent( &buffer );

    EXPECT_EQ(expectedDomDocument.toString().replace(placeholderString, subKeyValueWithSpecialCharactersEscaped),
              actualDomDocument.toString())
            << "Xml output has the Xml special characters in encoded form";
}

