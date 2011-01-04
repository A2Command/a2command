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
#include <conio.h>
#include <peekpoke.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "Configuration-Apple2Enh.h"
#include "screen.h"
#include "Configuration.h"
#include "constants.h"
#include "globalInput.h"
#include "globals.h"
#include "PlatformSpecific.h"
#include "input.h"

unsigned char SCREEN_BUFFER[2][24][40];

// Prepares the screen 
void setupScreen(void)
{
	clrscr();

	//textcolor(color_text_other);
	//bgcolor(color_background);
	//bordercolor(color_border);

	return;
}

void  saveScreen(void)
{
	unsigned char i;

	for(i=0; i<24; ++i)
	{
		gotoy(i);
		memcpy(SCREEN_BUFFER[0][i], *(char **)0x28, 40);
		if(size_x == 80)
		{
			*(char *)0xC055 = 0;
			memcpy(SCREEN_BUFFER[1][i], *(char **)0x28, 40);
			*(char *)0xC054 = 0;
		}
	}
}

void  retrieveScreen(void)
{
	unsigned char i;

	for(i=0; i<24; ++i)
	{
		gotoy(i);
		memcpy(*(char **)0x28, SCREEN_BUFFER[0][i], 40);
		if(size_x == 80)
		{
			*(char *)0xC055 = 0;
			memcpy(*(char **)0x28, SCREEN_BUFFER[1][i], 40);
			*(char *)0xC054 = 0;
		}
	}
}

void writeStatusBar(const char message[])
{
	unsigned char oldX, oldY;

	oldX = wherex();
	oldY = wherey();

	//textcolor(color_text_status);
	revers(true);

	cclearxy(0, 0, size_x);

	cputsxy(0, 0, message);
	
	revers(false);

	gotoxy(oldY, oldY);
}

void drawBox(
	unsigned char x,
	unsigned char y,
	unsigned char w,
	unsigned char h,
	unsigned char color,
	unsigned reverse)
{
	unsigned char i;
	
	//textcolor(color);
	revers(reverse);

	// draw frame
	textframexy(x, y, w + 1, h + 1, TEXTFRAME_TALL);

        // draw body
	for(i=y+1; i<y+h; ++i)
	{
		cclearxy(x + 1, i, w - 1);
	}
}

unsigned char  getCenterX(unsigned char w)
{
	return (size_x / 2) - (w / 2) - 1;
}

unsigned char  getCenterY(unsigned char h)
{
	return (size_y / 2) - (h / 2) - 1;
}

void writePanel(
	unsigned drawBorder,
	unsigned reverse,
	unsigned char color,
	unsigned char x, unsigned char y,
	unsigned char h, unsigned char w,
	unsigned char *title,
	unsigned char *cancel,
	unsigned char *ok)
{
	unsigned int i = 0, okLeft = 0, cancelLeft = 0;
	unsigned char buffer[80];

	saveScreen();

	//textcolor(color);
	revers(reverse);

	if(drawBorder)
	{
		drawBox(x, y, w, h, color, reverse);
	}
	else
	{
		//strncpy(buffer, SPACES, w);
		//buffer[w] = '\0';

		for(i=0; i<h; ++i)
		{
			//cputsxy(x, y+i,buffer);
			cclearxy(x,y+i,w);
		}
	}

	if(title != NULL)
	{
		if(strlen(title) > w-4)
		{
			title[w-4]='\0';
		}

		sprintf(buffer, "[%s]", title);
		cputsxy(x+2, y,buffer);
	}

	revers(false);

	okLeft = x + w - 2;
	if(ok != NULL)
	{
		sprintf(buffer, "[%s]", ok);
		okLeft -= strlen(buffer);
		cputsxy(okLeft, y + h - 1, buffer);
	}

	cancelLeft = okLeft - 2;
	if(cancel != NULL)
	{
		sprintf(buffer, "[%s]", cancel);
		cancelLeft -= strlen(buffer);
		cputsxy(cancelLeft, y + h - 1,buffer);
	}
}

void  notImplemented(void)
{
	//unsigned char h = 5, w = 23;
	//unsigned char x, y;

	//saveScreen();

	//x = getCenterX(w);
	//y = getCenterY(h);

	//writePanel(true, true, color_border, x, y, h, w,
	//	"Sorry...", "OK", NULL);

	////textcolor(color_text_other);
	//revers(true);
	//cputsxy(x+2, y+2, "Not yet implemented.");

	//waitForEnterEsc();

	//retrieveScreen();
	saveScreen();
	writeStatusBar("Not implemented...");
	waitForEnterEsc();
	retrieveScreen();
}

