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
    const KeyName schemaName                         ( "testSchemaName" );
    const KeyName firstTopLevelSingleValuedKeyName   ( "name" );
    const KeyName secondTopLevelSingleValuedKeyName  ( "description" );
    const KeyName topLevelMultiValuedKeyName         ( "matrix" );

    const KeyName firstGroupKeyName                  ( "group1" );
    const KeyName firstGroupFirstSubKeyName          ( "group1Key1" );
    const KeyName firstGroupSecondSubKeyName         ( "group1Key2" );

    const KeyName secondGroupKeyName                 ( "group2" );
    const KeyName secondGroupFirstSubKeyName         ( "group2Key1" );
    const KeyName secondGroupSecondSubKeyName        ( "group2Key2" );

    struct TestSetup
    {
        TestSetup()
            :
            schema( schemaName )
        {
            schema.AddSingleValueKey( firstTopLevelSingleValuedKeyName, WbSchemaElement::Multiplicity::One );
            schema.AddSingleValueKey( secondTopLevelSingleValuedKeyName, WbSchemaElement::Multiplicity::One );
            schema.AddSingleValueKey( topLevelMultiValuedKeyName, WbSchemaElement::Multiplicity::One );

            schema.AddKeyGroup( firstGroupKeyName, WbSchemaElement::Multiplicity::One,
                                KeyNameList() << firstGroupFirstSubKeyName << firstGroupSecondSubKeyName );

            schema.AddKeyGroup( secondGroupKeyName,
                                WbSchemaElement::Multiplicity::One,
                                KeyNameList() << secondGroupFirstSubKeyName << secondGroupSecondSubKeyName );
        }

        WbSchema schema;
    };

    class MockConfigFileReader : public WbConfigFileReader
    {
    public:
        MockConfigFileReader()
        {
        }

        virtual MockConfigFileReader* const Clone() const
        {
            return new MockConfigFileReader;

        }
        virtual bool ReadFrom( QIODevice& ioDevice )
        {
            ( void ) ioDevice;
            return true;
        }

        virtual const KeyName GetSchemaName() const
        {
            return m_schemaName;
        }

        virtual void ReadKeyValues( const KeyName& keyName, WbKeyValues& keyValues )
        {
            ( void ) keyValues;
            m_keyValuesRequestedNoGroup.push_back( keyName );
        }

        virtual void ReadKeyValuesFromGroup( const KeyName& keyName, const KeyName& groupName, WbKeyValues& keyValues )
        {
            ( void ) keyValues;
            m_keyValuesRequestedByGroup[ groupName ].push_back( keyName );
        }

        virtual void ReadSubSchemaLocations( const KeyName& keyName, SchemaLocationsList& locationsList )
        {
            ( void ) keyName;
            ( void ) locationsList;
        }

        KeyName m_schemaName;

        std::vector<KeyName> m_keyValuesRequestedNoGroup;
        std::map< KeyName, std::vector<KeyName> > m_keyValuesRequestedByGroup;
    };

    class MockConfigFileWriter : public WbConfigFileWriter
    {
        typedef std::vector< QString > CallsVector;

        void EnsureTestMakesSense() const
        {
            ASSERT_GT(m_actualCalls->size(), 0U) << "Must be at least one call for this to make sense";
        }

    public:
        MockConfigFileWriter()
            :
            m_actualCalls( new CallsVector )
        {
        }

        MockConfigFileWriter* const Clone() const
        {
            return new MockConfigFileWriter( *this );
        }

        virtual bool WriteTo( QIODevice& ioDevice )
        {
            ( void ) ioDevice;
            return true;
        }

        virtual void StartConfigFile( const KeyName& name )
        {
            RecordCall( CallString( "StartConfigFile", name ) );
        }

        virtual void EndConfigFile( const KeyName& name )
        {
            RecordCall( CallString( "EndConfigFile", name ) );
        }

        virtual void WriteKey( const KeyName& name, const KeyValue& value, const KeyId& id )
        {
            RecordCall( CallString( "WriteKey", name, value, id ) );
        }

        virtual void StartGroup( const KeyName& name, const KeyId& id )
        {
            RecordCall( CallString( "StartGroup", name, id ) );
        }

        virtual void EndGroup( const KeyName& name, const KeyId& id )
        {
            RecordCall( CallString( "EndGroup", name, id ) );
        }

        virtual void WriteSubConfig( const KeyName& name, const QFileInfo& configFileLocation, const KeyId& id )
        {
            RecordCall( CallString( "WriteSubConfig", name, configFileLocation, id ) );
        }

        const size_t GetLastCallIndex() const
        {
            EnsureTestMakesSense();
            return m_actualCalls->size() - 1;
        }

        const size_t CountOfCalls( const QString& callString ) const
        {
            return std::count( m_actualCalls->begin(), m_actualCalls->end(), callString );
        }

        const size_t FindCallIndex( const QString& callString ) const
        {
            CallsVector::const_iterator itr = std::find( m_actualCalls->begin(), m_actualCalls->end(), callString );

            if ( itr == m_actualCalls->end() )
            {
                return ( size_t )( -1 );
            }

            return itr-m_actualCalls->begin();
        }

        const QString CallString( const QString& functionName, const KeyName& keyName ) const
        {
            return QString( "%1(%2)" ).arg( functionName ).arg( keyName.ToQString() );
        }

        const QString CallString( const QString& functionName, const KeyName& keyName, const KeyId& id ) const
        {
            return QString( "%1(%2,%3)" ).arg( functionName ).arg( keyName.ToQString() ).arg( id );
        }

        const QString CallString( const QString& functionName, const KeyName& keyName, const KeyValue& value, const KeyId& id ) const
        {
            return QString( "%1(%2,%3,%4)" ).arg( functionName ).arg( keyName.ToQString() ).arg( value.ToQString() ).arg( id );
        }

        const QString CallString( const QString& functionName, const KeyName& keyName, const QFileInfo& fileInfo, const KeyId& id ) const
        {
            return QString( "%1(%2,%3,%4)" ).arg( functionName ).arg( keyName.ToQString() ).arg( fileInfo.filePath() ).arg( id );
        }

        void ListCalls() const
        {
            for ( size_t i = 0; i < m_actualCalls->size(); ++i )
            {
                std::cout << m_actualCalls->at( i ).toStdString() << std::endl;
            }
        }

    private:
        MockConfigFileWriter( const MockConfigFileWriter& other )
            :
            m_actualCalls( other.m_actualCalls ) //share data so we can access clone calls too
        {

        }

        void RecordCall( const QString& call )
        {
            m_actualCalls->push_back( call );
        }

        QSharedPointer<CallsVector> m_actualCalls;

    };
}

