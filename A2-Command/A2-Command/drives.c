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
#include <errno.h>
#include <dirent.h>
#include <peekpoke.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "A2-disks.h"
#include "constants.h"
#include "drives.h"
#include "globalInput.h"
#include "globals.h"
#include "input.h"
#include "screen.h"
#include "menus.h"

#ifndef __fastcall
#define __fastcall __fastcall
#endif

unsigned char commandPath[256];
unsigned char fileBuffer[COPY_BUFFER_SIZE];

struct drive_status drives[9] =
{
	{ 0 },	// 0
	{ 0 },	// 1
	{ 0 },	// 2
	{ 0 },	// 3
	{ 0 },	// 4
	{ 0 },	// 5
	{ 0 },	// 6
	{ 0 },	// 7
	{ 0 }	// 8
};

unsigned areDrivesInitialized = false;
struct panel_drive leftPanelDrive; 
struct panel_drive rightPanelDrive;
struct panel_drive *selectedPanel;
struct panel_drive *targetPanel;

void initializeDrives(void)
{
	static unsigned char i = 0;

	if(!areDrivesInitialized)
	{
		leftPanelDrive.drive = NULL;
		leftPanelDrive.currentIndex = 0;
		leftPanelDrive.displayStartAt = 0;
		leftPanelDrive.position = left;
		
		for(i=0; i<SLIDING_WINDOW_SIZE; ++i)
		{
			leftPanelDrive.slidingWindow[i].size = 0u;
			leftPanelDrive.slidingWindow[i].type = 0;
		}

		rightPanelDrive.drive = NULL;
		rightPanelDrive.currentIndex = 0;
		rightPanelDrive.displayStartAt = 0;
		rightPanelDrive.position = right;
		
		for(i=0; i<SLIDING_WINDOW_SIZE; ++i)
		{
			rightPanelDrive.slidingWindow[i].size = 0u;
			rightPanelDrive.slidingWindow[i].type = 0;
		}

		areDrivesInitialized = true;
		selectedPanel = &leftPanelDrive;
	}
}

int __fastcall  getDirectory(
	struct panel_drive *drive,
	const int slidingWindowStartAt)
{
	unsigned int counter=0, read=0;
	unsigned char i;
	struct dirent *currentDE;
	DIR *dir = NULL;

	drive->length = 0;
	drive->slidingWindowStartAt = slidingWindowStartAt;

	for(i=0; i<SLIDING_WINDOW_SIZE; ++i)
	{
		drive->slidingWindow[i].size = 0u;
		drive->slidingWindow[i].type = 0;
	}

	if(strlen(drive->path) == 0)
	{
		writeStatusBar("Aborting.");
		return 0;
	}
	else
	{
		dir = opendir(drive->path);

		if(dir != NULL)
		{
			writeStatusBar("Reading directory...");
			counter = 0;
			read = 0;
			currentDE = readdir(dir);
			if(strlen(currentDE->d_name) > 0)
			{
				for(i = 0; i<SLIDING_WINDOW_SIZE; ++i)
				{
					strcpy(drive->slidingWindow[i - slidingWindowStartAt].name, "");
				}

				while(currentDE != NULL)
				{
					if(counter >= slidingWindowStartAt &&
						read <= SLIDING_WINDOW_SIZE)
					{
						++read;
						i = counter;
						if(i - slidingWindowStartAt >= 0 && 
							i - slidingWindowStartAt <= SLIDING_WINDOW_SIZE)
						{
							strcpy(drive->slidingWindow[i - slidingWindowStartAt].name, currentDE->d_name);
							drive->slidingWindow[i - slidingWindowStartAt].size = currentDE->d_size;
							drive->slidingWindow[i - slidingWindowStartAt].blocks = currentDE->d_blocks;
							drive->slidingWindow[i - slidingWindowStartAt].type = currentDE->d_type;
							drive->slidingWindow[i - slidingWindowStartAt].aux_type = currentDE->d_auxtype;
							drive->slidingWindow[i - slidingWindowStartAt].index = counter;
							drive->slidingWindow[i - slidingWindowStartAt].date.day = currentDE->d_cdate.day;
							drive->slidingWindow[i - slidingWindowStartAt].date.mon = currentDE->d_cdate.mon;
							drive->slidingWindow[i - slidingWindowStartAt].date.year = currentDE->d_cdate.year;
							drive->slidingWindow[i - slidingWindowStartAt].time.hour = currentDE->d_ctime.hour;
							drive->slidingWindow[i - slidingWindowStartAt].time.min = currentDE->d_ctime.min;
						}
					}
					++counter;
					currentDE = readdir(dir);
				}

				drive->length = counter;
				if(drive->currentIndex >= drive->length)
				{
					drive->currentIndex = drive->length;
				}
				writeStatusBarf("Finished reading %u files.", counter);
			}
			else
			{
				writeStatusBar("No entries in directory.");
			}

			closedir(dir);
	
			rootdir(drive->drive->drive, drive->path);
		}
		else
		{
			sprintf(commandPath, "Could not open %s - %d", drive->path, _oserror);
			if(strlen(commandPath) > 76)
			{
				commandPath[76] = '\0';
			}
			waitForEnterEscf(commandPath);
		}
	}

	return counter;
}

