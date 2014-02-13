#include "Formatter.h"
#include "drives.h"
#include "screen.h"
#include "globalInput.h"

#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <unistd.h>

#pragma code-name("FORMAT")

void formatDisk(struct panel_drive *panel)
{
	static unsigned char* confirm[] =
	{
		"This will erase all data",
		" on disk. Are you sure?"
	};
	
	static unsigned char* entername[] =
	{
		"Type a name for",
		"the disk"
	};
	static unsigned char newName[16], r;
	static bool yesNo;
	
	saveScreen();
	yesNo = writeYesNo("Format Disk", confirm, 2);
	retrieveScreen();
	
	if(yesNo)
	{
		newName[0] = '\0';
		r = drawInputDialog(2, 15, entername, "Format Disk", newName);
		if((unsigned char)r == OK_RESULT)
		{
			writeStatusBarf("Formatting Slot %u, Drive %u. Please wait",
							(panel->drive) & 7,
							(panel->drive >> 3) + 1);
			FORMATTER(((panel->drive & 0x08) >> 3) + 1, newName,(panel->drive) << 4);
		}
	}
	
}