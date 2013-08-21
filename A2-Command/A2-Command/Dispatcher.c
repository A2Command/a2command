#include <string.h>
#include <fcntl.h>
#include <unistd.h>

extern void _TEXTVIEW_LOAD__[], _TEXTVIEW_SIZE__[];
extern void _DISKCOPY_LOAD__[], _DISKCOPY_SIZE__[];
extern void _DISKIMGS_LOAD__[], _DISKIMGS_SIZE__[];

void loadOverlay(char * name)
{
	static void *addr;
	static void *size;
	static int file;

	if(strcmp(name, "a2cmd.system.TV") == 0)
	{
		addr = _TEXTVIEW_LOAD__;
		size = _TEXTVIEW_SIZE__;
	}

	if(strcmp(name, "a2cmd.system.DC") == 0)
	{
		addr = _DISKCOPY_LOAD__;
		size = _DISKCOPY_SIZE__;
	}

	if(strcmp(name, "a2cmd.system.DI") == 0)
	{
		addr = _DISKIMGS_LOAD__;
		size = _DISKIMGS_SIZE__;
	}

    file = open (name, O_RDONLY);
    if (file == -1) {
        return;
    }
    read (file, addr, (unsigned) size);
    close (file);
}