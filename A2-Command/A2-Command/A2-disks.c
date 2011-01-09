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
#include <dio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "A2-disks.h"
#include "screen.h"
#include "globals.h"
#include "input.h"
#include "globalInput.h"
#include "drives.h"
#include "menus.h"

unsigned char _driveCount;
unsigned char* _drives;

void __fastcall__ selectDrive(struct panel_drive *panel)
{
	static unsigned char i, key, current;
	static unsigned char buffer[68];
	static unsigned char temp[80];

	_driveCount = drivecount();
	_drives = drivelist();

	saveScreen();
	drawBox(5, 5, size_x - 10, size_y - 10, COLOR_WHITE, 0);

	cputsxy(7, 7, "Select directory for panel.");

	for(i=0; i<_driveCount; ++i)
	{
		if(rootdir(_drives[i], buffer) > -1)
		{
			sprintf(temp, "Slot %u Drive %u - %s", (_drives[i]>>4)&7, (_drives[i]>>7)+1, buffer);
		}
		else
		{
			sprintf(temp, "Slot %u Drive %u - ERROR or No Disk", (_drives[i]>>4)&7, (_drives[i]>>7)+1);
		}

		cputsxy(8, 9 + i, temp); 
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

	retrieveScreen();

	if(key == CH_ENTER)
	{
		panel->drive = &drives[current];
		panel->drive->drive = _drives[current];		
		if(rootdir(_drives[current], panel->path) == -1)
		{
			strcpy(panel->path, "");
		}

		selectedPanel = panel;

		return;
	}
}

unsigned long __fastcall__ getDriveSize(unsigned char driveNumber)
{
	static dhandle_t drive;
	static sectsize_t sectorSize;
	static sectnum_t sectorCount;
	static unsigned long driveSize;	
		
	drive = dio_open(driveNumber);
	sectorSize = dio_query_sectsize(drive);
	sectorCount = dio_query_sectcount(drive);
	dio_close(drive);
	driveSize = (unsigned long)sectorSize * (unsigned long)sectorCount;

	return driveSize;
}

unsigned char filePath[MAX_PATH_LENGTH];
void __fastcall__ writeDiskImage(void)
{
	unsigned int i, r;
	struct panel_drive *targetPanel;
	dhandle_t targetDrive;
	FILE *sourceFile;
	struct dir_node *selectedNode;
	unsigned long targetDriveSize;
	unsigned int sectorSize;
	unsigned long sectorCount;
	
	targetPanel = (selectedPanel == &leftPanelDrive ? &rightPanelDrive : &leftPanelDrive);

	selectedNode = getSelectedNode(selectedPanel);

	if(isDiskImage(selectedPanel))
	{
		sprintf(filePath, "%s/%s", selectedPanel->path, selectedNode->name); 

		targetDriveSize = getDriveSize(targetPanel->drive->drive);

		if(selectedNode->size == targetDriveSize)
		{
			sourceFile = fopen(filePath, "rb");

			targetDrive = dio_open(targetPanel->drive->drive);

			sectorSize = dio_query_sectsize(targetDrive);

			sectorCount = dio_query_sectcount(targetDrive);

			writeStatusBarf("Begin writing....");
			for(i=0; i<sectorCount; ++i)
			{
				r = fread(fileBuffer, sectorSize, 1, sourceFile);

				if(r == 1)
				{
					r = dio_write(targetDrive, i, fileBuffer);

					writeStatusBarf("Wrote sector %u (%ld%% complete).", i, (unsigned long)(i * (unsigned long)100) / sectorCount);
				}
			}
		
			dio_close(targetDrive);
			fclose(sourceFile);

			if(rootdir(targetPanel->drive->drive, targetPanel->path) == -1)
			{
				strcpy(targetPanel->path, "");
			}

			selectedPanel = targetPanel;	
			rereadSelectedPanel();

			writeStatusBarf("Writing completed.");
		}
		else
		{
			dio_close(targetDrive);
			fclose(sourceFile);

			writeStatusBarf("Disk image size does not match target drive. (drive: %ld, image: %ld)", 
				targetDriveSize, selectedNode->size);
		}
	}
	else
	{
		writeStatusBarf("%s is not a disk image.", selectedNode->name);
	}
}

void __fastcall__ createDiskImage(void)
{
	static unsigned char* message[] =
	{
		{ "Type a name for" },
		{ "the disk image" }
	};
	static unsigned int i, r;
	static unsigned char newName[17];
	static dhandle_t sourceDrive;
	static FILE *targetFile;
	static unsigned int sectorSize;
	static unsigned long sectorCount;
	static struct panel_drive *targetPanel;
	
	targetPanel = (selectedPanel == &leftPanelDrive ? &rightPanelDrive : &leftPanelDrive);

	newName[0] = '\0';
	r = drawInputDialog(
		2, 17,
		message, "Make Image",
		newName);
	retrieveScreen();

	if((unsigned char)r == OK_RESULT)
	{
		sprintf(filePath, "%s/%s", targetPanel->path, newName);

		targetFile = fopen(filePath, "wb");

		sourceDrive = dio_open(selectedPanel->drive->drive);

		sectorSize = dio_query_sectsize(sourceDrive);

		sectorCount = dio_query_sectcount(sourceDrive);

		writeStatusBarf("Begin creation...");
		for(i=0; i<sectorCount; ++i)
		{
			r = dio_read(sourceDrive, i, fileBuffer);

			if(r == 0)
			{
				r = fwrite(fileBuffer, sectorSize, 1, targetFile);

				writeStatusBarf("Created sector %u (%ld%% complete).", i, (unsigned long)(i * (unsigned long)100) / sectorCount);
			}
		}
		
		dio_close(sourceDrive);
		fclose(targetFile);

		selectedPanel = targetPanel;	
		reloadPanels();

		writeStatusBarf("Created %s.", filePath);

	}
}

void __fastcall__ copyDisk(void)
{
	static unsigned char* message[] =
	{
		{ "Are you ready?" }
	};
	static bool yesNo;
	unsigned long counter = 0;
	static dhandle_t sourceDrive;
	static dhandle_t targetDrive;
	static unsigned long sectorCount;
	static struct panel_drive *targetPanel;
	
	targetPanel = (selectedPanel == &leftPanelDrive ? &rightPanelDrive : &leftPanelDrive);

	saveScreen();
	yesNo = writeYesNo("Copy Disk", message, 1);
	retrieveScreen();

	if(yesNo)
	{
		sourceDrive = dio_open(selectedPanel->drive->drive);
		targetDrive = dio_open(targetPanel->drive->drive);

		if(dio_query_sectsize(sourceDrive) == dio_query_sectsize(targetDrive) 
			&& dio_query_sectcount(sourceDrive) == dio_query_sectcount(targetDrive))
		{
			sectorCount = dio_query_sectcount(sourceDrive);

			writeStatusBarf("Begin copy....");
			for(; counter < sectorCount; ++counter)
			{
				dio_read(sourceDrive, counter, fileBuffer);
				dio_write(targetDrive, counter, fileBuffer);
			
				writeStatusBarf(
					"Copied %ld blocks, %ld remaining. (%ld%% complete)", 
					counter + 1, 
					sectorCount - counter + 1,
					(unsigned long)(counter * (unsigned long)100) / sectorCount);
			}

			writeStatusBarf("Copied S%uD%u to S%uD%u",
				(selectedPanel->drive->drive >> 4) & 7,
				(selectedPanel->drive->drive >> 7) + 1,
				(targetPanel->drive->drive >> 4) & 7,
				(targetPanel->drive->drive >> 7) + 1);
		}
		else
		{
			writeStatusBarf("Cannot copy, drives are not the same size.");
		}

		dio_close(sourceDrive);
		dio_close(targetDrive);

		if(rootdir(targetPanel->drive->drive, targetPanel->path) == -1)
		{
			strcpy(targetPanel->path, "");
		}

		selectedPanel = targetPanel;	
		rereadSelectedPanel();

		writeStatusBarf("Disk copy complete.");
	}
	else
	{
		writeStatusBarf("Disk copy aborted.");
	}
}
