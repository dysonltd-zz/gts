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

#include "FileCapture.h"

#include "OpenCvUtility.h"

#include "Logging.h"

#include <opencv/cv.h>

#include <QtGlobal>

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include <stdio.h>

FileCapture::FileCapture( const char* filename ) :
	m_path		(""),
	m_sequence	(0),
	m_img		(0),
	m_index		(0),
	m_numFrames	(0),
	m_IsSetup (false)
{
	// load in a file name sequence
	std::ifstream file( filename );

	if ( file.is_open() )
	{
		std::string line;

		// First line of file is path
		getline( file, line );

		if ( line.length()>0 )
		{
			m_path = std::string( line.begin(), line.end() );

			// Strip off tag
			if ( m_path.find( "PATH=" ) != std::string::npos )
			{
				m_path = std::string( m_path.begin()+5 , m_path.end() );
			}
			else
			{
			    LOG_ERROR(QObject::tr("File format incorrect %1!").arg(filename));

				return;
			}

			// Strip off any end-space from path
			size_t pos = m_path.find( " " );
			if ( pos != std::string::npos )
			{
				m_path = std::string( m_path.begin(), m_path.begin() + pos );
			}
		}

		// Remaining lines contain file names then timestamps
		std::string name;
		std::string time;
		while ( !file.eof() )
		{
			getline( file, line );
			if ( line.length()>0 )
			{
				name = std::string( line.begin(), line.begin() + line.find(" ") );
				time = std::string( line.begin() + line.find(" ") + 1, line.end() );

				std::istringstream num( time );

				double t;
				num >> t;
				m_sequence.push_back( FileCapture::Frame( name, t ) );
				m_numFrames++;
			}
		}

		if ( m_sequence.size() > 0 )
		{
			m_IsSetup = true;
		}
	}
	else
	{
		LOG_ERROR(QObject::tr("Could not open file %1!").arg(filename));
	}

}

FileCapture::~FileCapture()
{
	cvReleaseImage(&m_img);
}

bool FileCapture::ReadyNextFrame()
{
	if ( m_index >= m_sequence.size() )
	{
		return false;
	}

	cvReleaseImage(&m_img);

	std::string file = m_path + m_sequence[m_index++].file;

	m_img = cvLoadImage( file.c_str() );

	if ( m_img )
	{

	}
	else
	{
	    LOG_ERROR(QObject::tr("Could not open image %1!").arg(file.c_str()));
	}

	return ( m_img != 0 );
}

bool FileCapture::ReadyNextFrame( double msec )
{
    Q_UNUSED(msec);

	return false;
}

double FileCapture::GetTimeStamp() const
{
	if(m_index>0)
	{
		return m_sequence[m_index-1].time;
	}
	else
	{
		return -1.f;
	}
}

/**
	Print all the files and timestamps in the sequence to the error stream.
**/
void FileCapture::PrintInfo() const
{
    LOG_INFO(QObject::tr("Sequence contains %1 frames.").arg(m_sequence.size()));

	for ( unsigned int i=0; i<m_sequence.size(); ++i )
	{
	    std::string file = m_path + m_sequence[i].file;

	    LOG_INFO(QObject::tr("File %1 with timestamp %2.").arg(file.c_str())
                                                          .arg(m_sequence[i].time));
	}
}
