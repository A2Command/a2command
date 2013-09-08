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


#pragma code-name("DISKIMGS");
#pragma rodata-name("DISKIMGS");
//#pragma data-name("DIDATA");

unsigned long __fastcall__ getDriveSize(unsigned char driveNumber)
{
	static dhandle_t drive;
	static unsigned int sectorSize;
	static unsigned int sectorCount;
	static unsigned long driveSize;
		
	drive = dio_open(driveNumber);
	sectorSize = dio_query_sectsize(drive);
	sectorCount = dio_query_sectcount(drive);
	dio_close(drive);
	driveSize = (unsigned long)sectorSize * (unsigned long)sectorCount;

	return driveSize;
}


void writeDiskImage(void)
{
	static unsigned char* message[] =
	{
		{ "Are you ready?" }
	};
	unsigned int i, j, r;
	dhandle_t targetDrive;
	int sourceFile;
	unsigned char imageType;
	struct dir_node *selectedNode;
	unsigned long targetDriveSize;
	unsigned int sectorSize;
	unsigned long sectorCount;
	
	targetPanel = (selectedPanel == &leftPanelDrive ? &rightPanelDrive : &leftPanelDrive);

	selectedNode = getSelectedNode(selectedPanel);

	imageType = getDiskImageType(selectedPanel);

	if(imageType)
	{
		saveScreen();
		if(writeYesNo("Write Disk Image", message, 1))
		{
			retrieveScreen();
			sprintf(filePath, "%s/%s", selectedPanel->path, selectedNode->name); 

			targetDriveSize = getDriveSize(targetPanel->drive);

			if(selectedNode->size <= targetDriveSize)
			{
				sourceFile = open(filePath, O_RDONLY);

				if(sourceFile == -1)
				{
					waitForEnterEscf("Could not open %s", filePath);
					dio_close(targetDrive);
					return;
				}

				targetDrive = dio_open(targetPanel->drive);

				sectorSize = dio_query_sectsize(targetDrive);

				sectorCount = dio_query_sectcount(targetDrive);

				writeStatusBarf("Begin writing....");
				if(imageType == 1)
				{
					for(i=0; i<sectorCount; ++i)
					{
						r = 0;
						r = read(sourceFile, fileBuffer, sectorSize);
						if(r == sectorSize)
						{
							r = dio_write(targetDrive, i, fileBuffer);
							writeStatusBarf("Wrote block %u (%ld%% complete).", i, (unsigned long)(i * (unsigned long)100) / sectorCount);
						}
						else
						{
							waitForEnterEscf("Could not read source file. r=%ld", r);
							break;
						}
					}
				}
				else
				{
					for(i=0; i<sectorCount/8; ++i)
					{
						r  = read(sourceFile, fileBuffer +  512, 256);
						r  = read(sourceFile, fileBuffer + 1024, 256);
						for(j=6; j>0; --j)
						{
							r  = read(sourceFile, fileBuffer + 256, 256);
							r  = read(sourceFile, fileBuffer,       256);
							if(r == 256)
							{
								r = dio_write(targetDrive, i * 8 + j, fileBuffer);
							}
							else
							{
								waitForEnterEscf("Could not write sector %d (error: %d)", i*8+j, r);
								i=sectorCount;
								break;
							}
						}
						if(i<sectorCount)
						{
							r  = read(sourceFile, fileBuffer +  768, 256);
							r  = read(sourceFile, fileBuffer + 1280, 256);
							if(r == 256)
							{
								r  = dio_write(targetDrive, i * 8,     fileBuffer +  512);
								r |= dio_write(targetDrive, i * 8 + 7, fileBuffer + 1024);
							}
							else
							{
								waitForEnterEscf("Could not write sector %d (error: %d)", i*8+j, r);
								i=sectorCount;
								break;
							}
							writeStatusBarf("Wrote track %u (%ld%% complete).", i, (unsigned long)(i * (unsigned long)100) / (sectorCount/8));
						}
					}
				}
		
				dio_close(targetDrive);
				close(sourceFile);

				if(!getdevicedir(targetPanel->drive, targetPanel->path, sizeof(targetPanel->path)))
				{
					strcpy(targetPanel->path, "");
				}

				selectedPanel = targetPanel;	
				rereadSelectedPanel();
				writeSelectorPosition(selectedPanel, '>');

				writeStatusBarf("Writing completed.");
			}
			else
			{
				dio_close(targetDrive);
				close(sourceFile);
#ifdef __APPLE2ENH__
				writeStatusBarf("Target drive is not large enough for image. (drive: %ld, image: %ld)",
#else
				writeStatusBarf("Target too small. (d: %ld, i: %ld)",
#endif
					targetDriveSize, selectedNode->size);
			}
		}
		else
		{
			retrieveScreen();
		}
	}
	else
	{
#ifdef __APPLE2ENH__		
		writeStatusBarf("%s is not a known disk image.", selectedNode->name);
#else
		writeStatusBarf("Unknown disk image %s.", selectedNode->name);
#endif
	}
}

void createDiskImage(void)
{
	static unsigned char* message[] =
	{
		{ "Type a name for" },
		{ "the disk image" }
	};
	static unsigned int i, j, r;
	static unsigned char newName[16];
	static dhandle_t sourceDrive;
	static int targetFile;
	static unsigned int sectorSize;
	static unsigned long sectorCount;
    // Setting the time and date to zero allows ProDOS to set
    // the current value if a clock is present
	static struct {
        unsigned day  :5;
        unsigned mon  :4;
        unsigned year :7;
    } date = {0, 0, 0};
    static struct {
        unsigned char min;
        unsigned char hour;
    } time = {0, 0};
	
	targetPanel = (selectedPanel == &leftPanelDrive ? &rightPanelDrive : &leftPanelDrive);

	newName[0] = '\0';
	r = drawInputDialog(
		2, 15,
		message, "Make Image",
		newName);
	retrieveScreen();

	if((unsigned char)r == OK_RESULT)
	{
		sprintf(filePath, "%s/%s", targetPanel->path, newName);
		_filetype = 0x06;
		_auxtype = 0x00;
		_datetime.createdate = date;
		_datetime.createtime = time;
		targetFile = open(filePath, O_WRONLY | O_CREAT | O_TRUNC);

		if(targetFile == -1)
		{
#ifdef __APPLE2ENH__			
			waitForEnterEscf("Could not open %s for write.", newName);
#else
			waitForEnterEscf("Could not open %s.", newName);
#endif
			if(_osmaperrno(_oserror) == 8)
			{
				writeStatusBarf("Disk or directory is full.", );
			}
			return;
		}

		sourceDrive = dio_open(selectedPanel->drive);

		sectorSize = dio_query_sectsize(sourceDrive);

		sectorCount = dio_query_sectcount(sourceDrive);

		writeStatusBarf("Begin creation...");

		if(strstr(filePath, ".po") || strstr(filePath, ".hdv") ||
			strstr(filePath, ".PO") || strstr(filePath, ".HDV") )
		{
			for(i=0; i<sectorCount; ++i)
			{
				r = dio_read(sourceDrive, i, fileBuffer);
				if(r == 0)
				{
					r = write(targetFile, fileBuffer, sectorSize);
					if(r != sectorSize)
					{
#ifdef __APPLE2ENH__
						waitForEnterEscf("Failed to write sector to target file. r: %d", r);
#else
						waitForEnterEscf("Failed to write sector. r: %d", r);
#endif
						break;
					}
					writeStatusBarf("Created block %u (%ld%% complete).", i, (unsigned long)(i * (unsigned long)100) / sectorCount);
				}
			}
		}
		else
		{
			for(i=0; i<sectorCount/8; ++i)
			{
				r  = dio_read(sourceDrive, i * 8,     fileBuffer +  512);
				r |= dio_read(sourceDrive, i * 8 + 7, fileBuffer + 1024);
				if(r == 0)
				{
					r  = write(targetFile, fileBuffer +  512, 256);
					r &= write(targetFile, fileBuffer + 1024, 256);
				}
				for(j=6; j>0; --j)
				{
					r = dio_read(sourceDrive, i * 8 + j, fileBuffer);
					if(r == 0)
					{
						r  = write(targetFile, fileBuffer + 256, 256);
						r &= write(targetFile, fileBuffer,       256);
					}
				}
				r  = write(targetFile, fileBuffer +  768, 256);
				r &= write(targetFile, fileBuffer + 1280, 256);
				writeStatusBarf("Created track %u (%ld%% complete).", i, (unsigned long)(i * (unsigned long)100) / (sectorCount/8));
			}
		}

		dio_close(sourceDrive);
		close(targetFile);

		selectedPanel = targetPanel;	
		reloadPanels();
		writeSelectorPosition(selectedPanel, '>');

		writeStatusBarf("Created %s.", newName);

	}
}

