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


#pragma code-name("DISKCOPY");
#pragma rodata-name("DISKCOPY");
////#pragma data-name("DCDATA");

void copyDisk(void)
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
	
	targetPanel = (selectedPanel == &leftPanelDrive ? &rightPanelDrive : &leftPanelDrive);

	saveScreen();
	yesNo = writeYesNo("Copy Disk", message, 1);
	retrieveScreen();

	if(yesNo)
	{
		sourceDrive = dio_open(selectedPanel->drive);
		targetDrive = dio_open(targetPanel->drive);

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
#ifdef __APPLE2ENH__								
					"Copied %ld blocks, %ld remaining. (%ld%% complete)",
					counter + 1,
					sectorCount - counter + 1,
					(unsigned long)(counter * (unsigned long)100) / sectorCount);
#else
					"Copied %ld blocks. (%ld%%)",
					counter + 1,
					(unsigned long)(counter * (unsigned long)100) / sectorCount);
#endif

			}

			writeStatusBarf("Copied S%uD%u to S%uD%u",
				(selectedPanel->drive) & 7,
				(selectedPanel->drive >> 3) + 1,
				(targetPanel->drive) & 7,
				(targetPanel->drive >> 3) + 1);
		}
		else
		{
#ifdef __APPLE2ENH__			
			writeStatusBarf("Cannot copy, drives are not the same size.");
#else
			writeStatusBarf("Error: drives are not the same size.");
#endif
		}

		dio_close(sourceDrive);
		dio_close(targetDrive);

		if(!getdevicedir(targetPanel->drive, targetPanel->path, sizeof(targetPanel->path)))
		{
			strcpy(targetPanel->path, "");
		}

		selectedPanel = targetPanel;	
		rereadSelectedPanel();
		writeSelectorPosition(selectedPanel, '>');

		writeStatusBarf("Disk copy complete.");
	}
	else
	{
		writeStatusBarf("Disk copy aborted.");
	}
}