void __fastcall  resetSelectedFiles(
	struct panel_drive *panel)
{
	free(panel->selectedEntries);
			
	panel->selectedEntries = 
		calloc((panel->length)/8 + 1, 
			sizeof(unsigned char));
}

void __fastcall  displayDirectory(
	struct panel_drive *drive)
{
	unsigned char w = 19, x = 0, y = 0;
	unsigned char 
		i = 0, start=0, ii = 0, mod = 0, bit = 0, r = 0;
	unsigned char temp[9];
	struct dir_node *currentNode;

	if(drive->path == NULL)
	{
		selectDrive(drive);
		getDirectory(drive, 0);
	}

	if(size_x > 40) w=39;
	if(drive->position == right) x=w + 1;
	
	writePanel(true, false, COLOR_WHITE, x, 1, 21, w, 
		drive->path, NULL, NULL);

	sprintf(temp, "[S%uD%u]", 
		(drive->drive->drive >> 4) & 7, 
		(drive->drive->drive >> 7) + 1);
	cputsxy(x + 3, 22, temp);
	start = drive->displayStartAt;

	for(i=start; i<start + 20 && i < drive->length; i++)
	{
		currentNode = getSpecificNode(drive, i);
		if(currentNode == NULL ||
			currentNode->name == NULL)
		{
			if(i == drive->length - 1) break;
			if(i >= start && drive->slidingWindowStartAt <= start)
			{
				// we are at bottom and scrollable
				drive->slidingWindowStartAt += 5;
				getDirectory(drive, drive->slidingWindowStartAt);
				currentNode = getSpecificNode(drive, i);
			}
			else
			{
				if(drive->slidingWindowStartAt > 5) drive->slidingWindowStartAt = i - 5;
				else drive->slidingWindowStartAt = 0;
				getDirectory(drive, drive->slidingWindowStartAt);
				currentNode = getSpecificNode(drive, i);
			}
		}

		ii =  (currentNode->index) / 8;
		mod =  (currentNode->index) % 8;
		bit = 1 << mod;
		r = drive->selectedEntries[ii] & bit;
		if(r != 0)
		{
			revers(true);
		}
		else
		{
			revers(false);
		}		

		y = i - start + 2;

		sprintf(commandPath, "%5u %-17s %02u-%02u-%02u %2X"
			, currentNode->blocks
			, currentNode->name
			, currentNode->date.year
			, currentNode->date.mon
			, currentNode->date.day
			, currentNode->type
			);
		cputsxy(x + 2, y, commandPath);
		
		revers(false);
	}
}

void __fastcall  writeSelectorPosition(
	struct panel_drive *panel,
	const unsigned char character)
{
	gotoxy(
		(panel == &leftPanelDrive ? 1 : size_x / 2 + 1),
		(panel->currentIndex - panel->displayStartAt) + 2);

	revers(false);
	cputc(character);
}

void __fastcall  writeCurrentFilename(
	struct panel_drive *panel)
{
	struct dir_node *currentDirNode;

