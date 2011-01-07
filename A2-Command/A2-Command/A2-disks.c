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

unsigned char _driveCount;
unsigned char* _drives;

void selectDrive(struct panel_drive *panel)
{
	unsigned char i, key, current;
	unsigned char buffer[68];
	unsigned char temp[80];

	_driveCount = drivecount();
	_drives = drivelist();

	saveScreen();
	drawBox(5, 5, size_x - 10, size_y - 10, COLOR_WHITE, 0);

	cputsxy(7, 7, "Select directory for panel.");

	for(i=0; i<_driveCount; ++i)
	{
		if(rootdir(_drives[i], buffer) > -1)
		{
			sprintf(temp, "S%uD%u (%u) - %s", (_drives[i]>>4)&7, (_drives[i]>>7)+1, _drives[i], buffer);
		}
		else
		{
			sprintf(temp, "S%uD%u (%u) - ERROR or No Disk", (_drives[i]>>4)&7, (_drives[i]>>7)+1, _drives[i], i);
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

unsigned long getDriveSize(unsigned char driveNumber)
{
	dhandle_t drive;
	sectsize_t sectorSize;
	sectnum_t sectorCount;
	unsigned long driveSize;	
		
	drive = dio_open(driveNumber);
	sectorSize = dio_query_sectsize(drive);
	sectorCount = dio_query_sectcount(drive);
	dio_close(drive);
	driveSize = (unsigned long)sectorSize * (unsigned long)sectorCount;

	return driveSize;
}

void writeDiskImage(void)
{
	static unsigned int i, r;
	static unsigned char filePath[MAX_PATH_LENGTH];
	static struct panel_drive *targetPanel;
	static dhandle_t targetDrive;
	static FILE *sourceFile;
	static struct dir_node *selectedNode;
	static unsigned long targetDriveSize;
	static unsigned int sectorSize;
	
	targetPanel = (selectedPanel == &leftPanelDrive ? &rightPanelDrive : &leftPanelDrive);

	selectedNode = getSelectedNode(selectedPanel);

	sprintf(filePath, "%s/%s", selectedPanel->path, selectedNode->name); 

	targetDriveSize = getDriveSize(targetPanel->drive->drive);

	if(selectedNode->size == targetDriveSize)
	{
		sourceFile = fopen(filePath, "rb");

		targetDrive = dio_open(targetPanel->drive->drive);

		sectorSize = dio_query_sectsize(targetDrive);

		for(i=0; i<dio_query_sectcount(targetDrive); ++i)
		{
			r = fread(fileBuffer, sectorSize, 1, sourceFile);

			if(r == 1)
			{
				r = dio_write(targetDrive, i, fileBuffer);

				writeStatusBarf("Wrote sector %u", i);
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

		writeStatusBarf("Wrote %s to disk.", selectedNode->name);
	}
	else
	{
		dio_close(targetDrive);
		fclose(sourceFile);

		writeStatusBarf("Disk image size does not match target drive. (drive: %ld, image: %ld)", 
			targetDriveSize, selectedNode->size);
	}
}

void createDiskImage(void)
{
	//static unsigned char* message[] =
	//{
	//	{ "Type a name for the disk image" }
	//};
	//static unsigned int i, r;
	//static unsigned char filePath[MAX_PATH_LENGTH];
	//static unsigned char newName[17];
	//static struct panel_drive *targetPanel;
	//static dhandle_t sourceDrive;
	//static FILE *targetFile;
	//static unsigned int sectorSize;
	//
	//targetPanel = (selectedPanel == &leftPanelDrive ? &rightPanelDrive : &leftPanelDrive);

	//newName[0] = '\0';
	//r = drawInputDialog(
	//	1, 17,
	//	message, "Make Image",
	//	newName);
	//retrieveScreen();

	//if((unsigned char)r == OK_RESULT)
	//{
	//	sprintf(filePath, "%s/%s", targetPanel->path, newName);

	//	targetFile = fopen(filePath, "rb");

	//	sourceDrive = dio_open(targetPanel->drive->drive);

	//	sectorSize = dio_query_sectsize(sourceDrive);

	//	for(i=0; i<dio_query_sectcount(sourceDrive); ++i)
	//	{
	//		r = dio_read(sourceDrive, i, fileBuffer);

	//		if(r == 0)
	//		{
	//			r = fwrite(fileBuffer, sectorSize, 1, targetFile);

	//			writeStatusBarf("Wrote sector %u", i);
	//		}
	//	}
	//	
	//	dio_close(sourceDrive);
	//	fclose(targetFile);

	//	selectedPanel = targetPanel;	
	//	rereadSelectedPanel();

	//	writeStatusBarf("Created %s.", newName);
	//}
}

void copyDisk()
{
}
