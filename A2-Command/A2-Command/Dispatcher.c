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

bool loadOverlay(unsigned char name)
{
	static void *addr;
	static void *size;
	static int file;

	switch(name)
	{
		case 1:
			addr = _TEXTVIEW_LOAD__;
			size = _TEXTVIEW_SIZE__;
			file = open ("A2CMD.TV", O_RDONLY);
			break;
		case 2:
			addr = _DISKCOPY_LOAD__;
			size = _DISKCOPY_SIZE__;
			file = open ("A2CMD.DC", O_RDONLY);
			break;
		case 3:
			addr = _DISKIMGS_LOAD__;
			size = _DISKIMGS_SIZE__;
			file = open ("A2CMD.DI", O_RDONLY);
			break;
	}

    if (file == -1) {
#ifdef __APPLE2ENH__		
		waitForEnterEscf("Please insert A2Command disk back to original drive, then try again.");
#else
		waitForEnterEscf("Please insert A2Command disk.");
#endif
		writeStatusBarf("");
        return false;
    }
    read (file, addr, (unsigned) size);
    close (file);
	return true;
}