TEST(WbConfigSchemaTests, SetAndRetrieve)
{
    TestSetup test;
    WbConfig config( test.schema, QFileInfo() );

    KeyValue singleValuedKeyValue( KeyValue::from( "testValue" ) );
    KeyId    singleValuedKeyId( "testId" );
    KeyValue multiValuedKeyValue =  KeyValue() << "testValue1" << "testValue2";

    config.SetKeyValue( firstTopLevelSingleValuedKeyName, singleValuedKeyValue );
    config.SetKeyValue( topLevelMultiValuedKeyName,       multiValuedKeyValue );

    EXPECT_EQ(singleValuedKeyValue , config.GetKeyValue( firstTopLevelSingleValuedKeyName )) << "Setting and retrieving a single-valued key works";
    EXPECT_EQ(multiValuedKeyValue , config.GetKeyValue( topLevelMultiValuedKeyName )) << "Setting and retrieving a multi-valued key works";

    config.SetKeyValue( secondTopLevelSingleValuedKeyName, singleValuedKeyValue, singleValuedKeyId );

    EXPECT_EQ(singleValuedKeyValue , config.GetKeyValue( secondTopLevelSingleValuedKeyName, singleValuedKeyId )) << "Setting and retrieving a single-valued key with id works";

    const KeyValue group1Key1Value( KeyValue::from( "group1Key1Value" ) );
    config.SetKeyValue( firstGroupFirstSubKeyName, group1Key1Value );

    EXPECT_EQ(group1Key1Value , config.GetKeyValue( firstGroupFirstSubKeyName )) << "Setting and retrieving a group subkey's value works";

}

