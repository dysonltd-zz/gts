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

#ifndef FILE_SEQ_H
#define FILE_SEQ_H

/**
	class for managing file-name sequences
**/
class FileSeq
{
public:
	FileSeq( const char* prefix, const char* postfix );
	~FileSeq();
	
	const char* Make( unsigned int index );
	const char* Get() const { return m_name; };

private:
	char* m_pre;
	char* m_post;
	char* m_name;
};

#endif // FILE_SEQ_H
