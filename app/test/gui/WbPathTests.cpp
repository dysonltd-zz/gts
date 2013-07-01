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

#include "WbPath.h"
#include <sstream>
#include "WbConfig.h"

namespace
{
    struct TestSetup
    {
        TestSetup()
        :
            path1               (),
            uniqueElement       ( WbPathElement::Unique( KeyName( "element1" ) ) ),
            anotherUniqueElement( WbPathElement::Unique(
                                                KeyName( "anotherUniqueElement" ) ) ),
            unknownIdElement    ( WbPathElement::UnknownId( KeyName( "element2" ) ) ),
            knownIdElement1     ( WbPathElement::KnownId( KeyName( "element3" ),
                                                          KeyId( "element3Id" ) ) ),
            knownIdElement1WithDifferentId( WbPathElement::KnownId(
                                                KeyName( "element3" ),
                                                KeyId( "element3DifferentId" ) ) ),
            unknownIdElementWithSameNameAsKnown1( WbPathElement::UnknownId(
                                                        KeyName( "element3" ) ) ),
            knownIdElement2  ( WbPathElement::KnownId( KeyName( "element4" ),
                                                       KeyId( "element4Id" ) ) )
        {
            InitialiseDefaultPath( path1 );
        }

        void InitialiseDefaultPath( WbPath& path )
        {
            path << uniqueElement
                 << knownIdElement1
                 << unknownIdElement
                 << knownIdElement2;
        }

        WbPath        path1;
        WbPathElement uniqueElement;
        WbPathElement anotherUniqueElement;
        WbPathElement unknownIdElement;
        WbPathElement knownIdElement1;
        WbPathElement knownIdElement1WithDifferentId;
        WbPathElement unknownIdElementWithSameNameAsKnown1 ;
        WbPathElement knownIdElement2;
    };
}

TEST(WbPathTests, OutputElementsToOstream)
{
    TestSetup test;
    {
        std::ostringstream oss;

        oss << test.uniqueElement;

        EXPECT_EQ(test.uniqueElement.Name().ToQString().toStdString() , oss.str()) << "A unique-type element is represented by its name alone";
    }
    {
        std::ostringstream oss;

        oss << test.unknownIdElement;

        EXPECT_EQ(test.unknownIdElement.Name().ToQString().toStdString() + std::string( ": ????" ) , oss.str()) << "A unknownId-type element is represented by its name: ????";
    }
    {
        std::ostringstream oss;

        oss << test.knownIdElement1;

        EXPECT_EQ(test.knownIdElement1.Name().ToQString().toStdString() + std::string( ": " ) + test.knownIdElement1.KnownId().toStdString() , oss.str()) << "A knownId-type element is represented by its name: its ID";
    }
}

TEST(WbPathTests, OutputPathToOstream)
{
    TestSetup test;

    std::ostringstream expectedOss;
    expectedOss << test.path1;

    std::ostringstream actualOss;
    actualOss << test.uniqueElement << ", " << test.knownIdElement1 << ", "
              << test.unknownIdElement << ", " << test.knownIdElement2 ;


    EXPECT_EQ(expectedOss.str(), actualOss.str()) << "A is represented as its elements, separated by commas";

}

TEST(WbPathTests, ElementEquality)
{
    TestSetup test;
    EXPECT_EQ(test.uniqueElement , test.uniqueElement) << "Unique element is equal to itself";
    EXPECT_EQ(test.unknownIdElement , test.unknownIdElement) << "Unknown ID element is equal to itself";
    EXPECT_EQ(test.knownIdElement1 , test.knownIdElement1) << "Known ID element is equal to itself";
    EXPECT_NE(test.anotherUniqueElement, test.uniqueElement) << "Unique element is not equal to a different unique element";
    EXPECT_NE(test.knownIdElement2, test.knownIdElement1) << "Known-ID element is not equal to a different known-ID element (different name)";
    EXPECT_NE(test.knownIdElement1WithDifferentId, test.knownIdElement1) << "Known-ID element is not equal to a different known-ID element (same name, different ID)";
    EXPECT_NE(test.unknownIdElementWithSameNameAsKnown1, test.knownIdElement1) << "Known-ID element is not equal to an unknown-ID element with the same name";
}

