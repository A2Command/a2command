/***************************************************************
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

#include "Viewer.h"
#include "Configuration.h"
#include "globalInput.h"
#include "globals.h"
#include "screen.h"
#include "menus.h"

#define BUFFERSIZE (sizeof fileBuffer)

void viewFile(
	const char *filename)
{
	char file[22];
	char line[81];
	char word[81];
	int r, i;
	char last, character = '\0';
	bool printLine = false;
	unsigned char
		counter = 0,
		currentLine = 1;
	FILE *fileStream;

	fileStream = fopen(filename, "rb");
	saveScreen();

	if(file != NULL)
	{
		clrscr();

		memset(word, line[0] = '\0', sizeof word);
		do
		{
			r = fread(fileBuffer, 1, BUFFERSIZE, fileStream);

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
					if ((word[counter] = character) == ' ')
					{
						strcat(line, word);
						memset(word, counter = 0, sizeof word);
					}
					else if (strlen(line) + (++counter) >= size_x)
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
							fclose(fileStream);
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

	fclose(fileStream);
}
