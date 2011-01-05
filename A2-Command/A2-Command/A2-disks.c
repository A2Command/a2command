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
#include <apple2enh.h>
#include <conio.h>

#include "A2-disks.h"
#include "screen.h"
#include "globals.h"
#include "input.h"
#include "globalInput.h"

unsigned char _driveCount;
unsigned char* _drives;

void selectDrive(struct panel_drive *panel)
{
	unsigned char i, key, current;
	unsigned char buffer[68];

	_driveCount = drivecount();
	_drives = drivelist();

	saveScreen();
	drawBox(5, 5, size_x - 10, size_y - 10, COLOR_WHITE, 0);

	cputsxy(7, 7, "Select directory for panel.");

	for(i=0; i<_driveCount; ++i)
	{
		rootdir(_drives[i], buffer);
		cputsxy(8, 9 + i, buffer); 
	}

	cputcxy(7, 9, '>');

	key = 0;
	current = 0;
	while(key != CH_ENTER && key != CH_ESC)
	{
		key = cgetc();

		switch(key)
		{
		case CH_CURS_DOWN:
			if(current < _driveCount - 1)
			{
				cputcxy(7, 9 + current, ' ');
				cputcxy(7, 9 + (++current), '>');
			}
			break;

		case CH_CURS_UP:
			if(current > 0)
			{
				cputcxy(7, 9 + current, ' ');
				cputcxy(7, 9 + (--current), '>');
			}
			break;
		}
	}

	if(key == CH_ENTER)
	{
		rootdir(_drives[current], panel->path);

		panel->drive->drive = current;		

		selectedPanel = panel;
	}

	retrieveScreen();
}
