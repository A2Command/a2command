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
	cursor(1);
	FORMATTER();
	cursor(0);
}