TEST(WbConfigSchemaTests, ReadConfigTest)
{
    TestSetup test;

    WbConfig config( test.schema, QFileInfo() );

    MockConfigFileReader reader;

    reader.m_schemaName = KeyName( "IncorrectSchemaName" );

    const bool incorrectSchemaNameResult = config.ReadUsing( reader );

    EXPECT_FALSE(incorrectSchemaNameResult ) << "If the schema name is wrong, the read result is false";
    EXPECT_TRUE(reader.m_keyValuesRequestedNoGroup.empty() ) << "If the schema name is wrong, no values are read";
    EXPECT_TRUE(reader.m_keyValuesRequestedByGroup.empty() ) << "If the schema name is wrong, no values are read";

    reader.m_schemaName = schemaName;
    const bool correctSchemaNameResult = config.ReadUsing( reader );

    EXPECT_TRUE(correctSchemaNameResult ) << "If the schema name is correct, the read result is true";
    EXPECT_TRUE(container( reader.m_keyValuesRequestedNoGroup ).contains( firstTopLevelSingleValuedKeyName )
           && container( reader.m_keyValuesRequestedNoGroup ).contains( secondTopLevelSingleValuedKeyName )
           && container( reader.m_keyValuesRequestedNoGroup ).contains( topLevelMultiValuedKeyName ) )
           << "If the schema name is correct, all schema keys not in a group are read";

    EXPECT_TRUE(!container( reader.m_keyValuesRequestedNoGroup ).contains( firstGroupKeyName )
           && !container( reader.m_keyValuesRequestedNoGroup ).contains( secondGroupKeyName )
           && !container( reader.m_keyValuesRequestedNoGroup ).contains( firstGroupFirstSubKeyName )
           && !container( reader.m_keyValuesRequestedNoGroup ).contains( firstGroupSecondSubKeyName )
           && !container( reader.m_keyValuesRequestedNoGroup ).contains( secondGroupFirstSubKeyName )
           && !container( reader.m_keyValuesRequestedNoGroup ).contains( secondGroupSecondSubKeyName ) )
            << "If the schema name is correct, schema's group keys are not read, "
               "nor are they're subkeys without specifying the group";
    EXPECT_TRUE(container(reader.m_keyValuesRequestedByGroup[firstGroupKeyName]).contains(firstGroupFirstSubKeyName)
           && container(reader.m_keyValuesRequestedByGroup[firstGroupKeyName]).contains(firstGroupSecondSubKeyName)
           && container(reader.m_keyValuesRequestedByGroup[secondGroupKeyName]).contains(secondGroupFirstSubKeyName)
           && container(reader.m_keyValuesRequestedByGroup[secondGroupKeyName]).contains(secondGroupSecondSubKeyName))
            << "If the schema name is correct, all schema keys in a group are read from the correct groups";
}

TEST(WbConfigSchemaTests, EmptyConfigWriteTest)
{
    TestSetup test;

    WbConfig config( test.schema, QFileInfo() );

    MockConfigFileWriter writer;

    config.WriteUsing( writer );

    EXPECT_EQ(0 , writer.FindCallIndex( writer.CallString( "StartConfigFile", schemaName ) )) << "Empty config file writes config file start first to writer";

    EXPECT_EQ(1 , writer.FindCallIndex( writer.CallString( "EndConfigFile", schemaName ) )) << "Empty config file then writes config file end to writer";

    EXPECT_EQ( 1 , writer.GetLastCallIndex() ) <<  "That's all empty config file writes";
}

