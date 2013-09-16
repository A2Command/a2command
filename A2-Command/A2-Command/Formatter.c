#include "Formatter.h"
#include "drives.h"
#include "screen.h"
#include "globalInput.h"

#include <stdlib.h>
#include <stdio.h>
#include <conio.h>

#pragma code-name("FORMAT")

void formatDisk(struct panel_drive *panel)
{
	clrscr();
	FORMATTER();
	waitForEnterEsc();
	GETYN();
}