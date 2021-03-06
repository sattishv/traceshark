// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015-2017  Viktor Rosendahl <viktor.rosendahl@gmail.com>
 *
 * This file is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 *
 *  a) This program is free software; you can redistribute it and/or
 *     modify it under the terms of the GNU General Public License as
 *     published by the Free Software Foundation; either version 2 of the
 *     License, or (at your option) any later version.
 *
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public
 *     License along with this library; if not, write to the Free
 *     Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 *     MA 02110-1301 USA
 *
 * Alternatively,
 *
 *  b) Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *     1. Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *     2. Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 *     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 *     CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 *     INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *     MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *     DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 *     CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *     SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *     NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *     LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *     HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *     CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 *     OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 *     EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <cstring>
#include "mm/stringpool.h"

#define MAX(A, B) ((A) >= (B) ? A:B)
#define MIN(A, B) ((A) < (B) ? A:B)


StringPool::StringPool(unsigned int nr_pages, unsigned int hSizeP)
{
	unsigned int entryPages, strPages;

	if (hSizeP == 0)
		hSize = 1;
	else
		hSize = hSizeP;

	entryPages = 2 * hSize *
		sizeof(vtl::AVLNode<TString, __DummySP>) / 4096;
	entryPages = MAX(1, entryPages);
	strPages = 2* hSize * sizeof(TString) / 4096;
	strPages = MAX(16, strPages);

	coldCharPool = new MemPool(nr_pages, 1);
	strPool = new MemPool(nr_pages, sizeof(TString));

	avlPools.charPool = new MemPool(nr_pages, sizeof(char));
	avlPools.nodePool = new MemPool(entryPages, sizeof(vtl::AVLNode<TString,
							   __DummySP>));
	hashTable = new StringPoolEntry*[hSize];
	countAllocs = new unsigned int[hSize];
	countReuse = new unsigned int[hSize];
	clearTable();
}

StringPool::~StringPool()
{
	unsigned int i, s;
	delete coldCharPool;
	delete strPool;
	delete avlPools.charPool;
	delete avlPools.nodePool;
	delete[] hashTable;
	delete[] countAllocs;
	delete[] countReuse;
	s = deleteList.size();
	for (i = 0; i < s; i++) {
		delete deleteList[i];
	}
}

void StringPool::clearTable()
{
	bzero(hashTable, hSize * sizeof(StringPoolEntry*));
	bzero(countAllocs, hSize * sizeof(unsigned int));
	bzero(countReuse, hSize * sizeof(unsigned int));
}

void StringPool::clear()
{
	unsigned int s, i;
	clearTable();
	coldCharPool->reset();
	strPool->reset();
	avlPools.nodePool->reset();
	avlPools.charPool->reset();
	s = deleteList.size();
	for (i = 0; i < s; i++) {
		delete deleteList[i];
	}
	deleteList.clear();
}

void StringPool::reset()
{
	clear();
}
