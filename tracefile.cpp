/*
 * Traceshark - a visualizer for visualizing ftrace traces
 * Copyright (C) 2014-2015  Viktor Rosendahl
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "tracefile.h"
#include "traceline.h"
#include "mempool.h"
#include <QtGlobal>
#include <new>

extern "C" {
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
};

ssize_t TraceFile::Read(int fd, void *buf, size_t count)
{
	return read(fd, buf, count);
}

TraceFile::TraceFile(char *name, bool &ok, quint32 bsize)
{
	ssize_t n;
	fd = open(name, O_RDONLY);
	if (fd >= 0)
	        ok = true;
	else
		ok = false;
	lastPos = 0;
	lastBuf = 0;
	eof = false;
	strPool = new MemPool(8*1024*1024, 1);
	ptrPool = new MemPool(1024*1024, sizeof(char *));
	bufSize = bsize;
	memory = new char[2*bsize];
	buffer[0] = memory;
	buffer[1] = memory + bsize;
	n = Read(fd, buffer[0], bsize);
	if (n <= 0)
		eof = true;
	nRead = n;
}

quint32 TraceFile::ReadLine(TraceLine* line)
{
	char **strings;
	quint32 col;
	char *word;
	quint32 n;

	strings = (char**) ptrPool->PreAllocN(MAXPTR);
	Q_ASSERT(strings != NULL);
	line->strings = strings;

	for(col = 0; col < MAXPTR; col++) {
		word = (char*) strPool->PreAllocChars(MAXSTR);
		n = ReadNextWord(word, MAXSTR);
		if (n == 0)
			break;
		strPool->CommitChars(n + 1 ); // + 1 for null termination
		strings[col] = word;
	}
	if (col > 0)
		ptrPool->CommitN(col);
	line->nStrings = col;
	return col;
}

TraceFile::~TraceFile()
{
	delete[] memory;
	delete strPool;
	delete ptrPool;
}

bool TraceFile::atEnd()
{
	return eof;
}