	if(panel != NULL)
	{
		if(panel->drive != NULL)
		{
			currentDirNode = getSelectedNode(panel);

			if(currentDirNode != NULL &&
				currentDirNode->name != NULL &&
				strlen(currentDirNode->name) > 0)
			{
				writeStatusBarf("Idx: %3u Sz: %8ld Nm: %s",
					currentDirNode->index,
					currentDirNode->size,
					currentDirNode->name);
			}
			else
			{
				writeStatusBar("No file.");
			}
		}
		else
		{
			writeStatusBar("No drive.");
		}
	}
	else
	{
		writeStatusBar("No panel.");
	}
}

void __fastcall  moveSelectorUp(
	struct panel_drive *panel)
{
	static unsigned char diff;
	static unsigned firstPage;

	writeSelectorPosition(panel, ' ');
	firstPage = panel->displayStartAt == 0;
	diff = panel->currentIndex - panel->displayStartAt;

	if(!firstPage && diff == 1)
	{
		--(panel->displayStartAt);
		--(panel->currentIndex);
		displayDirectory(panel);
	}
	else if(diff > 0)
	{
		--(panel->currentIndex);
	}
	
	writeSelectorPosition(panel, '>');
	writeCurrentFilename(panel);
}

void __fastcall  moveSelectorDown(
	struct panel_drive *panel)
{
	static const unsigned char offset = 19;
	static unsigned char diff;
	static unsigned lastPage;

	writeSelectorPosition(panel, ' ');

	lastPage = panel->displayStartAt + offset + 2 >= panel->length;
	diff = panel->length - panel->displayStartAt;

	if(!lastPage && diff > offset &&
		((panel->currentIndex - panel->displayStartAt) == offset))
	{
		++(panel->displayStartAt);
		++(panel->currentIndex);
		displayDirectory(panel);
	}
	else if(lastPage && 
		(panel->currentIndex - panel->displayStartAt) < offset &&
		(panel->currentIndex + 1) < panel->length)
	{
		++(panel->currentIndex);
	}
	else if(!lastPage)
	{
		++(panel->currentIndex);
	}

	if(panel->currentIndex < 0) panel->currentIndex=0;
	writeSelectorPosition(panel, '>');
	writeCurrentFilename(panel);
}

void selectCurrentFile(void)
{
	static unsigned char 
		index = 0, bit = 0, mod = 0, 
		r = 0, nbit = 0, v = 0, o = 0;

	static struct dir_node *currentDirNode;

	if(selectedPanel != NULL)
	{
		if(selectedPanel->drive != NULL)
		{
			currentDirNode = getSelectedNode(selectedPanel);

			if(currentDirNode != NULL)
			{	
				index = (currentDirNode->index) / 8;
				mod = (currentDirNode->index) % 8;
				bit = 1 << mod;
				nbit = (0xFF ^ bit);
				o = selectedPanel->selectedEntries[index];
				r = o & bit;
				if(r != 0)
				{
					v = o & nbit;
					selectedPanel->selectedEntries[index] = v;
				}
				else 
				{
					selectedPanel->selectedEntries[index] |= bit;
				}

				displayDirectory(selectedPanel);
				moveSelectorDown(selectedPanel);
			}
		}
	}
}

void __fastcall  enterDirectory(
	struct panel_drive *panel)
{
	static struct dir_node *node;

	node = getSelectedNode(panel);

	if(isDirectory(panel))
	{
		sprintf(commandPath, "%s/%s", panel->path, node->name);

		strcpy(panel->path, commandPath);

		getDirectory(selectedPanel, 0);

		panel->currentIndex = 0;
		panel->displayStartAt = 0;
		rereadSelectedPanel();
	}
}

void __fastcall  leaveDirectory(
	struct panel_drive *panel)
{
	const unsigned char* position = 
		strrchr(panel->path, '/');

	panel->path[strlen(panel->path) - strlen(position)] = '\0';

	panel->currentIndex = 0;
	panel->displayStartAt = 0;
	rereadSelectedPanel();
}

unsigned char __fastcall  getDiskImageType(
	struct panel_drive *panel)
{
	static unsigned result = false;
	static unsigned char name[17];
	static struct dir_node *currentDirNode = NULL;

	currentDirNode = getSelectedNode(panel);

	strcpy(name, currentDirNode->name);
	strlower(name);

	if(currentDirNode != NULL)
	{
		if(strstr(name, ".do") > 0
			|| strstr(name, ".dsk") > 0
		)
		{
			result = 2;
		}
		else if(strstr(name, ".po") > 0
			|| strstr(name, ".hdv") > 0
		)
		{
			result = 1;
		}
		else
		{
			result = 0;
		}
	}

