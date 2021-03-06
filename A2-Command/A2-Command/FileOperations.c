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
#include <errno.h>
#include <peekpoke.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <fcntl.h>

#include "constants.h"
#include "globalInput.h"
#include "globals.h"
#include "drives.h"
#include "input.h"
#include "menus.h"
#include "screen.h"
#include "Viewer.h"

#pragma code-name("FILEOPS");
#pragma rodata-name("FILEOPS");
//#pragma data-name("FODATA");
//#pragma bss-name("FOBSS");

void copyFiles(void)
{
	int sourceFile = -1, targetFile = -1;
	unsigned numSelectors = selectedPanel->length / 8 + 1;
	static unsigned char sourcePath[MAX_PATH_LENGTH], targetPath[MAX_PATH_LENGTH];
	unsigned char i = 0, j = 0, sd = 0, td = 0, bit = 0;
	unsigned int index = 0;
	size_t bytes;
	unsigned RELOAD = false;
	size_t bytesCopied;
	size_t r;
	//static unsigned char targetFilename[21], type[2], status[40];
	struct dir_node *currentNode;
	unsigned long totalBytes = 0;
	bool multipleSelected = false;
	//clock_t timeStart, timeSpent;

	targetPanel = selectedPanel == &leftPanelDrive ? &rightPanelDrive : &leftPanelDrive;

	if(strcmp(selectedPanel->path, targetPanel->path) == 0 
		//&& targetPanel->drive->drive == selectedPanel->drive->drive
		)
	{
		saveScreen();
		writeStatusBar("Cannot copy to the same path.");
		waitForEnterEsc();
		retrieveScreen();
		return;
	}
    
    for(i=0; i<numSelectors; ++i)
    {
        if (selectedPanel->selectedEntries[i] != 0x00)
        {
            multipleSelected = true;
            break;
        }
	}
	if(!multipleSelected)
	{
#ifdef __APPLE2ENH__
        writeStatusBar("No files selected, selecting current file.");
#else
		writeStatusBar("Selecting current file.");
#endif
		selectCurrentFile();
		writeSelectorPosition(selectedPanel, '>');
	}

	//timeStart = time(NULL);
	//timeStart = clock();
	for(i=0; i<selectedPanel->length / 8 + 1; ++i)
	{
		for(j=0; j<8; ++j)
		{
			if(i*8 + j > selectedPanel->length) break;
			bit = 1 << j;
			r = selectedPanel->selectedEntries[i] & bit;
			if(r != 0)
			{
				currentNode = getSpecificNode(selectedPanel, i*8+j);
				if(currentNode == NULL)
				{
					getDirectory(selectedPanel, i*8+j);
					currentNode = getSpecificNode(selectedPanel, i*8+j);
					{
						if(currentNode == NULL)
						{
							waitForEnterEscf("Cannot get file %u", i*8+j); 
							return;
						}
					}
				}

				if(currentNode->type != 0x0F)
				{
					writeStatusBarf("Copying file %s.....", currentNode->name);
					sprintf(sourcePath, "%s/%s", selectedPanel->path, currentNode->name);
					sourceFile = open(sourcePath, O_RDONLY);
					if(sourceFile != -1)
					{
						sprintf(targetPath, "%s/%s", targetPanel->path, currentNode->name);
						_filetype = currentNode->type;
						_auxtype = currentNode->aux_type;
						_datetime.createtime = currentNode->time;
						_datetime.createdate = currentNode->date;
						targetFile = open(targetPath, O_WRONLY | O_CREAT | O_TRUNC);
						if(targetFile != -1)
						{
							bytesCopied = 0;
							while(true)
							{
								bytes = read(sourceFile, fileBuffer, sizeof(fileBuffer));

								if(kbhit())
								{
									r = cgetc();
									if(r == CH_ESC)
									{
										close(sourceFile);
										close(targetFile);

										reloadPanels();

										writeStatusBar("Aborted copy.");
										return;
									}

								}

								if(bytes > 0)
								{
									r = write(targetFile, fileBuffer, bytes);
								
									if(r == -1)
									{
										writeStatusBarf("Problem (%d) writing %s", 
											_oserror, 
											currentNode->name); 
										waitForEnterEsc();
										break;
									}
									else
									{
										bytesCopied += bytes;
										totalBytes += bytes;
									}
								}

								////timeSpent = (time(NULL) - timeStart);
								//timeSpent = (clock() - timeStart)/CLOCKS_PER_SEC;
								//writeStatusBarf("%u:%02u e.t. %d B/s",
								//	(unsigned)timeSpent/60u,
								//	(unsigned)timeSpent%60u,
								//	(unsigned)((totalBytes +=
								//		(unsigned long)bytes)/timeSpent));

								writeStatusBarf("%-15s - %8u bytes copied",
									currentNode->name, bytesCopied);

								if(bytes < sizeof(fileBuffer))
								{
									break;
								}
							}
							RELOAD = true;
						}
					}
					else
					{
#ifdef __APPLE2ENH__
						writeStatusBarf("Cannot open %s for read (%d)",
#else
						writeStatusBarf("Cannot open %s (%d)",
#endif
							currentNode->name, r);
						waitForEnterEsc();
					}

					writeStatusBarf("%-15s - %8u bytes copied",
						currentNode->name, bytesCopied);
					close(sourceFile);
					close(targetFile);
				}
				else
				{
					writeStatusBarf("Skipping directory %s", currentNode->name);
				}
			}
		}
	}

	////timeSpent = (time(NULL) - timeStart);
	//timeSpent = (clock() - timeStart)/CLOCKS_PER_SEC;
	//writeStatusBarf("%u:%02u e.t. %d B/s",
	//	(unsigned)timeSpent/60u,
	//	(unsigned)timeSpent%60u,
	//	(unsigned)(totalBytes/timeSpent));

	if(RELOAD == true)
	{
		reloadPanels();
	}
}

unsigned char oldName[MAX_PATH_LENGTH], newName[MAX_PATH_LENGTH];
void renameFile(void)
{
	enum results dialogResult;
	struct dir_node *selectedNode = NULL;
	unsigned char filename[16];
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
				2, 15,
				dialogMessage,
				"Rename File",
				filename);

			retrieveScreen();

			if(dialogResult == OK_RESULT)
			{
				writeStatusBarf("Renaming to %s", filename);

				sprintf(oldName, "%s/%s", selectedPanel->path, selectedNode->name);
				sprintf(newName, "%s/%s", selectedPanel->path, filename);

				rename(oldName, newName);

				rereadSelectedPanel();

				writeStatusBarf("Renamed to %s", filename);
			}
		}
	}
}

