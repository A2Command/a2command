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

#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <device.h>

#include "A2-disks.h"
#include "constants.h"
#include "drives.h"
#include "globals.h"
#include "globalInput.h"
#include "menus.h"
#include "screen.h"

char __fastcall__ rereadDrivePanel(enum menus menu)
{
	if(menu == left)
	{
		selectedPanel = &leftPanelDrive;
	}
	else
	{
		selectedPanel = &rightPanelDrive;
	}

	return rereadSelectedPanel();
}


char __fastcall__ rereadSelectedPanel(void)
{
	if(selectedPanel == NULL)
	{
		selectedPanel = &leftPanelDrive;
	}

	selectedPanel->currentIndex = 0;
	selectedPanel->displayStartAt = 0;
	
	if(getDirectory(selectedPanel, 0) == -1)
	{
		if(!getdevicedir(selectedPanel->drive, selectedPanel->path, sizeof(selectedPanel->path)))
		{
			strcpy(selectedPanel->path, "");
		}
		else
		{
			getDirectory(selectedPanel, 0);
		}
	}
	
	selectedPanel->totalblocks = getDriveBlocks(selectedPanel->drive);	
	resetSelectedFiles(selectedPanel);
	displayDirectory(selectedPanel);
	
	if(selectedPanel == &leftPanelDrive)
	{
		writeSelectorPosition(&leftPanelDrive, '>');
		writeSelectorPosition(&rightPanelDrive, ' ');
	}
	else
	{
		writeSelectorPosition(&leftPanelDrive, ' ');
		writeSelectorPosition(&rightPanelDrive, '>');
	}
	
	writeCurrentFilename(selectedPanel);
	
}

void __fastcall__ writeDriveSelectionPanel(enum menus menu)
{
	selectDrive((menu == left ? &leftPanelDrive : &rightPanelDrive));
	retrieveScreen();

	rereadDrivePanel(menu);

	if(menu == left)
	{
		selectedPanel = &leftPanelDrive;
	}
	else
	{
		selectedPanel = &rightPanelDrive;
	}
}