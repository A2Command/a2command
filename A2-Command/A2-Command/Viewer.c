/***************************************************************
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

* Neither the name of Payton Byrd nor the names of its
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
#include <unistd.h>
#include <fcntl.h>

#include "Viewer.h"
#include "globalInput.h"
#include "globals.h"
#include "screen.h"
#include "menus.h"
#include "drives.h"

#ifndef __fastcall
#define __fastcall __fastcall__
#endif

#pragma code-name("TEXTVIEW");
#pragma rodata-name("TEXTVIEW");
//#pragma data-name ("TVDATA");
//#pragma bss-name ("TVBSS");

#define BUFFERSIZE (sizeof fileBuffer)

void  writeHelpPanel(void)
{
	viewFile("a2cmdhelp.txt");
}

void __fastcall viewFile(
	const char *filename)
{
	char line[81];
	char word[81];
	int r, i;
	char last, character = '\0';
	bool printLine = false;
	unsigned char
		counter = 0,
		currentLine = 1;
	int file;

	file = open(filename, O_RDONLY);
	saveScreen();

	if(file != -1)
	{
		clrscr();

		memset(word, line[0] = '\0', sizeof word);
		do
		{
			r = read(file, fileBuffer, BUFFERSIZE);

			for(i=0; i<r; i++)
			{
				last = character;
				character = fileBuffer[i];
				if(character == '\n' ||
					character == '\r')
				{
					if(
						(character == '\n' && last != '\r') ||
						(character == '\r' && last != '\n'))
					{
						printLine = true;

						strcat(line, word);
						memset(word, counter = 0, sizeof word);
					}
				}
				else
				{
                    if ((word[counter++] = character) == ' ')
                    {
                        strcat(line, word);
                        memset(word, counter = 0, sizeof word);
                    }
                    if (strlen(line) + counter > size_x)
                    {
                        printLine = true;
                        
                        if (line[0] == '\0')
                        {
                            strcat(line, word);
                            memset(word, counter = 0, sizeof word);
                        }
                    }
				}

				if(printLine)
				{
					cputsxy(0, currentLine, line);
					printLine = false;
					line[0] = '\0';

					if(++currentLine == size_y)
					{
						writeStatusBar(
							"RETURN to continue, ESC to stop");
						if(waitForEnterEsc() == CH_ESC)
						{
							retrieveScreen();
							close(file);
							return;
						}
						else
						{
							(void)textcolor(color_text_other);
							clrscr();
							currentLine = 1;
						}
					}
				}
			} // FOR
		}
		while(r == BUFFERSIZE);

		// Flush the last line.
		cputsxy(0, currentLine, line);
		cputs(word);

		waitForEnterEscf("Done reading :%s", filename);
	}

	retrieveScreen();

	close(file);
}
