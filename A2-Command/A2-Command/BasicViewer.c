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
#include "BasicViewer.h"

#pragma code-name("BASICVW");
#pragma rodata-name("BASICVW");

// Token list by Kelvin Sherlock
const char *tokens[]={
/*80*/	" END ",
	" FOR ",
	" NEXT ",
	" DATA ",
	" INPUT ",
	" DEL ",
	" DIM ",
	" READ ",
	" GR ",
	" TEXT ",
	" PR# ",
	" IN# ",
	" CALL ",
	" PLOT ",
	" HLIN ",
	" VLIN ",
	" HGR2 ",
	" HGR ",
	" HCOLOR= ",
	" HPLOT ",
	" DRAW ",
	" XDRAW ",
	" HTAB ",
	" HOME ",
	" ROT= ",
	" SCALE= ",
	" SHLOAD ",
	" TRACE ",
	" NOTRACE ",
	" NORMAL ",
	" INVERSE ",
	" FLASH ",
	" COLOR= ",
	" POP ",
	" VTAB ",
	" HIMEM: ",
	" LOMEM: ",
	" ONERR ",
	" RESUME ",
	" RECALL ",
	" STORE ",
	" SPEED= ",
	" LET ",
	" GOTO ",
	" RUN ",
	" IF ",
	" RESTORE ",
	" & ",
/*B0*/	" GOSUB ",
	" RETURN ",
	" REM ",
	" STOP ",
	" ON ",
	" WAIT ",
	" LIST ",
	" SAVE ",
	" DEF ",
	" POKE ",	
	" PRINT ",
	" CONT ",
	" LIST ",
	" CLEAR ",
	" GET ",
	" NEW ",
	" TAB( ",
	" TO ",
	" FN ",
	" SPC( ",
	" THEN ",
	" AT ",
	" NOT ",
	" STEP ",
	" + ",
	" - ",
	" * ",
	" / ",
	" ^ ",
	" AND ",
	" OR ",
	" > ",
	" = ",
	" < ",
	" SGN ",
	" INT ",
	" ABS ",
	" USR ",
	" FRE ",
	" SCRN( ",
	" PDL ",
	" POS ",
	" SQR ",
	" RND ",
	" LOG ",
	" EXP ",
	" COS ",
	" SIN ",
	" TAN ",
	" ATN ",
	" PEEK ",
	" LEN ",
	" STR$ ",
	" VAL ",
	" ASC ",
	" CHR$ ",
	" LEFT$ ",
	" RIGHT$ ",
	" MID$ ",
/*EB*/	" ?? ",	/* I'm pretty sure these aren't valid ... */
	" ?? ",
	" ?? ",
	" ?? ",
	" ?? ",
	" ?? ",
	" ?? ",
	" ?? ",
	" ?? ",
	" ?? ",
	" ?? ",
	" ?? ",
	" ?? ",
	" ?? ",
	" ?? ",
	" ?? ",
	" ?? ",
	" ?? ",
	" ?? ",
	" ?? ",
};       



unsigned int display(char *fileName, unsigned int lineNumber, unsigned int *previousLineNumber, unsigned int *lastLineNumber);

void viewFileAsBASIC(struct panel_drive * panel)
{
	static char fullPath[MAX_PATH_LENGTH];
	unsigned int lineNumber = 0;
	unsigned int lastLineNumber = 0;
	unsigned int previousLineNumber = 0;
	char key;

	sprintf(fullPath, "%s/%s", panel->path,
		panel->slidingWindow[panel->currentIndex - panel->slidingWindowStartAt].name);

#ifdef __APPLE2ENH__
#define UP CH_CURS_UP
#define DN CH_CURS_DOWN
#else
#define UP 'I'
#define DN 'M'
#endif

	lineNumber = display(fullPath, 0, &previousLineNumber, &lastLineNumber);

	key = 0;

	while(key != CH_ESC)
	{
		key = cgetc();
		switch(key)
		{
		case UP:
			lineNumber = display(fullPath, previousLineNumber, &previousLineNumber, &lastLineNumber);
			break;

		case DN:
			lineNumber = display(fullPath, lineNumber + 1, &previousLineNumber, &lastLineNumber);
			break;

		case HK_PAGE_DOWN:
			lineNumber = display(fullPath, lastLineNumber, &previousLineNumber, &lastLineNumber);
			break;
		}
	}

}

unsigned int display(char *fileName, unsigned int lineNumber, unsigned int *previousLineNumber, unsigned int *lastLineNumber)
{
	int file = 0;
	unsigned int current = 0;
	unsigned int lastUnused = 0;
	char curByte;

	file = open(fileName, O_RDONLY);

	clrscr();
#ifdef __APPLE2ENH__
	cputsxy(0, size_y - 1, "UP Up Line     DN Down Line                ] PgDn             ESC Back");
	revers(1);
	cputsxy(0, size_y - 1, "UP");
	cputsxy(15, size_y - 1, "DN");
	cputsxy(43, size_y - 1, "]");
	cputsxy(62, size_y - 1, "ESC");
	revers(0);
#else
	cputsxy(0, size_y - 1, "Q Up  Z Down          ] PgDn  ");
	revers(1);
	cputsxy(0, size_y - 1, "I");
	cputsxy(6, size_y - 1, "M");
	cputsxy(22, size_y - 1, "]");
	revers(0);
#endif

	gotoxy(0, 1);

	if(file > -1)
	{
		while(true)
		{
			if(read(file, fileBuffer, 2)==2)  // Address of line, discard
			{
				read(file, &current, 2);	// Current line number
				*lastLineNumber = current;

				memset(fileBuffer, 0, sizeof(fileBuffer));

				do
				{
					read(file, &curByte, 1);

					if(curByte != 0)
					{
						if(curByte > 0x7F)
						{
							strcat(fileBuffer, tokens[curByte - 0x80]);
						}
						else
						{
							fileBuffer[strlen(fileBuffer)] = curByte;
						}
					}
				} while(curByte != 0);

				if(current >= lineNumber)
				{
					if(lastUnused == 0)
					{
						lastUnused = current;
					}

					cprintf("%u ", current);
					puts(fileBuffer);
				}
				else
				{
					*previousLineNumber = current;
				}

#ifdef __APPLE2ENH__
				if(wherey() > 20) break;
#else
				if(wherey() > 18) break;
#endif
			}
			else
			{
				break;
			}
		}
	}

	close(file);

	writeStatusBarf("Lines %u through %u", lastUnused, *lastLineNumber);

	return lastUnused;
}