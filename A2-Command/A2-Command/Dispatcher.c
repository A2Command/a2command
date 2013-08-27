#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>

#include "globals.h"
#include "globalInput.h"
#include "screen.h"

extern void _TEXTVIEW_LOAD__[], _TEXTVIEW_SIZE__[];
extern void _DISKCOPY_LOAD__[], _DISKCOPY_SIZE__[];
extern void _DISKIMGS_LOAD__[], _DISKIMGS_SIZE__[];

bool loadOverlay(char * name)
{
	static void *addr;
	static void *size;
	static int file;
	static char * buf;

	buf = (char*)malloc(MAX_PATH_LENGTH);

	strcpy(buf, exePath);
	strcat(buf, name);

	if(strcmp(name, ".TV") == 0)
	{
		addr = _TEXTVIEW_LOAD__;
		size = _TEXTVIEW_SIZE__;
	}

	if(strcmp(name, ".DC") == 0)
	{
		addr = _DISKCOPY_LOAD__;
		size = _DISKCOPY_SIZE__;
	}

	if(strcmp(name, ".DI") == 0)
	{
		addr = _DISKIMGS_LOAD__;
		size = _DISKIMGS_SIZE__;
	}

    file = open (buf, O_RDONLY);
	free(buf);
    if (file == -1) {
		waitForEnterEscf("Please insert A2Command disk back to original drive, then try again.");
		writeStatusBarf("");
        return false;
    }
    read (file, addr, (unsigned) size);
    close (file);
	return true;
}