TEST(WbPathTests, PathEquality)
{
    TestSetup test;
    WbPath copiedPath( test.path1 );

    EXPECT_EQ(test.path1 , copiedPath) << "Copied path is equal to the original";

    WbPath identicallyConstructedPath;
    test.InitialiseDefaultPath( identicallyConstructedPath );

    EXPECT_EQ(test.path1 , identicallyConstructedPath) << "Identically constructed path is equal to the original";

    WbPath extendedPath( test.path1 );
    extendedPath << test.uniqueElement;
    EXPECT_NE(test.path1, extendedPath) << "Copied, but then extended path is not equal to original";

    WbPath differentOrderedPath;
    differentOrderedPath << test.unknownIdElement
                         << test.uniqueElement
                         << test.knownIdElement2
                         << test.knownIdElement1;

    EXPECT_NE(test.path1, differentOrderedPath) << "Differently-ordered path is not equal to the original";

}

TEST(WbPathTests, PathFromSingleLevelWbConfig)
{
    WbSchema parentSchema( KeyName( "parentSchema" ) );
    WbConfig singleLevelConfig( parentSchema, QFileInfo() );

    WbPath pathFromSingleLevelConfig(
        WbPath::FromWbConfig( singleLevelConfig ) );
    WbPath equivalentSingleLevelPath;
    equivalentSingleLevelPath <<
        WbPathElement::Unique( parentSchema.Name() );

    EXPECT_EQ(equivalentSingleLevelPath , pathFromSingleLevelConfig) << "Single-level config creates a unique path element with the name of the schema for the config";
}

TEST(WbPathTests, PathFromMultiLevelWbConfig)
{
    WbSchema parentSchema     ( KeyName( "parentSchema" ) );
    WbSchema childSchema      ( KeyName( "childSchema" ) );
    WbSchema grandchildSchema ( KeyName( "grandchildSchema" ) );
    childSchema.AddSubSchema( grandchildSchema, WbSchemaElement::Multiplicity::One );
    parentSchema.AddSubSchema( childSchema, WbSchemaElement::Multiplicity::One );
    WbConfig multiLevelConfig( parentSchema, QFileInfo() );
    const KeyId testId( "testID" );
    WbConfig childConfig = multiLevelConfig.CreateSubConfig( childSchema.Name(),
                                                             QString(),
                                                             testId );
    WbConfig grandchildConfig =
        childConfig.CreateSubConfig( grandchildSchema.Name(), QString() );

    WbPath pathFromMultiLevelGrandchildConfig(
        WbPath::FromWbConfig( grandchildConfig ) );
    WbPath equivalentMultiLevelGrandchildPath;
    equivalentMultiLevelGrandchildPath <<
        WbPathElement::Unique( parentSchema.Name() ) <<
        WbPathElement::KnownId( childSchema.Name(), testId ) <<
        WbPathElement::Unique( grandchildSchema.Name() );

    EXPECT_EQ(equivalentMultiLevelGrandchildPath , pathFromMultiLevelGrandchildConfig) << "Multi-level grandchild config gives correct path (unique top-level, KnownId type if there is an ID, Unique if not";

    WbPath pathFromMultiLevelConfig(
                            WbPath::FromWbConfig( multiLevelConfig ) );
    WbPath equivalentMultiLevelPath;
    equivalentMultiLevelPath <<
            WbPathElement::Unique( parentSchema.Name() );

    EXPECT_EQ(equivalentMultiLevelPath , pathFromMultiLevelConfig) << "Multi-level top-level config gives correct path (just unique top-level)";
}

