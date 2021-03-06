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
#include <stdarg.h>
#include <stdbool.h>
#include <conio.h>
#include <string.h>

#include "constants.h"
#include "globals.h"
#include "screen.h"

unsigned char waitForEnterEsc(void)
{
	unsigned char key = 0;

	revers(true); 
	//textcolor(color_text_other); 
	cputsxy(size_x - 5, 0, "(RET)"); 
	revers(false);

	while(key != CH_ESC
		&& key != CH_ENTER)
	{
		key = cgetc();
	}
	
	return key;
}

unsigned char waitForEnterEscf(const char* format, ...)
{
	va_list ap;

	va_start(ap,  format);
	vwriteStatusBarf(format, ap);
	va_end(ap);

	return waitForEnterEsc();
}

unsigned char* trimString(unsigned char *source)
{
	unsigned char i = 0, bufferPosition = 0;
	unsigned char trimBuffer[81];
	bool atStart = true;

	// Trim Right
	while(strlen(source) > 0 && source[strlen(source) - 1] == ' ') source[strlen(source)-1] = '\0';

	// Trim left
	for(i = 0; i<strlen(source); ++i)
	{
		if(!atStart)
		{
			trimBuffer[bufferPosition++] = source[i];
		}
		else if(source[i] != ' ')
		{
			atStart = false;
			trimBuffer[bufferPosition++] = source[i];
		}
	}
	trimBuffer[bufferPosition] = '\0';

	strcpy(source, trimBuffer);

	return source;
}