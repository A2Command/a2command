/**************************************************************
Copyright (c) 2010-2013, Payton Byrd
All rights reserved.

Redistribution and use in source and binary forms, with or 
without modification, are permitted provided that the following 
conditions are met:

* Redistributions of source code must retain the above 
copyright notice, this list of conditions and the following 
disclaimer.

* Redistributions in binary form must reproduce the above 
copyright notice, this list of conditions and the following 
disclaimer in the documentation and/or other materials 
provided with the distribution.

* Neither the name of A2-Command Team nor the names of its 
contributors may be used to endorse or promote products 
derived from this software without specific prior written 
permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, 
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***************************************************************/

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <errno.h>
#include <peekpoke.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <fcntl.h>

#include "HexEditor.h"
#include "constants.h"
#include "globalInput.h"
#include "globals.h"
#include "drives.h"
#include "input.h"
#include "menus.h"
#include "screen.h"


#pragma code-name("HEXEDIT");
#pragma rodata-name("HEXEDIT");


void hexEditCurrentFile(struct panel_drive *panel)
{
	static char fullPath[MAX_PATH_LENGTH];
	int bytesPerLine;
	off_t offset = 0;
	char key;
	static char input[15];
	static char *message[] = 
	{
		{ "Enter Offset" }
	};

	sprintf(fullPath, "%s/%s", panel->path,
		panel->slidingWindow[panel->currentIndex - panel->slidingWindowStartAt].name);

#ifdef __APPLE2ENH__
	bytesPerLine = 16;
#define UP CH_CURS_UP
#define DN CH_CURS_DOWN
#else
	bytesPerLine = 8;
#define UP 'I'
#define DN 'M'
#endif

	display(bytesPerLine, fullPath, offset);

	key = 0;

	while(key != CH_ESC)
	{
		key = cgetc();
		switch(key)
		{
		case UP:
			if(offset > 0)
			{
				offset -= bytesPerLine;
				if(offset < 0) offset = 0;
				display(bytesPerLine, fullPath, offset);
			}
			break;

		case DN:
			offset += bytesPerLine;
			display(bytesPerLine, fullPath, offset);
			break;

		case HK_PAGE_UP:
			if(offset > 0)
			{
				offset -= bytesPerLine * (size_y - 2);
				if(offset < 0) offset = 0;
				display(bytesPerLine, fullPath, offset);
			}
			break;

		case HK_PAGE_DOWN:
			offset += bytesPerLine * (size_y - 2);
			display(bytesPerLine, fullPath, offset);
			break;

		case 'G':
		case 'g':
			drawInputDialog(1, 14, message, "Goto Position", input);
			
			offset = atol(input);
			display(bytesPerLine, fullPath, offset);

			break;
		}
	}
}

void display(int bytesPerLine, char *fileName, off_t startAt)
{
	int file = -1;
	char x, y;
	int bytesRead;
	off_t t;

#ifdef __APPLE2ENH__
	static char readBuffer[16];
#else
	static char readBuffer[8];
#endif

	clrscr();
#ifdef __APPLE2ENH__
	cputsxy(0, size_y - 1, "UP Up Line     DN Down Line     [ PgUp     ] PgDn    G Goto   ESC Back");
	revers(1);
	cputsxy(0, size_y - 1, "UP");
	cputsxy(15, size_y - 1, "DN");
	cputsxy(32, size_y - 1, "[");
	cputsxy(43, size_y - 1, "]");
	cputsxy(53, size_y - 1, "G");
	cputsxy(62, size_y - 1, "ESC");
	revers(0);
#else
	cputsxy(0, size_y - 1, "Q Up  Z Down  [ PgUP  ] PgDn  G Goto");
	revers(1);
	cputsxy(0, size_y - 1, "I");
	cputsxy(6, size_y - 1, "M");
	cputsxy(14, size_y - 1, "[");
	cputsxy(22, size_y - 1, "]");
	cputsxy(30, size_y - 1, "G");
	revers(0);
#endif
	writeStatusBarf("Reading from %lu", startAt);

	file = open(fileName, O_RDWR);

	if(file != -1)
	{
#if 0
		lseek(file, startAt, SEEK_SET);
#else
		t = startAt;
		while(t >= sizeof(fileBuffer))
		{
			read(file, fileBuffer, sizeof(fileBuffer));
			t -= sizeof(fileBuffer);
		}
		read(file, fileBuffer, t);
#endif

		for(y = 1; y < size_y - 1; ++y)
		{
			if((bytesRead = read(file, readBuffer, bytesPerLine)) < bytesPerLine)
			{
				processLine(readBuffer, startAt + ((y - 1) * bytesPerLine), bytesRead, y);
				break;
			}
			else
			{
				processLine(readBuffer, startAt + ((y - 1) * bytesPerLine), bytesPerLine, y);
			}
		}
	}

	close(file);
}

void processLine(char *readBuffer, off_t at, int bytesRead, char y)
{
	char i, c;
	int athi, atlo;
	gotoxy(0, y);
	athi = (int)((at & 0xFFFF0000)>>16);
	atlo = (int)(at & 0xFFFF);

#ifdef __APPLE2ENH__
	cprintf("%04X%04X", athi, atlo);
	gotoxy(10, y);
#else
	cprintf("%03X%04X", athi, atlo);
	gotoxy(8, y);
#endif
	for(i = 0; i < bytesRead; ++i)
	{
		c = readBuffer[i];
		cprintf("%02X ", c);

		if(i == 7) cputc(' ');
	}
	
#ifdef __APPLE2ENH__
	gotoxy(63, y);
#else
	gotoxy(32, y);
#endif

	for(i = 0; i < bytesRead; ++i)
	{
		c = readBuffer[i];
		if(c > 31 && c < 127) 
			cputc(c);
		else
			cputc('.');

		if(i == 7) cputc(' ');
	}
}