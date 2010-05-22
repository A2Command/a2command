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
#include <stdlib.h>
#include <stdio.h>
#include "Configuration-Apple2Enh.h"
#include <conio.h>
#include <errno.h>
#include <peekpoke.h>

#include "globalInput.h"
#include "globals.h"
#include "screen.h"

/* Drive Configuration */
unsigned char defaultLeftDrive = 8;
unsigned char defaultRightDrive = 8;

/* Color Configuration */
// The C= 128 defaults to different colors
// than the C= 64.
unsigned char color_background	= COLOR_BLACK;
unsigned char color_border		= COLOR_BLACK;
unsigned char color_selector	= COLOR_WHITE;
unsigned char color_text_borders= COLOR_WHITE;
unsigned char color_text_menus	= COLOR_WHITE;
unsigned char color_text_files	= COLOR_WHITE;
unsigned char color_text_status = COLOR_WHITE;
unsigned char color_text_other	= COLOR_WHITE;
unsigned char color_text_highlight = COLOR_WHITE;

/* Load configuration
 * --------------------------
 * - Payton Byrd 
 * --------------------------
 * Loads the configuration
 * from disk.
 */
void  load(void)
{
	loadA2E();
}

void loadA2E(void)
{ 
	return;
}

