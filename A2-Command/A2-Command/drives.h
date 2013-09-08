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

#ifndef _DRIVES_H
#define _DRIVES_H

#ifndef __fastcall
#define __fastcall __fastcall__
#endif

#include <stdbool.h>

#include "menus.h"

/* STRUCTS */

/* Directory Node
 *-----------------------------
 *- Payton Byrd
 *-----------------------------
 * Stores the name, type, size
 * and index of a section of the
 * sliding directory window.
 */
struct dir_node
{
	unsigned char name[16];
	unsigned char type;
	unsigned char access;
	unsigned int aux_type;
	unsigned int blocks;
	unsigned long size;
	int index;
	struct {
        unsigned day  :5;
        unsigned mon  :4;
        unsigned year :7;
    }             date;
    struct {
        unsigned char min;
        unsigned char hour;
    }             time;
};

/* Panel Drive
 *-----------------------------
 *- Payton Byrd
 *-----------------------------
 * Contains the "moving parts"
 * of the application.  There are
 * two Panels declared, each with
 * it's own data for selected 
 * entries, the drive that is 
 * associated with the panel,
 * the directory header, the 30
 * directory nodes for the sliding
 * window, the total length of the 
 * directory, where the directory
 * display starts from, where the
 * sliding window starts from and
 * the position (left or right) of
 * the panel.
 */
struct panel_drive
{
	unsigned char* selectedEntries;
	unsigned char drive;
	unsigned char path[65];
	struct dir_node slidingWindow[SLIDING_WINDOW_SIZE + 1];
	int length;
	unsigned totalblocks;
	unsigned usedblocks;
	int currentIndex;
	int displayStartAt;
	int slidingWindowStartAt;
	enum menus position;
};

/* GLOBAL VARIABLES */
extern unsigned areDrivesInitialized;		// Has the drives been intialized?
extern struct panel_drive leftPanelDrive;	// The left panel
extern struct panel_drive rightPanelDrive;	// The right panel
extern struct panel_drive *selectedPanel;	// The current panel
extern struct panel_drive *targetPanel;

/* METHODS */
void initializeDrives(void);

void  __fastcall listDrives(enum menus menu);

int  __fastcall getDirectory(
	struct panel_drive *drive,
	const int slidingWindowStartAt);

void  __fastcall displayDirectory(
	struct panel_drive *drive);

void  __fastcall writeSelectorPosition(
	struct panel_drive *panel,
	unsigned char character);

void __fastcall  moveSelectorUp(
	struct panel_drive *panel);

void __fastcall  moveSelectorDown(
	struct panel_drive *panel);

void selectCurrentFile(void);

void __fastcall  writeCurrentFilename(
	struct panel_drive *panel);

void __fastcall  enterDirectory(
	struct panel_drive *panel);

void __fastcall  leaveDirectory(
	struct panel_drive *panel);

unsigned char __fastcall  getDiskImageType(
	struct panel_drive *panel);

bool __fastcall  isDirectory(
	struct panel_drive *panel);

struct dir_node* __fastcall  getSelectedNode(
	struct panel_drive *panel);

struct dir_node* __fastcall  getSpecificNode(
	struct panel_drive *panel, 
	const int index);

//unsigned char  sendCommand(
//	struct panel_drive *panel,
//	unsigned char *command);

void __fastcall  resetSelectedFiles(
	struct panel_drive *panel);

void __fastcall  selectAllFiles(
	struct panel_drive *panel, 
	const bool select);

void __fastcall  moveTop(
	struct panel_drive *panel);

void __fastcall  movePageUp(
	struct panel_drive *panel);

void __fastcall  movePageDown(
	struct panel_drive *panel);

void __fastcall moveBottom(
	struct panel_drive *panel);

	
extern unsigned char fileBuffer[COPY_BUFFER_SIZE];

#endif