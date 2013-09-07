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
//#pragma data-name("DCDATA");

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
#else
					"Copied %ld/%ld blocks. (%ld%%)",
#endif
					counter + 1, 
					sectorCount - counter + 1,
					(unsigned long)(counter * (unsigned long)100) / sectorCount);
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
