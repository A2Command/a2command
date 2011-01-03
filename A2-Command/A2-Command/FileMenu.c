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
#include <errno.h>
#include <peekpoke.h>
#include <unistd.h>

#include "Configuration.h"
#include "constants.h"
#include "globalInput.h"
#include "globals.h"
#include "drives.h"
#include "input.h"
#include "menus.h"
#include "screen.h"

unsigned char *quit_message[1] =
{
	"Quit A2-Command?"
};
	

void  writeHelpPanel(void)
{
	writeStatusBar("See http://a2command.codeplex.com/documentation");
}

unsigned char fileBuffer[COPY_BUFFER_SIZE];
struct panel_drive *targetPanel = NULL, *tempPanel = NULL;
void  copyFiles(void)
{
#if defined(__C128__) || defined(__C64__)
	unsigned char i = 0, j = 0, sd = 0, td = 0, bit = 0, r = 0;
	unsigned int index = 0, bytes = 0;
	unsigned RELOAD = false;
	unsigned char targetFilename[21], type[2], status[40];
	struct dir_node *currentNode;

	if(selectedPanel == &leftPanelDrive)
	{
		targetPanel = &rightPanelDrive;
	}
	else
	{
		targetPanel = &leftPanelDrive;
	}

	sd = selectedPanel->drive->drive;
	td = targetPanel->drive->drive;

	if(sd == td)
	{
		saveScreen();
		writeStatusBar("Cannot copy to the same drive.");
		waitForEnterEsc();
		retrieveScreen();
		return;
	}

	for(i=0; i<selectedPanel->length / 8 + 1; ++i)
	{
		for(j=0; j<8; ++j)
		{
			bit = 1 << j;
			r = selectedPanel->selectedEntries[i] & bit;
			if(r != 0)
			{
				currentNode = getSpecificNode(selectedPanel, i*8+j);
				if(currentNode->type < 4)
				{
					if(currentNode == NULL)
					{
						getDirectory(selectedPanel, i*8+j);
						currentNode = getSpecificNode(selectedPanel, i*8+j);
						{
							if(currentNode == NULL)
							{
								writeStatusBarf("Cannot get file %u", i*8+j); 
								waitForEnterEsc();
								return;
							}
						}
					}

					cbm_open(15, sd, 15, "");
					r = cbm_open(1, sd, 2, currentNode->name);					
					if(r == 0)
					{
						sprintf(type, "%c", getFileType(currentNode->type));
						strlower(type);
						sprintf(targetFilename,"@0:%s,%s,w",currentNode->name,type);
						cbm_open(14,td,15,"");
						r = cbm_open(2, td, 3, targetFilename);
						if(r == 0)
						{
							for(index=0; index < currentNode->size; index+=(COPY_BUFFER_SIZE/254))
							{
								bytes = cbm_read(1, fileBuffer, COPY_BUFFER_SIZE);
								if(bytes == -1)
								{
									writeStatusBarf("Problem (%d) reading %s", 
										_oserror, 
										currentNode->name); 
									waitForEnterEsc();
									cbm_read(15, status, 40);
									writeStatusBar(status); waitForEnterEsc();
									break;
								}
								else if(bytes == EOF)
								{
									break;
								}

								if(kbhit())
								{
									r = cgetc();
									if(r == CH_ESC || r == CH_STOP)
									{
										cbm_close(2); 
										cbm_close(1);
										cbm_close(15);
										cbm_close(14);

										reloadPanels();

										writeStatusBar("Aborted copy.");
										return;
									}

								}

								r = cbm_write(2, fileBuffer, bytes);
								if(r == -1)
								{
									writeStatusBarf("Problem (%d) writing %s", 
										_oserror, 
										currentNode->name); 
									waitForEnterEsc();
									cbm_read(14, status, 40);
									writeStatusBar(status); waitForEnterEsc();
									break;
								}
								writeStatusBarf("%s - %d of %d.", currentNode->name, index, currentNode->size);
							}
							RELOAD = true;
						}
						else
						{
							writeStatusBarf("Cannot open %s for write (%d)", 
								currentNode->name, r); 
							waitForEnterEsc();
							cbm_read(14, status, 40);
							writeStatusBar(status); waitForEnterEsc();
						}
					}
					else
					{
						writeStatusBarf("Cannot open %s for read (%d)", 
							currentNode->name, r); 
						waitForEnterEsc();
						cbm_read(15, status, 40);
						writeStatusBar(status); waitForEnterEsc();
					}
					cbm_close(2); 
					cbm_close(1);
					cbm_close(15);
					cbm_close(14);
				}
			}
		}
	}
	if(RELOAD == true)
	{
		reloadPanels();
	}
#endif
}

