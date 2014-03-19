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

#ifndef FILE_CAPTURE_H
#define FILE_CAPTURE_H

#include "FileSeq.h"
#include "VideoSequence.h"

#include <opencv/highgui.h>

#include <vector>
#include <string>

#include <QtGlobal>

/**
	Class for loading and accessing video stored in a sequence of files.

	Inherits from the abstract class VideoSequence in order that file sequences,
	single video files, and live video can be treated in exactly the same.
**/
class FileCapture : public VideoSequence
{
public:
	FileCapture( const char* filenameBase );
	~FileCapture();

	virtual bool IsRewindable()	 const  { return true; };
	virtual bool IsForwardable() const  { return true; };
	virtual bool IsWindable()	 const  { return true; };
	virtual bool IsLive()		 const  { return false;  };

	virtual bool ReadyNextFrame();
	virtual bool ReadyNextFrame( double msec );

	virtual const IplImage* RetrieveNextFrame() const { return m_img; };

	virtual double GetTimeStamp()	const;
	virtual double GetFrameIndex()	const { return (double)(m_index-1); };
	virtual double GetNumFrames()	const { return m_numFrames; };

	virtual int GetFrameWidth() const  { return (int)m_img->width; };
	virtual int GetFrameHeight() const  { return (int)m_img->height; };

    virtual void SetFrameRate( const double fps ) { Q_UNUSED(fps); };

    virtual double GetFrameRate() { return 0.0; };

	virtual bool IsSetup() const { return m_IsSetup; };
	virtual int Flip() const { return 0; };

	virtual void PrintInfo() const;

	virtual void ReadyFrame() {};
	virtual bool TakeFrame() { return true; };

private:

	struct Frame
	{
		Frame() {};
		Frame(const std::string& s, double t) : file(s), time(t) {};
		const Frame& operator = (const Frame& f) { file=f.file; time=f.time; return *this; };
		std::string file;
		double time;
	};

	std::string			m_path;
	std::vector<Frame>	m_sequence;
	IplImage*			m_img;
	unsigned int		m_index;
	unsigned int		m_numFrames;
	bool				m_IsSetup;
};

#endif // FILE_CAPTURE_H
