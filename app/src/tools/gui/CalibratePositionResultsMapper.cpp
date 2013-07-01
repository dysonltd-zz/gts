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

#include "CalibratePositionResultsMapper.h"

#include "ConfigKeyMapper.h"
#include "KeyName.h"
#include "ExtrinsicCalibrationSchema.h"

#include <QtCore/qstringbuilder.h>

#include <opencv/cv.h>

using namespace ExtrinsicCalibrationSchema;

const QString CalibratePositionResultsMapper::startHeader( tr( "<p style=\"text-decoration:underline\">" ) );
const QString CalibratePositionResultsMapper::endHeader( tr( "</p>" ) );
const QString CalibratePositionResultsMapper::startBody( tr( "<p style=\"margin-left:1em\">" ) );
const QString CalibratePositionResultsMapper::endBody( tr( "</p>" ) );

CalibratePositionResultsMapper::CalibratePositionResultsMapper( QTextBrowser& textBrowser ) :
    ConfigKeyMapper( ExtrinsicCalibrationSchema::resultsGroup ),
    m_textBrowser( textBrowser )
{
}

void CalibratePositionResultsMapper::CommitData( WbConfig& config )
{
    Q_UNUSED(config);
}

const QString CalibratePositionResultsMapper::MatrixText( const WbConfig& config,
                                                        const KeyName keyName,
                                                        const int rows,
                                                        const int columns )
{
    QString matrixText( tr( "<Unknown>" ) );
    cv::Mat matrix( rows, columns, CV_64FC1 );
    const bool success = config.GetKeyValue( keyName ).TocvMat( matrix );
    if ( success )
    {
        matrixText = "<table border=\"1\" cellpadding=\"2\" cellspacing=\"0\">\n";
        for ( int r = 0; r < rows; ++r )
        {
            matrixText.append( "<tr>\n" );
            for ( int c = 0; c < columns; ++c )
            {
                const double thisElement = matrix.at<double>( r, c );
                matrixText.append( QString( "<td align=\"right\">%1</td>\n" ).arg( thisElement ) );
            }
            matrixText.append( "</tr>\n" );
        }
        matrixText.append( "</table>\n" );
    }
    return matrixText;
}

void CalibratePositionResultsMapper::SetConfig( const WbConfig& config )
{
    QString resultsText;

    const bool calibrationWasSuccessful =
        config.GetKeyValue( calibrationSuccessfulKey ).ToBool();

    resultsText.append( GetSuccessOrFailureText( config, calibrationWasSuccessful ) );

    if (calibrationWasSuccessful)
    {
        resultsText.append( startHeader % tr( "Rotation Matrix" ) % endHeader );
        resultsText.append( startBody % MatrixText( config, rotationMatrixKey, 3, 3 ) % endBody );

        resultsText.append( startHeader % tr( "Translation Coefficients" ) % endHeader );
        resultsText.append( startBody % MatrixText( config, translationKey, 3, 1 ) % endBody );

        resultsText.append( startHeader % tr( "Square Size" ) % endHeader );
        resultsText.append( startBody %
                            tr( "%1 pixels" )
                                .arg( config.GetKeyValue( gridSquareSizeInPxKey ).ToDouble() ) %
                            endBody );
    }

    m_textBrowser.setHtml( resultsText );
}

const QString
CalibratePositionResultsMapper::GetSuccessOrFailureText( const WbConfig& config, bool successful ) const
{
    QString successOrFailureText;

    if ( successful )
    {
        const QString calibDate( config.GetKeyValue( calibrationDateKey ).ToQString() );
        const QString calibTime( config.GetKeyValue( calibrationTimeKey ).ToQString() );

        successOrFailureText = startHeader %
                               tr( "<strong>Calibrated</strong>" ) %
                               endHeader %
                               startBody %
                               tr( "On: %1, at: %2." ).arg( calibDate ).arg( calibTime ) %
                               endBody;
    }
    else
    {
        successOrFailureText = startHeader %
                               tr( "<strong>Uncalibrated</strong>" ) %
                               endHeader;
    }

    return successOrFailureText;
}