void  reloadPanels(void)
{
	tempPanel = selectedPanel;
	selectedPanel = targetPanel;
	rereadSelectedPanel();
	selectedPanel = tempPanel;
	rereadSelectedPanel();
	writeSelectorPosition(selectedPanel, '>');
	writeSelectorPosition(targetPanel, ' ');
	writeCurrentFilename(selectedPanel);
}

void  renameFile(void)
{
	enum results dialogResult;
	struct dir_node *selectedNode = NULL;
	unsigned char command[40];
	unsigned char filename[17];
	unsigned char* dialogMessage[] =
	{
		{ "Enter new name" },
		{ "for file:" }
	};

	if(selectedPanel != NULL)
	{
		selectedNode = getSelectedNode(selectedPanel);
		if(selectedNode != NULL)
		{
			saveScreen();

			writeStatusBarf("Old name: %s", selectedNode->name);

			dialogResult = drawInputDialog(
				2, 16,
				dialogMessage,
				"Rename File",
				filename);

			retrieveScreen();

			if(dialogResult == OK_RESULT)
			{
				writeStatusBarf("Renaming to %s", filename);

				sprintf(command, "r0:%s=%s",
					filename, selectedNode->name);

				sendCommand(selectedPanel, command);

				rereadSelectedPanel();

				writeStatusBarf("Renamed to %s", filename);
			}
		}
	}
}

void  makeDirectory(void)
{
	enum results dialogResult;
	struct dir_node *selectedNode = NULL;
	unsigned char command[40];
	unsigned char filename[17];
	unsigned char* dialogMessage[] =
	{
		{ "Enter name for" },
		{ "new directory:" }
	};

	if(selectedPanel != NULL)
	{
			saveScreen();

			dialogResult = drawInputDialog(
				2, 16,
				dialogMessage,
				"New Directory",
				filename);

			retrieveScreen();

			if(dialogResult == OK_RESULT)
			{
				strcpy(command, selectedPanel->path);
				strcat(command, filename);
				if(command[strlen(command)-1] != '/')
				{
					strcat(command, "/");
				}

				mkdir(command);

				getDirectory(selectedPanel, 
					selectedPanel->slidingWindowStartAt);
				displayDirectory(selectedPanel);
			}
	}
}

void  deleteFiles(void)
{
	unsigned dialogResult;
	struct dir_node *selectedNode = NULL;
	unsigned char command[40];
	unsigned char* dialogMessage[] =
	{
		{ "Are you sure?" }
	};

	if(selectedPanel != NULL)
	{
		selectedNode = getSelectedNode(selectedPanel);
		if(selectedNode != NULL)
		{
			saveScreen();

			writeStatusBarf("File to delete: %s", selectedNode->name);

			dialogResult = writeYesNo(
				"Delete File",
				dialogMessage,
				1);

			retrieveScreen();

			if(dialogResult == true)
			{
				writeStatusBarf("Deleting %s", selectedNode->name);

				if(selectedNode->type != 6)
				{
					sprintf(command, "s0:%s", selectedNode->name);
				}
				else
				{
					sprintf(command, "rd:%s", selectedNode->name);
				}

				sendCommand(selectedPanel, command);

				rereadSelectedPanel();
			}
		}
	}
}

void  quit(void)
{
	unsigned result;

	saveScreen();

	result = writeYesNo("Confirm", quit_message, 1);
	
	if(result == true)
	{
		writeStatusBar("Goodbye!");
		exit(EXIT_SUCCESS);
	}

	retrieveScreen();
}

void  writeAboutBox(void)
{
	writeStatusBarf("Thank You for using A2-Command Alpa");
}

void  executeSelectedFile(void)
{
	struct dir_node *currentNode;
	unsigned result;

	if(selectedPanel != NULL)
	{
		currentNode = getSelectedNode(selectedPanel);

		if(currentNode != NULL && currentNode->type == 2)
		{
			saveScreen();

			result = writeYesNo("Confirm", quit_message, 1);

			retrieveScreen();
	
			if(result == true)
			{
				clrscr();
				exit(EXIT_SUCCESS);
			}
		}
	}
}

void  inputCommand(void)
{
	enum results dialogResult;
	struct dir_node *selectedNode = NULL;
	unsigned char command[77];
	unsigned char* dialogMessage[] =
	{
		{ "Enter Command for drive:" }
	};

	if(selectedPanel != NULL)
	{
		selectedNode = getSelectedNode(selectedPanel);
		if(selectedNode != NULL)
		{
			saveScreen();

			dialogResult = drawInputDialog(
				1, 
				size_x - 4,
				dialogMessage,
				"Command",
				command
				);

			retrieveScreen();

			if(dialogResult == OK_RESULT)
			{
				sendCommand(selectedPanel, command);

				waitForEnterEsc();

				rereadSelectedPanel();				
			}
		}
	}
}