TEST(WbConfigSchemaTests, FullConfigWriteTest)
{
    TestSetup test;

    WbConfig config( test.schema, QFileInfo() );

    const KeyValue firstTopLevelSingleValuedKeyValue  ( KeyValue::from( "aName" ) );
    const KeyId secondTopLevelSingleValuedKeyId1      ( "anId" );
    const KeyId secondTopLevelSingleValuedKeyId2      ( "anotherId" );
    const KeyValue secondTopLevelSingleValuedKeyValue1( KeyValue::from( "aDescription" ) );
    const KeyValue secondTopLevelSingleValuedKeyValue2( KeyValue::from( "anotherDescription" ) );
    const KeyValue topLevelMultiValuedKeyValue        ( KeyValue::from( "aMatrix" ) );

    const KeyValue firstGroupFirstSubKeyValue         ( KeyValue::from( "aGroup1Key1" ) );
    const KeyValue firstGroupSecondSubKeyValue        ( KeyValue::from( "aGroup1Key2" ) );

    const KeyId    secondGroupId1                     ( "anId" );
    const KeyValue secondGroupFirstSubKeyValue1       ( KeyValue::from( "aGroup2Key1" ) );
    const KeyValue secondGroupSecondSubKeyValue1      ( KeyValue::from( "aGroup2Key2" ) );

    const KeyId    secondGroupId2                     ( "anotherId" );
    const KeyValue secondGroupFirstSubKeyValue2       ( KeyValue::from( "anotherGroup2Key1" ) );
    const KeyValue secondGroupSecondSubKeyValue2      ( KeyValue::from( "anotherGroup2Key2" ) );

    MockConfigFileWriter writer;

    config.SetKeyValue( firstTopLevelSingleValuedKeyName, firstTopLevelSingleValuedKeyValue );

    config.SetKeyValue( secondTopLevelSingleValuedKeyName, secondTopLevelSingleValuedKeyValue1, secondTopLevelSingleValuedKeyId1 );
    config.SetKeyValue( secondTopLevelSingleValuedKeyName, secondTopLevelSingleValuedKeyValue2, secondTopLevelSingleValuedKeyId2 );

    config.SetKeyValue( topLevelMultiValuedKeyName, topLevelMultiValuedKeyValue );

    config.SetKeyValue( firstGroupFirstSubKeyName, firstGroupFirstSubKeyValue );
    config.SetKeyValue( firstGroupSecondSubKeyName, firstGroupSecondSubKeyValue );

    config.SetKeyValue( secondGroupFirstSubKeyName, secondGroupFirstSubKeyValue1, secondGroupId1 );
    config.SetKeyValue( secondGroupSecondSubKeyName, secondGroupSecondSubKeyValue1, secondGroupId1 );

    config.SetKeyValue( secondGroupFirstSubKeyName, secondGroupFirstSubKeyValue2, secondGroupId2 );
    config.SetKeyValue( secondGroupSecondSubKeyName, secondGroupSecondSubKeyValue2, secondGroupId2 );

    config.WriteUsing( writer );

    EXPECT_EQ(0 , writer.FindCallIndex( writer.CallString( "StartConfigFile", schemaName ) )) << "Config file start is the first thing written";

    EXPECT_EQ(writer.GetLastCallIndex() , writer.FindCallIndex( writer.CallString( "EndConfigFile", schemaName ) )) << "Config file end is the last thing written";


    EXPECT_EQ(1 , writer.CountOfCalls( writer.CallString( "WriteKey", firstTopLevelSingleValuedKeyName, firstTopLevelSingleValuedKeyValue, KeyId() ) )) << "The first top level key (no id) is written once";

    EXPECT_EQ(1 , writer.CountOfCalls( writer.CallString( "WriteKey", secondTopLevelSingleValuedKeyName, secondTopLevelSingleValuedKeyValue1, secondTopLevelSingleValuedKeyId1 ) )) << "The second top level key is written once with the first id...";

    EXPECT_EQ(1 , writer.CountOfCalls( writer.CallString( "WriteKey", secondTopLevelSingleValuedKeyName, secondTopLevelSingleValuedKeyValue2, secondTopLevelSingleValuedKeyId2 ) )) << "...and once with the second id";

    EXPECT_EQ(1 , writer.CountOfCalls( writer.CallString( "WriteKey", topLevelMultiValuedKeyName, topLevelMultiValuedKeyValue, KeyId() ) )) << "The top level multi-valued key (no id) is written once";

    EXPECT_EQ(1 , writer.CountOfCalls( writer.CallString( "StartGroup", firstGroupKeyName, KeyId() ) )) << "The first group (no id) is started once";

    EXPECT_EQ(1 , writer.CountOfCalls( writer.CallString( "EndGroup", firstGroupKeyName, KeyId() ) )) << "The first group (no id) is ended once";

    EXPECT_EQ(1 , writer.CountOfCalls( writer.CallString( "WriteKey", firstGroupFirstSubKeyName, firstGroupFirstSubKeyValue, KeyId() ) )) << "The first group first subKey is written once";

    EXPECT_EQ(1 , writer.CountOfCalls( writer.CallString( "WriteKey", firstGroupSecondSubKeyName, firstGroupSecondSubKeyValue, KeyId() ) )) << "The first group second subKey is written once";

    EXPECT_LT(writer.FindCallIndex( writer.CallString( "StartGroup", firstGroupKeyName, KeyId() ) ), writer.FindCallIndex( writer.CallString( "WriteKey", firstGroupFirstSubKeyName, firstGroupFirstSubKeyValue, KeyId() ) ) ) << "The first group is started before its first subkey";

    EXPECT_GT(writer.FindCallIndex( writer.CallString( "EndGroup", firstGroupKeyName, KeyId() ) ), writer.FindCallIndex( writer.CallString( "WriteKey", firstGroupFirstSubKeyName, firstGroupFirstSubKeyValue, KeyId() ) ) ) << "The first group is ended after its first subkey";

    EXPECT_LT(writer.FindCallIndex( writer.CallString( "StartGroup", firstGroupKeyName, KeyId() ) ), writer.FindCallIndex( writer.CallString( "WriteKey", firstGroupSecondSubKeyName, firstGroupSecondSubKeyValue, KeyId() ) ) ) << "The first group is started before its second subkey";

    EXPECT_GT(writer.FindCallIndex( writer.CallString( "EndGroup", firstGroupKeyName, KeyId() ) ), writer.FindCallIndex( writer.CallString( "WriteKey", firstGroupSecondSubKeyName, firstGroupSecondSubKeyValue, KeyId() ) ) ) << "The first group is ended after its second subkey";


    EXPECT_EQ(1 , writer.CountOfCalls( writer.CallString( "StartGroup", secondGroupKeyName, secondGroupId1 ) )) << "The second group is started once with the first id";
    EXPECT_EQ(1 , writer.CountOfCalls( writer.CallString( "EndGroup", secondGroupKeyName, secondGroupId1 ) )) << "The second group is ended once with the first id";

    EXPECT_EQ(1 , writer.CountOfCalls( writer.CallString( "StartGroup", secondGroupKeyName, secondGroupId2 ) )) << "The second group is started once with the second id";

    EXPECT_EQ(1 , writer.CountOfCalls( writer.CallString( "EndGroup", secondGroupKeyName, secondGroupId2 ) )) << "The second group is ended once with the second id";

    EXPECT_LT(writer.FindCallIndex( writer.CallString( "StartGroup", secondGroupKeyName, secondGroupId1 ) ), writer.FindCallIndex( writer.CallString( "WriteKey", secondGroupFirstSubKeyName, secondGroupFirstSubKeyValue1, secondGroupId1 ) ) ) << "The second group is started before its first subkey with first id";

    EXPECT_GT(writer.FindCallIndex( writer.CallString( "EndGroup", secondGroupKeyName, secondGroupId1 ) ), writer.FindCallIndex( writer.CallString( "WriteKey", secondGroupFirstSubKeyName, secondGroupFirstSubKeyValue1, secondGroupId1 ) ) ) << "The second group is ended after its first subkey with first id";

    EXPECT_LT(writer.FindCallIndex( writer.CallString( "StartGroup", secondGroupKeyName, secondGroupId1 ) ), writer.FindCallIndex( writer.CallString( "WriteKey", secondGroupSecondSubKeyName, secondGroupSecondSubKeyValue1, secondGroupId1 ) ) ) << "The second group is started before its second subkey with first id";

    EXPECT_GT(writer.FindCallIndex( writer.CallString( "EndGroup", secondGroupKeyName, secondGroupId1 ) ), writer.FindCallIndex( writer.CallString( "WriteKey", secondGroupSecondSubKeyName, secondGroupSecondSubKeyValue1, secondGroupId1 ) ) ) << "The second group is ended after its second subkey with first id";

    EXPECT_LT(writer.FindCallIndex( writer.CallString( "StartGroup", secondGroupKeyName, secondGroupId1 ) ), writer.FindCallIndex( writer.CallString( "WriteKey", secondGroupFirstSubKeyName, secondGroupFirstSubKeyValue2, secondGroupId2 ) ) ) << "The second group is started before its first subkey with second id";

    EXPECT_GT(writer.FindCallIndex( writer.CallString( "EndGroup", secondGroupKeyName, secondGroupId2 ) ), writer.FindCallIndex( writer.CallString( "WriteKey", secondGroupFirstSubKeyName, secondGroupFirstSubKeyValue2, secondGroupId2 ) ) ) << "The second group is ended after its first subkey with second id";

    EXPECT_LT(writer.FindCallIndex( writer.CallString( "StartGroup", secondGroupKeyName, secondGroupId2 ) ), writer.FindCallIndex( writer.CallString( "WriteKey", secondGroupSecondSubKeyName, secondGroupSecondSubKeyValue2, secondGroupId2 ) ) ) << "The second group is started before its second subkey with second id";

    EXPECT_GT(writer.FindCallIndex( writer.CallString( "EndGroup", secondGroupKeyName, secondGroupId2 ) ), writer.FindCallIndex( writer.CallString( "WriteKey", secondGroupSecondSubKeyName, secondGroupSecondSubKeyValue2, secondGroupId2 ) ) ) << "The second group is ended after its second subkey with second id";

}

