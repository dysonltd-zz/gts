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

#include "CalibrateCameraResultsMapper.h"

#include "ConfigKeyMapper.h"
#include "KeyName.h"
#include "CalibrationSchema.h"

#include <QtCore/QStringBuilder>

#include <opencv/cv.h>

using namespace CalibrationSchema;

const QString CalibrateCameraResultsMapper::startHeader( tr( "<p style=\"text-decoration:underline\">" ) );
const QString CalibrateCameraResultsMapper::endHeader( tr( "</p>" ) );
const QString CalibrateCameraResultsMapper::startBody( tr( "<p style=\"margin-left:1em\">" ) );
const QString CalibrateCameraResultsMapper::endBody( tr( "</p>" ) );

CalibrateCameraResultsMapper::CalibrateCameraResultsMapper( QTextBrowser& textBrowser ) :
    ConfigKeyMapper( CalibrationSchema::resultsGroup ),
    m_textBrowser( textBrowser )
{
}

void CalibrateCameraResultsMapper::CommitData( WbConfig& config )
{
    Q_UNUSED(config);
}

const QString CalibrateCameraResultsMapper::MatrixText( const WbConfig& config,
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

void CalibrateCameraResultsMapper::SetConfig( const WbConfig& config )
{
    QString resultsText;

    const bool calibrationWasSuccessful =
        config.GetKeyValue( calibrationSuccessfulKey ).ToBool();

    resultsText.append( GetSuccessOrFailureText( config, calibrationWasSuccessful ) );

    if (calibrationWasSuccessful)
    {
        resultsText.append( startHeader % tr( "Calibration Matrix" ) % endHeader );
        resultsText.append( startBody % MatrixText( config, cameraMatrixKey, 3, 3 ) % endBody );

        resultsText.append( startHeader % tr( "Distortion Coefficients" ) % endHeader );
        resultsText.append( startBody % MatrixText( config, distortionCoefficientsKey, 5, 1 ) % endBody );

        resultsText.append( startHeader % tr( "Average Reprojection Error" ) % endHeader );
        resultsText.append( startBody %
                            QString::number( config.GetKeyValue( avgReprojectionErrorKey ).ToDouble() ) %
                            endBody );

        resultsText.append( startHeader % tr( "Image" ) % endHeader );
        resultsText.append( startBody %
                            tr( "%1x%2 pixels" )
                                .arg( config.GetKeyValue( imageWidthKey ).ToInt() )
                                .arg( config.GetKeyValue( imageHeightKey ).ToInt() ) %
                            endBody );

        resultsText.append( startHeader % tr( "Calibration Grid" ) % endHeader );
        resultsText.append( startBody %
                            tr( "Rows: %1<br />" ).arg( config.GetKeyValue( rowsUsedForCalibrationKey ).ToInt() ) %
                            tr( "Columns: %1<br />" ).arg( config.GetKeyValue( columnsUsedForCalibrationKey ).ToInt() ) %
                            endBody );
    }

    m_textBrowser.setHtml( resultsText );
}

const QString
CalibrateCameraResultsMapper::GetSuccessOrFailureText( const WbConfig& config, bool successful ) const
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

