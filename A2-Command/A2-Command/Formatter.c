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
	clrscr();
	SLOT = (panel->drive) << 4;
	DRV = ((panel->drive & 0x08) >> 3) + 1;
	writeStatusBarf("%02X %02X %02X", panel->drive, SLOT, DRV);
	FORMATTER();
	
}