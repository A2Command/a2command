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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "Configuration.h"
#include "constants.h"
#include "drives.h"
#include "globalInput.h"
#include "globals.h"
#include "input.h"
#include "screen.h"
#include "menus.h"

unsigned char drivesBuffer[80];

struct drive_status drives[9] =
{
	{ 8, "" },	// 0
	{ 9, "" },	// 1
	{ 10, "" },	// 2
	{ 11, "" },	// 3
	{ 12, "" },	// 4
	{ 13, "" },	// 5
	{ 14, "" },	// 6
	{ 15, "" },	// 7
	{ 16, "" }	// 8
};

unsigned areDrivesInitialized = FALSE;
struct panel_drive leftPanelDrive; 
struct panel_drive rightPanelDrive;
struct panel_drive *selectedPanel;

unsigned char currentLeft = 0;
unsigned char currentRight = 0;

void  initializeDrives(void)
{
	unsigned char i = 0;
	if(!areDrivesInitialized)
	{
		startupDevice = PEEK(0x00BA);

		leftPanelDrive.drive = &(drives[defaultLeftDrive - 8]);
		leftPanelDrive.currentIndex = 0;
		leftPanelDrive.displayStartAt = 0;
		leftPanelDrive.position = left;
		
		for(i=0; i<SLIDING_WINDOW_SIZE; ++i)
		{
			leftPanelDrive.slidingWindow[i].size = 0u;
			leftPanelDrive.slidingWindow[i].type = 0;
		}

		rightPanelDrive.drive = &(drives[defaultRightDrive - 8]);
		rightPanelDrive.currentIndex = 0;
		rightPanelDrive.displayStartAt = 0;
		rightPanelDrive.position = right;
		
		for(i=0; i<SLIDING_WINDOW_SIZE; ++i)
		{
			rightPanelDrive.slidingWindow[i].size = 0u;
			rightPanelDrive.slidingWindow[i].type = 0;
		}

		areDrivesInitialized = TRUE;
		selectedPanel = &leftPanelDrive;
	}
}

int  getDriveStatus(
	struct drive_status *drive)
{
	return 0;
}

void  listDrives(enum menus menu)
{
	unsigned char *message[] =
	{
		{ "Please select a directory." }
	};
	unsigned char dirName[81];
	struct panel_drive *drive;

	if(menu == left)
	{
		leftPanelDrive.drive = &(drives[0]);
		currentLeft = leftPanelDrive.drive->drive;
		drive = &leftPanelDrive;
	}
	else
	{
		rightPanelDrive.drive = &(drives[0]);
		currentRight = rightPanelDrive.drive->drive;
		drive = &rightPanelDrive;
	}

	saveScreen();
	if(drawInputDialog(1, 76, message, "Open Directory", dirName) == OK_RESULT
		&& strlen(dirName) > 0)
	{
			strncpy(drive->header.name, dirName, 16);
			drive->header.size = 0;
			drive->header.type = 10;
			drive->header.index = 0;
	}
	retrieveScreen();

}

int  getDirectory(
	struct panel_drive *drive,
	int slidingWindowStartAt)
{
	unsigned int counter=0, read=0;
	unsigned char result, dr, i;
//	struct cbm_dirent currentDE;
	struct dirent *currentDE;
	DIR *dir = NULL;
	unsigned char *message[] =
	{
		{ "Please select a directory." }
	};
	unsigned char dirName[81];
	

	drive->length = 0;
	drive->slidingWindowStartAt = slidingWindowStartAt;

	dr = drive->drive->drive;

	for(i=0; i<SLIDING_WINDOW_SIZE; ++i)
	{
		drive->slidingWindow[i].size = 0u;
		drive->slidingWindow[i].type = 0;
	}

	saveScreen();
	if(strlen(drive->header.name) == 0 &&
		drawInputDialog(1, 76, message, "Open Directory", dirName) == OK_RESULT
		&& strlen(dirName) > 0)
	{
			strncpy(drive->header.name, dirName, 16);
			drive->header.size = 0;
			drive->header.type = 10;
			drive->header.index = 0;
	}
	retrieveScreen();
	
	if(strlen(drive->header.name) == 0)
	{
		writeStatusBar("Aborting directory operation.");
		return 0;
	}
	else
	{
		dir = opendir(drive->header.name);
		//result = cbm_opendir(dr, dr);
		if(dir != NULL)
		{

			writeStatusBar("Reading directory...");
			counter = 0;
			currentDE = readdir(dir);
			while(currentDE != NULL)
			{
/*				if(currentDE.type == 10 && counter==0)
				{
					strcpy(drive->header.name, currentDE.name);
					drive->header.size = currentDE.size;
					drive->header.type = currentDE.type;
					drive->header.index = 0;
				}
				else*/ 
				if(counter >= slidingWindowStartAt &&
					read < SLIDING_WINDOW_SIZE)
				{
					++read;
					i = counter - 1;
					if(i - slidingWindowStartAt >= 0 && 
						i - slidingWindowStartAt < SLIDING_WINDOW_SIZE)
					{
						strcpy(drive->slidingWindow[i - slidingWindowStartAt].name, currentDE->d_name);
						drive->slidingWindow[i - slidingWindowStartAt].size = currentDE->d_blocks;
						drive->slidingWindow[i - slidingWindowStartAt].type = currentDE->d_type;
						drive->slidingWindow[i - slidingWindowStartAt].index = counter;
					}
				}
				++counter;
				currentDE = readdir(dir);
			}
			//cbm_closedir(dr);

			drive->length = counter;
			if(drive->currentIndex >= drive->length)
			{
				drive->currentIndex = drive->length - 1;
			}

			//cbm_open(2,drive->drive->drive,0,"$:'y/%&");
			//cbm_read(2,buffer,34); // skip unwanted data
			//cbm_read(2,buffer,2);
			//drive->header.size = buffer[1]*256 + buffer[0];

			//cbm_close(2);
			closedir(dir);
			writeStatusBarf("Finished reading %u files.", counter - 1);
		}
	}

	return counter;
//#else
//	return 0;
//#endif
}