	return result;
}

bool __fastcall isDirectory(
	struct panel_drive *panel)
{
	static unsigned result = false;
	static struct dir_node *currentDirNode = NULL;

	currentDirNode = getSelectedNode(panel);

	if(currentDirNode != NULL)
	{
		if(currentDirNode->type == 0x0F)
		{
			result = true;
		}
		else
		{
			result = false;
		}
	}

	return result;
}

struct dir_node*  __fastcall getSelectedNode(
	struct panel_drive *panel)
{
	return getSpecificNode(panel, panel->currentIndex);
}

struct dir_node*  __fastcall getSpecificNode(
	struct panel_drive *panel, 
	const int index)
{
	static struct dir_node *currentDirNode = NULL;

	if(panel != NULL)
	{
		if(panel->drive != NULL)
		{

			if(index >= panel->slidingWindowStartAt &&
				index < panel->slidingWindowStartAt + SLIDING_WINDOW_SIZE)
			{
				return &(panel->slidingWindow[index - panel->slidingWindowStartAt]);
			}
			else
			{
				return NULL;
			}
		}
	}

	return currentDirNode;
}

//unsigned char  sendCommand(
//	struct panel_drive *panel,
//	unsigned char *command)
//{
//	return 0;
//}

void  __fastcall selectAllFiles(
	struct panel_drive *panel, 
	const bool select)
{
	unsigned int i = 0;

	if(panel != NULL)
	{
		for(;i< panel->length / 8 + 1; ++i)
		{
			panel->selectedEntries[i] = 
				(select ? 0xFF : 0x00);
		}
	}

	displayDirectory(panel);
	writeSelectorPosition(panel, '>');
	writeCurrentFilename(panel);

	writeStatusBarf("%s all files", select ? "Selected" : "Deselected");
}

void __fastcall  moveTop(
	struct panel_drive *panel)
{
	if(panel != NULL)
	{
		panel->slidingWindowStartAt = 0;
		panel->currentIndex = 0;
		panel->displayStartAt = 0;

		getDirectory(panel, panel->slidingWindowStartAt);
		displayDirectory(panel);
		writeSelectorPosition(panel, '>');
		writeCurrentFilename(panel);
	}
}

void __fastcall  movePageUp(
	struct panel_drive *panel)
{
	if(panel != NULL)
	{
		if(panel->currentIndex < 20) 
		{
			moveTop(panel);
			return;
		}
		else
		{
			panel->currentIndex -= 19;
			panel->displayStartAt = panel->currentIndex;
			panel->slidingWindowStartAt = panel->currentIndex;

			getDirectory(panel, panel->slidingWindowStartAt);
			displayDirectory(panel);
			writeSelectorPosition(panel, '>');
			writeCurrentFilename(panel);
		}
	}
}

void  __fastcall movePageDown(
	struct panel_drive *panel)
{
	if(panel != NULL)
	{
		panel->currentIndex += 19;
		if(panel->currentIndex > panel->length - 2) 
		{
			moveBottom(panel);
			return;
		}
		else
		{
			panel->displayStartAt = panel->currentIndex - 19;
			panel->slidingWindowStartAt = panel->currentIndex - 19;

			getDirectory(panel, panel->slidingWindowStartAt);
			displayDirectory(panel);
			writeSelectorPosition(panel, '>');
			writeCurrentFilename(panel);
		}
	}
}

void __fastcall  moveBottom(
	struct panel_drive *panel)
{
	if(panel != NULL)
	{
		panel->currentIndex = panel->length - 1;

		if(panel->length > SLIDING_WINDOW_SIZE)
		{
			panel->slidingWindowStartAt = panel->length - SLIDING_WINDOW_SIZE;
		}
		else
		{
			panel->slidingWindowStartAt = 0;
		}

		if(panel->length > 21)
		{
			panel->displayStartAt = panel->length - 20;
		}
		else
		{
			panel->displayStartAt = 0;
		}

		getDirectory(panel, panel->slidingWindowStartAt);
		displayDirectory(panel);
		writeSelectorPosition(panel, '>');
		writeCurrentFilename(panel);
	}
}