TEST(WbConfigSchemaTests, WriteWithSubConfig)
{
    WbSchema schemaWithSubSchema( KeyName( "testSchemaWithSubSchema" ) );

    WbSchema subSchemaWithDefaultFileName( KeyName( "testSubSchema" ) );

    const KeyName subSchemaKeyName( subSchemaWithDefaultFileName.Name() );
    schemaWithSubSchema.AddSubSchema( subSchemaWithDefaultFileName,
                                      WbSchemaElement::Multiplicity::One );

    WbConfig configWithSubConfig( schemaWithSubSchema, QFileInfo() );
    const QString subConfigFileName( "subConfigFileName" );

    const QFileInfo subConfigFileInfo( subConfigFileName );

    configWithSubConfig.CreateSubConfig( subSchemaKeyName, subConfigFileName );

    MockConfigFileWriter writer;
    configWithSubConfig.WriteUsing( writer );

    EXPECT_EQ(1 , writer.CountOfCalls( writer.CallString( "WriteSubConfig", subSchemaKeyName, subConfigFileInfo, KeyId() ) )) << "The subConfig info is written once to the parent config file";

    EXPECT_EQ(1 , writer.CountOfCalls( writer.CallString( "StartConfigFile", subSchemaWithDefaultFileName.Name() ) )) << "The subConfig file is written too";

}

