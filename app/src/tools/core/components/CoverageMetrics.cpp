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

#include "CoverageMetrics.h"

#include "OpenCvTools.h"
#include "FileUtilities.h"

#include "Logging.h"

const QColor RunEntry::colours[MAX_LEVEL] = { QColor("cyan"),
                                              QColor("magenta"),
                                              QColor("red"),
                                              QColor("green"),
                                              QColor("blue") };

namespace CoverageMetrics
{
    void PrintCsvHeaders( FILE* fp )
    {
        fprintf(fp, "Run, ");

        for (int i = 0; i < RunEntry::MAX_LEVEL; ++i)
        {
            int numberOfPasses = i + 1;
            fprintf(fp, "%d Pass", numberOfPasses);

            if (numberOfPasses > 1)
            {
                fprintf(fp, "es");
            }
            if (i < RunEntry::MAX_LEVEL-1)
            {
                fprintf(fp, ", ");
            }
            else
            {
                fprintf(fp, "+\n");
            }
        }
    }

    void PrintCsvLineForPass( FILE* fp,
                              const int run,
                              IplImage* totalCoverageImg,
                              const int nFloorPixels )
    {
        fprintf( fp, "%d", run);

        for (int level = 0; level < RunEntry::MAX_LEVEL; ++level)
        {
            const int numTimesCovered = level+1;
            const bool isTopNumPasses = (level == (RunEntry::MAX_LEVEL-1));
            int comparisonOp = CV_CMP_EQ;

            if ( isTopNumPasses )
            {
                comparisonOp = CV_CMP_GE;
            }

            int nPixels = OpenCvTools::GetPixelCoverageCount( totalCoverageImg,
                                                              numTimesCovered,
                                                              comparisonOp );
            float percent = nPixels * (100.f / nFloorPixels);
            fprintf(fp, ", %f", percent);

            LOG_INFO(QObject::tr("%1 pixels (%2%%) were covered %3 time(s).").arg(nPixels)
                                                                             .arg(percent)
                                                                             .arg(numTimesCovered));
        }

        fprintf(fp, "\n");
    }

    bool ReadCsv( const char* filename, RunMetrics& metrics )
    {
        FILE* fp = fopen( filename, "r" );

        if (fp)
        {
            metrics.clear();

            // Skip headers
            FileUtilities::LineSkip(fp);

            while (!feof(fp))
            {
                int run;
                int cnt = 0;
			    RunEntry entry;

                fscanf( fp, "%d", &run);

                for (int i=0; i<RunEntry::MAX_LEVEL-1; i++)
				{
                    cnt += fscanf( fp, ", %f", &entry.level[i] );
				}

				cnt += fscanf( fp, ", %f\n", &entry.level[RunEntry::MAX_LEVEL-1]);

				if ( cnt == RunEntry::MAX_LEVEL )
                {
                    metrics.push_back( entry );
                }
				else
				{
                    LOG_ERROR(QObject::tr("Unexpected data (%1) in %2!").arg(cnt).arg(filename));
				}
            }

            fclose(fp);
        }
        else
        {
            LOG_ERROR(QObject::tr("Unable to read coverage metrics file: %1!")
                         .arg(filename));

            return false;
        }

        return true;
    }
}
