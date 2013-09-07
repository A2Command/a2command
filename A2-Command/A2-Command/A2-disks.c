/**************************************************************
Copyright (c) 2010, Payton Byrd
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
#ifdef __APPLE2ENH__
#include <apple2enh.h>
#else
#include <apple2.h>
#endif

#include <conio.h>
#include <dio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <device.h>

#include "A2-disks.h"
#include "screen.h"
#include "globals.h"
#include "input.h"
#include "globalInput.h"
#include "drives.h"
#include "menus.h"

unsigned char _driveCount;
unsigned char _devices[14];

void __fastcall__ defaultDrive(struct panel_drive *panel)
{
	unsigned char dev;
	
	if(!getcwd(panel->path, sizeof(panel->path)))
	{
		strcpy(panel->path, "");
	}
	
	for(dev = getfirstdevice(); dev != INVALID_DEVICE; dev = getnextdevice(dev))
	{
		if(getdevicedir(dev, filePath, sizeof(filePath)))
		{
			if(!strcmp(filePath, panel->path))
			{
				panel->drive = dev;
				break;
			}
		}
	}
}

void __fastcall__ selectDrive(struct panel_drive *panel)
{
	unsigned char dev, key, current, x = 5, y = 2;
	static unsigned char temp[80];

	saveScreen();
	drawBox(x, y, size_x - 10, size_y - 5, COLOR_WHITE, 0);

	cputsxy(7, y + 2, "Select directory for panel.");

    _driveCount = 0;
    for(dev = getfirstdevice(); dev != INVALID_DEVICE; dev = getnextdevice(dev))
	{
		if(!getdevicedir(dev, filePath, sizeof(filePath)))
		{
			strcpy(filePath, _stroserror(_oserror));
		}
#ifdef __APPLE2ENH__
        sprintf(temp, "Slot %u Drive %u - %s", dev & 7, (dev>>3)+1, filePath);
#else
		sprintf(temp, "S%uD%u - %.20s", dev & 7, (dev>>3)+1, filePath);
#endif		
		cputsxy(8, y + 4 + _driveCount, temp);
        _devices[_driveCount++] = dev;
	}

	cputcxy(7, y + 4, '>');

	key = 0;
	current = 0;
	while(key != CH_ENTER && key != CH_ESC)
	{
		key = cgetc();

		switch(key)
		{
#ifdef __APPLE2ENH__
		case CH_CURS_DOWN:
#else
		case CH_CURS_RIGHT:
#endif
			if(current < _driveCount - 1)
			{
				cputcxy(7, y + 4 + current, ' ');
				cputcxy(7, y + 4 + (++current), '>');
			}
			else
			{
				cputcxy(7, y + 4 + current, ' ');
				cputcxy(7, y + 4, '>');
				current = 0;
			}
			break;

#ifdef __APPLE2ENH__
			case CH_CURS_UP:
#else
			case CH_CURS_LEFT:
#endif
			if(current > 0)
			{
				cputcxy(7, y + 4 + current, ' ');
				cputcxy(7, y + 4 + (--current), '>');
			}
			else
			{
				current = _driveCount - 1;
				cputcxy(7, y + 4, ' ');
				cputcxy(7, y + 4 + current, '>');
			}
			break;
		}
	}

	retrieveScreen();

	if(key == CH_ENTER)
	{
		panel->drive = _devices[current];
		if(!getdevicedir(panel->drive, panel->path, sizeof(panel->path)))
		{
			strcpy(panel->path, "");
		}
	}
}

unsigned long __fastcall__ getDriveBlocks(unsigned char driveNumber)
{
	static dhandle_t drive;
	static unsigned int sectorCount;
	
	drive = dio_open(driveNumber);
	sectorCount = dio_query_sectcount(drive);
	dio_close(drive);
	
	return sectorCount;
}


unsigned char _fileTypes[256][4] = {
"UNK", "BAD", "PCD", "PTX", "TXT", "PDA", "BIN", "FNT", "FOT", "BA3", "DA3", "WPF", "SOS", ""   , ""   , "DIR",
"RPD", "RPI", "AFD", "AFM", "AFR", "SCL", "PFS", ""   , ""   , "ADB", "AWP", "ASP", ""   , ""   , ""   , ""   ,
"TDM", "IPS", "UPV", ""   , ""   , ""   , ""   , ""   , ""   , "3SD", "8SC", "8OB", "8IC", "8LD", "P8C", ""   ,
""   , ""   , ""   , ""   , ""   , ""   , ""   , ""   , ""   , ""   , ""   , ""   , ""   , ""   , ""   , ""   ,
""   , "OCR", "FTD", ""   , ""   , ""   , ""   , ""   , ""   , ""   , ""   , ""   , ""   , ""   , ""   , ""   ,
"GWP", "GSS", "GDB", "DRW", "GDP", "HMD", "EDU", "STN", "HLP", "COM", "CFG", "ANM", "MUM", "ENT", "DVU", ""   ,
"PRE", ""   , ""   , ""   , ""   , ""   , ""   , ""   , ""   , ""   , ""   , "BIO", "NCF", "DVR", "PRE", "HDV",
"SN2", "KMT", "DSR", "BAN", "CG7", "TNJ", "SA7", "KES", "JAP", "CSL", "TME", "TLB", "MR7", "MLR", "MMM", "JCP",
"GES", "GEA", "GEO", "GED", "GEF", "GEP", "GEI", "GEX", ""   , "GEV", ""   , "GEC", "GEK", "GEW", ""   , ""   ,
""   , ""   , ""   , ""   , ""   , ""   , ""   , ""   , ""   , ""   , ""   , ""   , ""   , ""   , ""   , ""   ,
"WP ", ""   , ""   , ""   , ""   , ""   , ""   , ""   , ""   , ""   , ""   , "GSB", "TDF", "BDF", ""   , ""   ,
"SRC", "OBJ", "LIB", "S16", "RTL", "EXE", "PIF", "TIF", "NDA", "CDA", "TOL", "DRV", "LDF", "FST", ""   , "DOC",
"PNT", "PIC", "ANI", "PAL", ""   , "OOG", "SCR", "CDV", "FON", "FND", "ICN", ""   , ""   , ""   , ""   , ""   ,
""   , ""   , ""   , ""   , ""   , "MUS", "INS", "MDI", "SND", ""   , ""   , "DBM", ""   , ""   , ""   , ""   ,
"LBR", ""   , "ATK", ""   , ""   , ""   , ""   , ""   , ""   , ""   , ""   , ""   , ""   , ""   , "R16", "PAR",
"CMD", "OVL", "UD2", "UD3", "UD4", "BAT", "UD6", "UD7", "PRG", "P16", "INT", "IVR", "BAS", "VAR", "REL", "SYS"};

void setupFileTypes(void)
{
	int i;

	for(i = 0; i<=0xFF; ++i)
	{
		if(!_fileTypes[i][0])
		{
			sprintf(_fileTypes[i], "$%2X", i);
		}
	}
}