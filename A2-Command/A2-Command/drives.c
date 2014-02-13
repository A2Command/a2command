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
#ifdef __APPLE2ENH__
#include <apple2enh.h>
#else
#include <apple2.h>
#endif

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

unsigned char fileBuffer[COPY_BUFFER_SIZE];

unsigned areDrivesInitialized = false;
struct panel_drive leftPanelDrive; 
struct panel_drive rightPanelDrive;
struct panel_drive *selectedPanel;
struct panel_drive *targetPanel;


int __fastcall  getDirectory(
	struct panel_drive *drive,
	const int slidingWindowStartAt)
{
	unsigned int counter=0, blocks=0;
	unsigned char i;
	struct dirent *currentDE;
	DIR *dir = NULL;

	drive->length = 0;
	drive->slidingWindowStartAt = slidingWindowStartAt;

	for(i=0; i<SLIDING_WINDOW_SIZE; ++i)
	{
		drive->slidingWindow[i].size = 0u;
		drive->slidingWindow[i].type = 0;
		strcpy(drive->slidingWindow[i].name, "");
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
			currentDE = readdir(dir);
			if(strlen(currentDE->d_name) > 0)
			{
				while(currentDE != NULL)
				{
					i = counter - slidingWindowStartAt;
					if(i >= 0 &&
					   i < SLIDING_WINDOW_SIZE)
					{
						strcpy(drive->slidingWindow[i].name, currentDE->d_name);
						drive->slidingWindow[i].size = currentDE->d_size;
						drive->slidingWindow[i].blocks = currentDE->d_blocks;
						drive->slidingWindow[i].type = currentDE->d_type;
						drive->slidingWindow[i].access = currentDE->d_access;
						drive->slidingWindow[i].aux_type = currentDE->d_auxtype;
						drive->slidingWindow[i].index = counter;
						drive->slidingWindow[i].date.day = currentDE->d_cdate.day;
						drive->slidingWindow[i].date.mon = currentDE->d_cdate.mon;
						drive->slidingWindow[i].date.year = currentDE->d_cdate.year;
						drive->slidingWindow[i].time.hour = currentDE->d_ctime.hour;
						drive->slidingWindow[i].time.min = currentDE->d_ctime.min;
					}
					++counter;
					blocks += currentDE->d_blocks;
					currentDE = readdir(dir);
				}

				drive->length = counter;
				drive->usedblocks = blocks;
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
		}
		else
		{
			sprintf(filePath, "Could not open %s - %d", drive->path, _oserror);
			if(strlen(filePath) > 76)
			{
				filePath[76] = '\0';
			}
			return -1;
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

static const unsigned char displayHeight = 20;

void __fastcall  displayDirectory(
	struct panel_drive *drive)
{
	unsigned char w = 39, x = 0, y = 0;
	unsigned char i, start, end;
	unsigned char temp[9];
	struct dir_node *currentNode;

	if(drive->path == NULL)
	{
		selectDrive(drive);
		getDirectory(drive, 0);
	}

#ifdef __APPLE2ENH__
	if(drive->position == right) x=w + 1;
#else
	w=20;
	if(drive->position == right) x=w;
#endif
	
	writePanel(true, false, COLOR_WHITE, x, 1, 21, w,
			   drive->path, NULL, NULL);
	
	sprintf(temp, "[S%uD%u]",
		(drive->drive) & 7,
		(drive->drive >> 3) + 1);
	cputsxy(x + 3, 22, temp);
	
	//gotox(x + w - 9); cprintf("[%5u]", get_info(drive->path));
	
	start = drive->displayStartAt;
	end = start + displayHeight < drive->length ? start + displayHeight: drive->length;
	for(i = start; i < end; i++)
	{
		currentNode = getSpecificNode(drive, i);
		if(currentNode == NULL ||
			currentNode->name == NULL)
		{
			if(i != start)
			{
				// we are at bottom and scrollable
				getDirectory(drive, drive->slidingWindowStartAt + 5);
			}
			else
			{
				getDirectory(drive, i - 5);
			}
			currentNode = getSpecificNode(drive, i);
		}

        revers(drive->selectedEntries[(currentNode->index) / 8u]
               & (unsigned char)(1 << ((currentNode->index) % 8u)));

		y = i - start + 2;
#ifdef __APPLE2ENH__
		sprintf(filePath, "%5u  %-16s %02u-%02u-%02u %3s"
			, currentNode->blocks
			, currentNode->name
			, currentNode->date.year
			, currentNode->date.mon
			, currentNode->date.day
			, _fileTypes[currentNode->type]
			);
		cputsxy(x + 2, y, filePath);
		
		if(!(currentNode->access & 0xC2))
			cputcxy(x + 8, y, '*');
#else
		sprintf(filePath, "%-14.14s %3s"
				, currentNode->name
				, _fileTypes[currentNode->type]);
		cputsxy(x + 2, y, filePath);
		
		if(!(currentNode->access & 0xC2))
			cputcxy(x + 1, y, '*');
#endif
		revers(false);
	}
}

void __fastcall  writeSelectorPosition(
	struct panel_drive *panel,
	unsigned char character)
{
#ifdef __APPLE2ENH__
	gotoxy(
		   (panel == &leftPanelDrive ? 1 : size_x / 2 + 1),
		   (panel->currentIndex - panel->displayStartAt) + 2);
#else
	gotoxy(
		(panel == &leftPanelDrive ? 0 : size_x / 2),
		(panel->currentIndex - panel->displayStartAt) + 2);

	if(character == ' ')
	{
		character = '!';
	}
#endif
	revers(false);
	cputc(character);
}

void __fastcall  writeCurrentFilename(
	struct panel_drive *panel)
{
	struct dir_node *currentDirNode;

	//if(panel != NULL)
	//{
	//	if(panel->drive != NULL)
	//	{
			currentDirNode = getSelectedNode(panel);

			if(currentDirNode != NULL &&
				currentDirNode->name != NULL &&
				strlen(currentDirNode->name) > 0)
			{
#ifdef __APPLE2ENH__				
				writeStatusBarf("Index: %3u  Size: %8ld  Name: %s",
								currentDirNode->index,
								currentDirNode->size,
								currentDirNode->name);
#else
				writeStatusBarf("%02u-%02u-%02u  %6ld Bytes  %s",
								currentDirNode->date.year,
								currentDirNode->date.mon,
								currentDirNode->date.day,
								currentDirNode->size,
								currentDirNode->name);
#endif
			}
			else
			{
				writeStatusBar("No file.");
			}
		//}
	//	else
	//	{
	//		writeStatusBar("No drive.");
	//	}
	//}
	//else
	//{
	//	writeStatusBar("No panel.");
	//}
}

void __fastcall  moveSelectorUp(
	struct panel_drive *panel)
{
	writeSelectorPosition(panel, ' ');

	if (panel->currentIndex > 0)
	{
		if (--(panel->currentIndex) < panel->displayStartAt)
		{
			--(panel->displayStartAt);
			displayDirectory(panel);
		}
	}
	
	writeSelectorPosition(panel, '>');
	writeCurrentFilename(panel);
}

void __fastcall  moveSelectorDown(
	struct panel_drive *panel)
{
	writeSelectorPosition(panel, ' ');

	if (panel->currentIndex < panel->length - 1)
	{
		if (++(panel->currentIndex) >= panel->displayStartAt + displayHeight)
		{
			++(panel->displayStartAt);
			displayDirectory(panel);
		}
	}

	writeSelectorPosition(panel, '>');
	writeCurrentFilename(panel);
}

void selectCurrentFile(void)
{
	static struct dir_node *currentDirNode;

	if(selectedPanel != NULL)
	{
		if(selectedPanel->drive != NULL)
		{
			currentDirNode = getSelectedNode(selectedPanel);

			if(currentDirNode != NULL)
			{	
				// Toggle the selected entry.
				selectedPanel->selectedEntries[(currentDirNode->index) / 8u]
				^= 1 << ((currentDirNode->index) % 8u);

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
		sprintf(filePath, "%s/%s", panel->path, node->name);

		if(strlen(filePath) < 65)
		{
			strcpy(panel->path, filePath);

			getDirectory(selectedPanel, 0);

			panel->currentIndex = 0;
			panel->displayStartAt = 0;
			rereadSelectedPanel();
		}
		else
		{
			writeStatusBar("Path would be too long.");
		}
	}
}

void __fastcall  leaveDirectory(
	struct panel_drive *panel)
{
	const unsigned char* position = 
		strrchr(panel->path, '/');

	if((void *)position != (void*)(panel->path))
	{
		panel->path[strlen(panel->path) - strlen(position)] = '\0';

		panel->currentIndex = 0;
		panel->displayStartAt = 0;
		rereadSelectedPanel();
	}
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

	//if(panel != NULL)
	//{
	//	if(panel->drive != NULL)
	//	{

			if(index >= panel->slidingWindowStartAt &&
				index < panel->slidingWindowStartAt + SLIDING_WINDOW_SIZE)
			{
				return &(panel->slidingWindow[index - panel->slidingWindowStartAt]);
			}
			else
			{
				return NULL;
			}
	//	}
	//}

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

	//if(panel != NULL)
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
	//if(panel != NULL)
	{
		panel->currentIndex = 0;
		panel->displayStartAt = 0;

		getDirectory(panel, 0);
		displayDirectory(panel);
		writeSelectorPosition(panel, '>');
		writeCurrentFilename(panel);
	}
}

void __fastcall  movePageUp(
	struct panel_drive *panel)
{
	//if(panel != NULL)
	{
		if(panel->currentIndex < 20) 
		{
			moveTop(panel);
		}
		else
		{
			panel->currentIndex -= 19;
			panel->displayStartAt = panel->currentIndex;

			getDirectory(panel, panel->currentIndex);
			displayDirectory(panel);
			writeSelectorPosition(panel, '>');
			writeCurrentFilename(panel);
		}
	}
}

void  __fastcall movePageDown(
	struct panel_drive *panel)
{
	//if(panel != NULL)
	{
		panel->currentIndex += 19;
		if(panel->currentIndex > panel->length - 1)
		{
			moveBottom(panel);
			return;
		}
		else
		{
			panel->displayStartAt = panel->currentIndex - 19;

			getDirectory(panel, panel->displayStartAt);
			displayDirectory(panel);
			writeSelectorPosition(panel, '>');
			writeCurrentFilename(panel);
		}
	}
}

void __fastcall  moveBottom(
	struct panel_drive *panel)
{
	//if(panel != NULL)
	{
		if ((panel->currentIndex = panel->length - 1) < 0)
		{
			panel->currentIndex = 0;
		}
		panel->displayStartAt =
		(panel->length > displayHeight) ?
		panel->length - displayHeight : 0;

		getDirectory(panel, panel->length - SLIDING_WINDOW_SIZE);
		displayDirectory(panel);
		writeSelectorPosition(panel, '>');
		writeCurrentFilename(panel);
	}
}