TEST(WbConfigSchemaTests, ReadWithSubConfig)
{
    const QString testXmlsRelativeDir( "../../app/test/res/" );
    const QString testXmlsDir( QApplication::applicationDirPath() + "/" + testXmlsRelativeDir );
    const QString validXmlConfigFile( testXmlsDir + "testValidXmlConfigFile.xml" );
    const QString validChildXmlConfigFile1( testXmlsDir + "testValidChildXmlConfigFile1.xml" );
    const QString validChildXmlConfigFile2( testXmlsDir + "testValidChildXmlConfigFile2.xml" );
    const QString validChildXmlConfigFile1Child( testXmlsDir + "testValidChildXmlConfigFile1Child.xml" );

    WbSchema schema( KeyName( "validXmlConfigFileRootTag" ) );
    schema.AddSingleValueKey( KeyName( "name" ), WbSchemaElement::Multiplicity::One );

    WbSchema childSchema1( KeyName( "validChildXmlConfigFile1RootTag" ) );
    childSchema1.AddSingleValueKey( KeyName( "name" ), WbSchemaElement::Multiplicity::One );

    WbSchema childSchema1Child( KeyName( "validChildXmlConfigFile1ChildRootTag" ) );
    childSchema1Child.AddSingleValueKey( KeyName( "name" ), WbSchemaElement::Multiplicity::One );

    childSchema1.AddSubSchema( childSchema1Child, WbSchemaElement::Multiplicity::One );

    WbSchema childSchema2( KeyName( "validChildXmlConfigFile2RootTag" ) );
    childSchema2.AddSingleValueKey( KeyName( "name" ), WbSchemaElement::Multiplicity::One );

    schema.AddSubSchema( childSchema1, WbSchemaElement::Multiplicity::One );
    schema.AddSubSchema( childSchema2, WbSchemaElement::Multiplicity::One );

    WbConfig config( schema, QFileInfo( validXmlConfigFile ) );

    XmlConfigFileReader reader;
    config.ReadUsing( reader );

    EXPECT_EQ(KeyValue::from( "Valid Xml Config File" ) , config.GetKeyValue( KeyName( "name" ) )) << "Can correctly read the Xml file's name tag contents";

    WbConfig childConfig1( config.GetSubConfig( childSchema1.Name() ) );
    EXPECT_EQ(KeyValue::from( "Valid Child Xml Config File 1" ) , childConfig1.GetKeyValue( KeyName( "name" ) )) << "can get name tag from child config, relative paths are relative the the directory of the file they're specified in";

    WbConfig childConfig2( config.GetSubConfig( childSchema2.Name() ) );
    EXPECT_EQ(KeyValue::from( "Valid Child Xml Config File 2" ) , childConfig2.GetKeyValue( KeyName( "name" ) )) << "can get name tag from other child config";

}

