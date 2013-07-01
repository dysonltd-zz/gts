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
#include <opencv/cv.h>
#include <vector>
#include "KeyValue.h"

namespace
{
    template< class VecType >
    bool CheckEqual( const VecType& vec1, const VecType& vec2 )
    {
        bool equal = ( vec1.size() == vec2.size() );
        for ( size_t i = 0; ( i < vec1.size() ) && equal; ++i )
        {
            equal = ( vec1.at( i ) == vec2.at( i ) );
        }
        return equal;
    }
}

TEST(KeyValuesTests, SetAndRetrieveVectorofCvPoint2f)
{
    std::vector< cv::Point2f > vec;
    vec.push_back( cv::Point2f( 3.5f, 65.1f ) );
    vec.push_back( cv::Point2f( 1234.f, 0.f ) );

    const KeyValue keyValue( KeyValue::from( vec ) );
    EXPECT_FALSE(keyValue.IsNull() ) << "We get a non-null key when we convert a std::vector of cv::Point2fs";

    std::vector< cv::Point2f > retrievedVec;
    const bool wasOk = keyValue.ToStdVectorOfCvPoint2f( retrievedVec );

    EXPECT_TRUE(wasOk ) << "KeyValue reports it's ok converting the value back";

    EXPECT_TRUE(CheckEqual( vec, retrievedVec ) ) << "The retrieved vector is the same as the original";

    const bool wasOkAgain = keyValue.ToStdVectorOfCvPoint2f( retrievedVec );
    EXPECT_TRUE(wasOkAgain ) << "KeyValue reports it's still ok if we convert back again";

    EXPECT_TRUE(CheckEqual( vec, retrievedVec ) ) << "The retrieved vector is the still same as the original";
}

