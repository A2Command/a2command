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

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <string.h>

#include "constants.h"
#include "globals.h"
#include "input.h"
#include "menus.h"
#include "screen.h"

#ifdef __APPLE2ENH__
unsigned char menuEntries[8][9] = {"HELP", "QUIT", "SELECT", "REFRESH", "COPY", "RENAME", "MAKEDIR", "DELETE"};
#else
unsigned char menuEntries[8][4] = {"HLP", "END", "SEL", "REF", "CPY", "REN", "MDR", "DEL"};
#endif

// Writes the menu bar at the top of the screen
// which is scaled to the current screen size.
void writeMenuBar(void)
{
	static unsigned char bottom = 0, i, x;
	
	bottom = size_y - 1;
	
	cclearxy(0, bottom, size_x);	
	
    for(i = 0; i < 8; i++){
#ifdef __APPLE2ENH__
        x = i*10;
#else
		x = i*5;
#endif
        revers(true);
        gotoxy(x, bottom); cputc('1' + i);
        revers(false);
        cputsxy(x + 1, bottom, menuEntries[i]);
    }
	
	revers(false);
}