TEST(WbConfigSchemaTests, FileInfoRelativeToParent)
{
    const QString parentFileDirAbsolute(  QDir::rootPath() + "someDir/" );
    const QString parentFileName( "anyFileName" );

    const QString parentFilePath( parentFileDirAbsolute + parentFileName );

    WbConfig parentConfig( WbSchema( KeyName( "anySchema" ) ), parentFilePath );

    const QString childFileName( "testRelativeFileName" );
    WbConfig subConfig( parentConfig.CreateSubConfig( KeyName( "anyKeyName" ), childFileName ) );

    EXPECT_EQ(subConfig.GetAbsoluteFileInfo().absoluteFilePath() , parentFileDirAbsolute + childFileName) << "Config with non-null parent whose fileName passed to its constructor is relative, returns a file info relative to the directory of the parent file";

}

TEST(WbConfigSchemaTests, NullConfig)
{
    WbConfig config;

    GTEST_SUCCEED() << "Can create a null config (no schema/file info)";

    const QString fileName( "testRelativeFileName" );

    const QFileInfo relativeFileInfo( fileName );

    WbConfig subConfig( WbSchema( KeyName( "anySchema" ) ), fileName );

    EXPECT_EQ(subConfig.GetAbsoluteFileInfo().absoluteFilePath() , relativeFileInfo.absoluteFilePath()) << "Config which has no parent returns a file info based only on the fileName passed to its constructor";
}

TEST(WbConfigSchemaTests, DefaultValuesSetInitiallyFromSchema)
{
    const KeyValue defaultValue( KeyValue::from( "Test Default Value" ) );

    const KeyName testSchemaName( "testSchema" );
    WbSchema testSchema( KeyName( "testSchema" ) );

    const KeyName keyWithDefaultName( "testKey" );

    WbSchema subSchemaWithDefaultFileName( KeyName( "subSchemaName" ) );
    const QString defaultSubSchemaFileName( "defaultSubSchemaFileName" );

    const KeyName subSchemaKeyName( "subSchemaKeyName" );
    const KeyValue subSchemaKeyDefaultValue( KeyValue::from( "subSchemaKeyName" ) );

    subSchemaWithDefaultFileName.AddSingleValueKey( subSchemaKeyName,
            WbSchemaElement::Multiplicity::One,
            subSchemaKeyDefaultValue );

    testSchema.AddSingleValueKey( keyWithDefaultName,
                                  WbSchemaElement::Multiplicity::One,
                                  defaultValue );
    testSchema.AddSubSchema( subSchemaWithDefaultFileName,
                             WbSchemaElement::Multiplicity::One,
                             defaultSubSchemaFileName );

    const QString parentDir( QDir::rootPath() + "parentDir/" );
    const QString parentFile( "parentFile" );

    WbConfig config( testSchema, QFileInfo( parentDir + parentFile ) );

    EXPECT_EQ( testSchemaName , config.GetSchemaName() ) <<  "The config has correct schema name";

    EXPECT_EQ(defaultValue , config.GetKeyValue( keyWithDefaultName )) << "The config has default set for key where default was specified in schema";

    EXPECT_EQ(subSchemaWithDefaultFileName.Name() , config.GetSubConfig( subSchemaWithDefaultFileName.Name() ) .GetSchemaName()) << "The config has a sub config at the default key where default was specified in schema";

    EXPECT_EQ(parentDir + defaultSubSchemaFileName , config.GetSubConfig( subSchemaWithDefaultFileName.Name() ) .GetAbsoluteFileInfo().absoluteFilePath()) << "The sub config has correct fileName";

    EXPECT_EQ(subSchemaKeyDefaultValue , config.GetSubConfig( subSchemaWithDefaultFileName.Name() ) .GetKeyValue( subSchemaKeyName )) << "The created sub config has its defaults applied";


    MockConfigFileReader reader;

    reader.m_schemaName = testSchemaName;

    config.ReadUsing( reader );

    EXPECT_EQ(defaultValue , config.GetKeyValue( keyWithDefaultName )) << "After reading an empty config file, the config still has default set for key where default was specified in schema";
}