void  resetSelectedFiles(struct panel_drive *panel)
{
	free(panel->selectedEntries);
			
	panel->selectedEntries = 
		calloc((panel->length)/8 + 1, 
			sizeof(unsigned char));
}

void  displayDirectory(
	struct panel_drive *drive)
{
	unsigned char w = 19, x = 0, y = 0;
	unsigned char i = 0, start=0, ii = 0, mod = 0, bit = 0, r = 0;
	unsigned char fileType;
	struct dir_node *currentNode;
	unsigned char size[4];

	if(drive->drive == NULL)
	{
		drive->drive = &(drives[startupDevice - 8]);
		getDirectory(drive, 0);
		resetSelectedFiles(drive);
	}

	if(drive->header.name == NULL)
	{
		getDirectory(drive, 0);
	}

	if(size_x > 40) w=39;
	if(drive->position == right) x=w + 1;
	
	writePanel(TRUE, FALSE, color_text_borders, x, 1, 21, w, 
		drive->header.name, NULL, NULL);

	gotoxy(x+1, 22); cprintf("[%2d]", drive->drive->drive);
	gotoxy(x + w - 7, 22); cprintf("[%5u]", drive->header.size);

	start = drive->displayStartAt;

	for(i=start; i<start + 20 && i < drive->length - 1; i++)
	{
		currentNode = getSpecificNode(drive, i);
		if(currentNode == NULL ||
			currentNode->name == NULL)
		{
			if(i == drive->length - 1) break;
			if(i > start)
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

		shortenSize(size, currentNode->size);
		fileType = getFileType(currentNode->type);

		//textcolor(color_text_files);
		ii =  (currentNode->index - 1) / 8;
		mod =  (currentNode->index - 1) % 8;
		bit = 1 << mod;
		r = drive->selectedEntries[ii] & bit;
		if(r != 0)
		{
			revers(TRUE);
		}
		else
		{
			revers(FALSE);
		}		

		y = i - start + 2;
		cputcxy(x + 2, y, fileType);
		cputsxy(x + 4, y, size);
		cputsxy(x + 8, y, shortenString(currentNode->name));
		
		revers(FALSE);
		
	}
	
}

void  writeSelectorPosition(struct panel_drive *panel,
	unsigned char character)
{
	unsigned char x, y;
	y = (panel->currentIndex - panel->displayStartAt) + 2;
	x = (panel == &leftPanelDrive ? 1 : size_x / 2 + 1);
	gotoxy(x, y);
	//textcolor(color_selector);
	revers(FALSE);
	cputc(character);
}

void  writeCurrentFilename(struct panel_drive *panel)
{
	struct dir_node *currentDirNode;

	if(panel != NULL)
	{
		if(panel->drive != NULL)
		{
			currentDirNode = getSelectedNode(panel);

			if(currentDirNode != NULL &&
				currentDirNode->name != NULL)
			{
				writeStatusBarf("Idx: %3u Sz: %5u Nm: %s",
					currentDirNode->index,
					currentDirNode->size,
					currentDirNode->name);
			}
		}
	}
}

void  moveSelectorUp(struct panel_drive *panel)
{
	unsigned char diff;
	unsigned firstPage;

	writeSelectorPosition(panel, ' ');
	firstPage = panel->displayStartAt == 0;
	diff = panel->currentIndex - panel->displayStartAt;

	if(!firstPage && diff == 1)
	{
		panel->displayStartAt--;
		panel->currentIndex--;
		displayDirectory(panel);
	}
	else if(diff > 0)
	{
		panel->currentIndex--;
	}
	
	writeSelectorPosition(panel, '>');
	writeCurrentFilename(panel);
}

void  moveSelectorDown(struct panel_drive *panel)
{
	const unsigned char offset = 19;
	unsigned char diff;
	unsigned lastPage;

	writeSelectorPosition(panel, ' ');

	lastPage = panel->displayStartAt + offset + 2 >= panel->length;
	diff = panel->length - panel->displayStartAt;

	if(!lastPage && diff > offset &&
		((panel->currentIndex - panel->displayStartAt) == offset))
	{
		panel->displayStartAt++;
		panel->currentIndex++;
		displayDirectory(panel);
	}
	else if(lastPage && 
		(panel->currentIndex - panel->displayStartAt) < offset &&
		(panel->currentIndex + 2) < panel->length)
	{
		panel->currentIndex++;
	}
	else if(!lastPage)
	{
		panel->currentIndex++;
	}

	if(panel->currentIndex < 0) panel->currentIndex=0;
	writeSelectorPosition(panel, '>');
	writeCurrentFilename(panel);
}

unsigned char  getFileType(unsigned char type)
{
	if(type < 7) return "DSPURCD"[type];
	return 'O';
}

void  shortenSize(unsigned char* buffer, unsigned int value)
{
	if(value < 1000)
	{
		sprintf(buffer, "%3d", value);
	}
	else
	{
		sprintf(buffer, "%2dK", (value + 512)/1024);
	}
}

unsigned char*  shortenString(unsigned char* source)
{
	const int targetLength = 11;
	unsigned char buffer[18];

	if(size_x == 40)
	{
		if(strlen(source) > targetLength)
		{
			strncpy(buffer, source, targetLength - 3);
			buffer[targetLength - 3] = '.';
			buffer[targetLength - 2] = '.';
			buffer[targetLength - 1] = '.';
			buffer[targetLength] = '\0';
		}
		else
		{
			return source;
		}
	}
	else
	{
		return source;
	}

	return buffer;
}

void  selectCurrentFile(void)
{
	unsigned char index = 0, bit = 0, mod = 0, 
		r = 0, nbit = 0, v = 0, o = 0;

	struct dir_node *currentDirNode;

	if(selectedPanel != NULL)
	{
		if(selectedPanel->drive != NULL)
		{
			currentDirNode = getSelectedNode(selectedPanel);

			if(currentDirNode != NULL)
			{	
				index = (currentDirNode->index - 1) / 8;
				mod = (currentDirNode->index - 1) % 8;
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

void  enterDirectory(struct panel_drive *panel)
{
	unsigned char command[40];
	struct dir_node *node;

	node = getSelectedNode(panel);

	if(isDiskImage(panel) || isDirectory(panel))
	{
		strcpy(command, "cd:");
		strcat(command, node->name);
		sendCommand(panel, command);
		panel->currentIndex = 0;
		panel->displayStartAt = 0;
		rereadSelectedPanel();
	}
}

void  leaveDirectory(struct panel_drive *panel)
{
	unsigned char buffer[4];
	buffer[0] = 'c';
	buffer[1] = 'd';
	buffer[2] = 95;
	buffer[3] = '\0';

	sendCommand(panel, buffer);
	panel->currentIndex = 0;
	panel->displayStartAt = 0;
	rereadSelectedPanel();
}

unsigned  isDiskImage(struct panel_drive *panel)
{
	unsigned result = FALSE;
	unsigned char name[17];
	struct dir_node *currentDirNode = NULL;

	currentDirNode = getSelectedNode(panel);

	strcpy(name, currentDirNode->name);
	strlower(name);

	if(currentDirNode != NULL)
	{
		if(strstr(name, ".d64") > 0
			|| strstr(name, ".d81") > 0
			|| strstr(name, ".d71") > 0
			|| strstr(name, ".dnp") > 0
		)
		{
			result = TRUE;
		}
		else
		{
			result = FALSE;
		}
	}

	return result;
}

unsigned  isDirectory(struct panel_drive *panel)
{
	unsigned result = FALSE;
	struct dir_node *currentDirNode = NULL;

	currentDirNode = getSelectedNode(panel);

	if(currentDirNode != NULL)
	{
		if(currentDirNode->type == 6)
		{
			result = TRUE;
		}
		else
		{
			result = FALSE;
		}
	}

	return result;
}

struct dir_node*  getSelectedNode(struct panel_drive *panel)
{
	return getSpecificNode(panel, panel->currentIndex);
}

struct dir_node*  getSpecificNode(
	struct panel_drive *panel, int index)
{
	struct dir_node *currentDirNode = NULL;

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

unsigned char  sendCommand(
	struct panel_drive *panel,
	unsigned char *command)
{
	return 0;
}

void  selectAllFiles(struct panel_drive *panel, 
	unsigned select)
{
	unsigned int i = 0;
	if(panel != NULL)
	{
		for(;i< panel->length / 8 + 1; ++i)
		{
			panel->selectedEntries[i] = 
				(select == TRUE ? 0xFF : 0x00);
		}
	}

	displayDirectory(panel);
	writeSelectorPosition(panel, '>');
	writeCurrentFilename(panel);
}

void  moveTop(struct panel_drive *panel)
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

void  movePageUp(struct panel_drive *panel)
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

void  movePageDown(struct panel_drive *panel)
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

void  moveBottom(struct panel_drive *panel)
{
	if(panel != NULL)
	{
		panel->currentIndex = panel->length - 2;

		if(panel->length > 30)
		{
			panel->slidingWindowStartAt = panel->length - 30;
		}
		else
		{
			panel->slidingWindowStartAt = 0;
		}

		if(panel->length > 21)
		{
			panel->displayStartAt = panel->length - 21;
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