enum results  drawDialog(
	unsigned char* message[],
	unsigned char lineCount,
	unsigned char* title,
	enum buttons button)
{
	unsigned char x = 0, y = 0, h = 0, w = 0, i = 0, key = 0, l = 0;
	unsigned char okButton[4];
	unsigned char cancelButton[7];

	h = lineCount + 5;
	w = 33;

	x = getCenterX(w);
	y = getCenterY(h);

	if(button & OK) 
	{
		strcpy(okButton, "OK");
	}

	if(button & YES)
	{
		strcpy(okButton, "Yes");
	}

	if(button & CANCEL)
	{
		strcpy(cancelButton, "Cancel");
	}

	if(button & NO)
	{
		strcpy(cancelButton, "No");
	}

	writePanel(
		true, false, color_text_borders,
		x, y, h, w,
		title,
		(button & NO || button & CANCEL ? cancelButton : NULL),
		(button & OK || button & YES ? okButton : NULL));

	for(i=0; i<lineCount; ++i)
	{
		//textcolor(color_text_other);
		cputsxy(x+2, i+2+y,message[i]);
	}	

	while(true)
	{
		key = cgetc();

		if(key == CH_ENTER) break;
		if(key == CH_ESC 
			) break;
		if((key == 'o'|| key == 'O') && button & OK) break;
		if((key == 'y'|| key == 'Y') && button & YES) break;
		if((key == 'c'|| key == 'C') && button & CANCEL) break;
		if((key == 'n'|| key == 'N') && button & NO) break;
	}

	switch((int)key)
	{
	case CH_ESC: 
	case (int)'n': case (int)'N':
	case (int)'c': case (int)'C':
		if(button & NO) return NO_RESULT;
		if(button & CANCEL) return CANCEL_RESULT;
		break;

	case CH_ENTER: 
	case (int)'y': case (int)'Y':
	case (int)'o': case (int)'O':
		if(button & YES) return YES_RESULT;
		if(button & OK) return OK_RESULT;
		break;
	}

	return CANCEL_RESULT;
}

enum results  drawInputDialog(
	unsigned char lineCount,
	unsigned char length,
	unsigned char *message[],
	unsigned char *title,
	unsigned char *resultText)
{
	unsigned char x = 0, y = 0, h = 0, w = 0, i = 0, 
		key = 0, count = 0;
	unsigned result;
	unsigned char *input;
	input = calloc(length+1, sizeof(unsigned char));
	h = lineCount + 6;
	w = length + 3;
	//for(i=0; i<lineCount; ++i);
	//{
	//	if(strlen(message[i]) > w) 
	//		w = strlen(message[i]);
	//}

	x = getCenterX(w);
	y = getCenterY(h);

	writePanel(
		true, false, color_text_borders,
		x, y, h, w,
		title,
		"Cancel",
		"Done");

	for(i=0; i<lineCount; ++i)
	{
		//textcolor(color_text_other);
		cputsxy(x+2, i+2+y,message[i]);
	}	
	++i;
	
	gotoxy(x+2, i+2+y);
	revers(true);
	//textcolor(color_text_other);
	
	cclearxy(x+2, i+2+y, length + 1);
	cputcxy(x+2, i+2+y, '<');
	count = 0;
	key = cgetc();
	while(key != CH_ESC 
		&& key != CH_ENTER)
	{
		if( count < length &&
			(
				(key >= 32 && key <= 95) ||
				(key >= 65 + 0x80 && key <= 90 + 0x80)
			)
		)
		{
			input[count] = key;
			input[count+1] = '\0';
			gotoxy(x+2+count, i+2+y);
			cputc(key);
			++count;
			gotoxy(x+2+count, i+2+y);
			cputc('<');
		}
		else if(
			key == 127
			&& count > 0)
		{
			input[count] = '\0';
			gotoxy(x+2+count, i+2+y);
			cputc(' ');
			--count;
			gotoxy(x+2+count, i+2+y);
			cputc('<');
		}

		key = cgetc();
	}

	strcpy(resultText, input);

	switch((int)key)
	{
	case CH_ENTER: result = OK_RESULT; break;
	default: result = CANCEL_RESULT; break;
	}

	revers(false);
	free(input);
	return result;
}

unsigned  writeYesNo(
	unsigned char *title,
	unsigned char *message[],
	unsigned char lineCount)
{
	enum results result =
		drawDialog(message, lineCount, title, YES | NO);

	return result == YES_RESULT;
}

void vwriteStatusBarf(const char format[], va_list ap)
{
	char buffer[81];

	vsprintf(buffer, format, ap);
	writeStatusBar(buffer);
}

void writeStatusBarf(const char format[], ...)
{
	va_list ap;

	va_start(ap,  format);
	vwriteStatusBarf(format, ap);
	va_end(ap);
}