TEST(WbPathTests, PathFromSchema)
{
    WbSchema parentSchema( KeyName( "testSchema" ) );

    WbPath expectedSingleLevelPath;
    expectedSingleLevelPath << WbPathElement::Unique( parentSchema.Name() );

    EXPECT_EQ(expectedSingleLevelPath , parentSchema.FindPathToSchema( parentSchema )) << "Path from a schema to itself is just its own name as unique";


    WbSchema uniqueChildSchema( KeyName( "childSchema" ) );
    parentSchema.AddSubSchema( uniqueChildSchema, WbSchemaElement::Multiplicity::One );

    WbPath expectedUniqueImmediateSubschemaPath;
    expectedUniqueImmediateSubschemaPath << WbPathElement::Unique( parentSchema.Name() )
                                         << WbPathElement::Unique( uniqueChildSchema.Name() );
    EXPECT_EQ(expectedUniqueImmediateSubschemaPath , parentSchema.FindPathToSchema( uniqueChildSchema )) << "Path from a schema to a unique immediate sub-schema as expected";

    WbSchema nonUniqueChildSchema( KeyName( "nonUniqueChildSchema" ) );
    parentSchema.AddSubSchema( nonUniqueChildSchema, WbSchemaElement::Multiplicity::Many );

    WbPath expectedNonUniqueImmediateSubschemaPath;
    expectedNonUniqueImmediateSubschemaPath
                            << WbPathElement::Unique( parentSchema.Name() )
                            << WbPathElement::UnknownId( nonUniqueChildSchema.Name() );
    EXPECT_EQ(expectedNonUniqueImmediateSubschemaPath , parentSchema.FindPathToSchema( nonUniqueChildSchema )) << "Path from a schema to a non-unique immediate sub-schema as expected";
}

TEST(WbPathTests, BestFit)
{
    WbPath activeConfigPath;
    activeConfigPath << WbPathElement::Unique( KeyName( "workbench" ) )
                     << WbPathElement::Unique( KeyName( "robots" ) )
                     << WbPathElement::KnownId( KeyName( "robot" ), "Robot1" )
                     << WbPathElement::Unique( KeyName( "metrics" ) );


    WbPath desiredPath;
    desiredPath << WbPathElement::Unique( KeyName( "workbench" ) )
                << WbPathElement::Unique( KeyName( "runs" ) )
                << WbPathElement::KnownId( KeyName( "run" ), "Run1" )
                << WbPathElement::Unique( KeyName( "analysis" ) );

    WbPath expectedPath( desiredPath );

    EXPECT_EQ(activeConfigPath.BestFitWith( desiredPath ) , expectedPath) << "For a fully-specific path the best-fit is exactly the desired path";

    desiredPath = WbPath();
    desiredPath << WbPathElement::Unique( KeyName( "workbench" ) )
                << WbPathElement::Unique( KeyName( "robots" ) )
                << WbPathElement::UnknownId( KeyName( "robot" ) )
                << WbPathElement::Unique( KeyName( "trackingTarget" ) );

    expectedPath = WbPath();
    expectedPath << WbPathElement::Unique( KeyName( "workbench" ) )
                 << WbPathElement::Unique( KeyName( "robots" ) )
                 << WbPathElement::KnownId( KeyName( "robot" ), "Robot1" )
                 << WbPathElement::Unique( KeyName( "trackingTarget" ) );

    EXPECT_EQ(activeConfigPath.BestFitWith( desiredPath ) , expectedPath) << "Changing a lower-level unique element below a non-unique element known from active path obtains the expected path";

    desiredPath = WbPath();
    desiredPath << WbPathElement::Unique( KeyName( "workbench" ) )
                << WbPathElement::Unique( KeyName( "robots" ) );

    expectedPath = WbPath();
    expectedPath << WbPathElement::Unique( KeyName( "workbench" ) )
                 << WbPathElement::Unique( KeyName( "robots" ) );

    EXPECT_EQ(activeConfigPath.BestFitWith( desiredPath ) , expectedPath) << "Changing to a higher-level element with non-unique elements below stops at first non-unique";

    desiredPath = WbPath();
    desiredPath << WbPathElement::Unique( KeyName( "workbench" ) )
                << WbPathElement::Unique( KeyName( "robots" ) )
                << WbPathElement::KnownId( KeyName( "robot" ), "Robot1" );

    expectedPath = activeConfigPath;

    EXPECT_EQ(activeConfigPath.BestFitWith( desiredPath ) , expectedPath) << "Changing to a higher-level element with only unique elements below goes to lowest unique";
}

