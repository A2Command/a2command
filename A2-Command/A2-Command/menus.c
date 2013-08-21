/**************************************************************
Copyright (c) 2010, Payton Byrd
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

//unsigned char currentMenuX;
//unsigned char currentMenuLine;

// Writes the menu bar at the top of the screen
// which is scaled to the current screen size.
void writeMenuBar(void)
{
	static unsigned char bottom = 0;
	
	bottom = size_y - 1;
	
	cclearxy(0, bottom, size_x);
	cputsxy(0, bottom, " HELP      QUIT    SELECT   REFRESH   COPY    RENAME    MAKE DIRECTORY    DELETE");

	revers(true);
	cputcxy(0, bottom, '1');
	cputcxy(10, bottom, '2');
	cputcxy(18, bottom, '3');
	cputcxy(27, bottom, '4');
	cputcxy(37, bottom, '5');
	cputcxy(45, bottom, '6');
	cputcxy(55, bottom, '7');
	cputcxy(73, bottom, '8');

	revers(false);
}

