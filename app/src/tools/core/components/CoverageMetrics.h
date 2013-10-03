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

#ifndef  COVERAGEMETRICS_H
#define  COVERAGEMETRICS_H

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include <QColor>

#include <fstream>
#include <vector>

class RunEntry
{
public:
    static const int MAX_LEVEL = 5;
    static const QColor colours[MAX_LEVEL];
    static const int GetMaxLevel() { return MAX_LEVEL; }; // < trick the buggy compiler

public:
    RunEntry() {};
    ~RunEntry() {};

    float level[MAX_LEVEL];
};

namespace CoverageMetrics
{
	typedef std::vector<RunEntry> RunMetrics;

    void PrintCsvHeaders( FILE* fp );
    void PrintCsvLineForPass( FILE* fp,
                              const int run,
                              IplImage* totalCoverageImg,
                              const int nFloorPixels );

    bool ReadCsv( const char* filename, RunMetrics& metrics );
}

#endif // COVERAGEMETRICS_H
