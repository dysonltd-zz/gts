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

#include "FileSeq.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

FileSeq::FileSeq(const char* pre, const char* post) :
	m_pre	(0),
	m_post	(0),
	m_name	(0)
{
	m_pre = new char[ strlen(pre)+1 ];
	strncpy(m_pre, pre, strlen(pre)+1);

	m_post = new char[ strlen(post)+1 ];
	strncpy(m_post, post, strlen(post)+1);

	m_name = new char[ strlen(pre) + strlen(post) + 17 ];
	Make(0);
}

FileSeq::~FileSeq()
{
	delete [] m_pre;
	delete [] m_post;
	delete [] m_name;
}

/**
	Make the file name for a specified index in the sequence.
	The string containing the file name is stored internally
	and a pointer to it returned. It can be accessed further
	with FileSeq::Get().
**/
const char* FileSeq::Make(unsigned int index)
{
	sprintf(m_name, "%s%.4d%s", m_pre, index, m_post);
	return m_name;
}
