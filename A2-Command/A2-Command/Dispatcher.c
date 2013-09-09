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
extern void _FILEOPS_LOAD__[], _FILEOPS_SIZE__[];
extern void _HEXEDIT_LOAD__[], _HEXEDIT_SIZE__[];

#ifdef __APPLE2ENH__
#define PROGNAME "A2CMD"
#else
#define PROGNAME "A2CMDP"
#endif

bool loadOverlay(unsigned char name)
{
	static void *addr;
	static void *size;
	static int file;
	static unsigned char lastOverlay = 0;

	if(lastOverlay != name)
	{
		switch(name)
		{
			case 1:
				addr = _TEXTVIEW_LOAD__;
				size = _TEXTVIEW_SIZE__;
				file = open (PROGNAME ".TV", O_RDONLY);
				break;
			case 2:
				addr = _DISKCOPY_LOAD__;
				size = _DISKCOPY_SIZE__;
				file = open (PROGNAME ".DC", O_RDONLY);
				break;
			case 3:
				addr = _DISKIMGS_LOAD__;
				size = _DISKIMGS_SIZE__;
				file = open (PROGNAME ".DI", O_RDONLY);
				break;
			case 4:
				addr = _FILEOPS_LOAD__;
				size = _FILEOPS_SIZE__;
				file = open (PROGNAME ".FO", O_RDONLY);
				break;
			case 5:
				addr = _HEXEDIT_LOAD__;
				size = _HEXEDIT_SIZE__;
				file = open (PROGNAME ".HE", O_RDONLY);
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
	}
	return true;
}