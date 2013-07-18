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

#ifndef COVERAGESYSTEM_H
#define COVERAGESYSTEM_H

#include <opencv/cv.h>
#include <stdio.h>

class RoboTrackKlt; // forward declaration

/**
	Class for managing recording and computation of coverage.
**/
class CoverageSystem
{
public:
	CoverageSystem(CvSize warpedImageSize);
	~CoverageSystem();

	//void Update( const RoboTrackKlt& tracker );
	void DirectUpdate( CvPoint pb, float robotRadiusPx );
	void Update( CvPoint2D32f prev, CvPoint2D32f curr, float robotRadiusPx );
	void BrushBarUpdate(
			CvPoint2D32f pl, CvPoint2D32f pr,
			CvPoint2D32f cl, CvPoint2D32f cr
		);


	void DrawMask( IplImage* img, CvScalar colour ) const;
	void SaveMask( const char* filename );
	void DrawMap( IplImage* img ) const;

	unsigned int GetCoveredPixelCount() const;
	float EstimateCoverage( const IplImage* floormask ) const;
	float EstimateCoverage() const;
	float EstimateRepeatCoverage() const;
	void CoverageHistogram( const char* file_name ) const;

	bool LoadFloorMask( const char* filename );
	void SetFloorMask( const IplImage* mask );
	const IplImage* GetFloorMask() const { return m_floorMask; };

	static unsigned int CountWhitePixels( const IplImage* floormask );
	static unsigned int CountRepeatCoverage( const IplImage* mask );

	void IncrementUncoveredPixels();
	void CreateColouredMap();


    void WriteIncrementalCoverage( FILE* fp, unsigned int count = 1 );
    int MissedMask( const char* fileName );

private:

	IplImage* m_cvgMask;
	IplImage* m_floorMask;

	unsigned int m_floorPixels;

	IplImage* m_inOutMask;
	IplImage* m_colMap;
};

#endif // COVERAGESYSTEM_H