void makeDirectory(void)
{
	enum results dialogResult;
	unsigned char command[65];
	unsigned char filename[16];
	unsigned char* dialogMessage[] =
	{
		{ "Enter name for" },
		{ "new directory:" }
	};

	if(selectedPanel != NULL)
	{
			saveScreen();

			dialogResult = drawInputDialog(
				2, 15,
				dialogMessage,
				"New Directory",
				filename);

			retrieveScreen();

			if(dialogResult == OK_RESULT)
			{
				if(strlen(selectedPanel->path) + 1 + strlen(filename) < 65)
				{
					sprintf(command, "%s/%s", selectedPanel->path, filename);

					mkdir(command);

					getDirectory(selectedPanel, 
						selectedPanel->slidingWindowStartAt);

					displayDirectory(selectedPanel);
					writeSelectorPosition(selectedPanel, '>');
					writeCurrentFilename(selectedPanel);
				}
				else
				{
					writeStatusBarf("Cannot create directory, path too long.");
				}
			}
	}
}

#ifdef __APPLE2ENH__
#define AREYOUSURE "Are you sure?"
#else
#define AREYOUSURE "ARE YOU SURE?"
#endif

void deleteFiles(void)
{
	struct dir_node *selectedNode;
	unsigned i, k, l;
	unsigned char j;
	bool dialogResult, isBatch = false;
	static unsigned char* dialogMessage[] =
	{
		{ AREYOUSURE }
	};
	struct dirent *currentDE;
	DIR *dir = NULL;

	saveScreen();
	dialogResult = writeYesNo(
		"Delete",
		dialogMessage,
		1);
	retrieveScreen();

	if(dialogResult)
	{
		dir = opendir(selectedPanel->path);

		writeStatusBar("Deleting files...");
		l = selectedPanel->length;
		for(k=0; k<l; ++k)
		{
			currentDE = readdir(dir);
			i = k / 8;
			j = k % 8;

			if ((selectedPanel->selectedEntries[i] & (1 << j)) != 0x00)
				//&& (k = i*8+j) < selectedPanel->length)
			{
				isBatch = true;

				sprintf(oldName, "%s/%s", selectedPanel->path, currentDE->d_name);
				writeStatusBarf("Deleting %-15s",currentDE->d_name);
					
				if (remove(oldName) < 0)
				{
#ifdef __APPLE2ENH__					
					waitForEnterEscf("%s removing %s.", _stroserror(_oserror), currentDE->d_name);
#else
					waitForEnterEscf("Error removing %s.", currentDE->d_name);
#endif
				}

				if(kbhit() && cgetc() == CH_ESC)
				{
					waitForEnterEscf("Aborted.");
					rereadSelectedPanel();
					writeSelectorPosition(selectedPanel, '>');
					writeCurrentFilename(selectedPanel);
					return;
				}
			}
		}

		if(!isBatch)
		{
			// No names are highlighted; delete the current file.
			selectedNode = getSelectedNode(selectedPanel);
			if(selectedNode != NULL)
			{
				writeStatusBarf(
					"File to delete: %s",
					selectedNode->name);

				retrieveScreen();
				writeCurrentFilename(selectedPanel);

				sprintf(oldName, "%s/%s", selectedPanel->path, selectedNode->name);
				writeStatusBarf("Deleting %s.", selectedNode->name);
                if (remove(oldName) < 0)
				{
#ifdef __APPLE2ENH__
					waitForEnterEscf("%s removing %s.", _stroserror(_oserror), selectedNode->name);
#else
					waitForEnterEscf("Error removing %s.", selectedNode->name);
#endif
				}
				rereadSelectedPanel();
				writeSelectorPosition(selectedPanel, '>');
				writeCurrentFilename(selectedPanel);
			}
		}
		else
		{
			rereadSelectedPanel();
			writeSelectorPosition(selectedPanel, '>');
			writeCurrentFilename(selectedPanel);
		}

		closedir(dir);
	}

}

