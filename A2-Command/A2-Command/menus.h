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

#ifndef _MENUS_H
#define _MENUS_H

// Menu Globals
extern unsigned char currentMenuX;
extern unsigned char currentMenuLine;
enum menus
{
	left, right, file, command, options
};


// Menu actions
void  inputCommand(void);
void  executeSelectedFile(void);
void  rereadDrivePanel(enum menus menu);
void  rereadSelectedPanel(void);
void  reloadPanels(void);
void  writeDriveSelectionPanel(enum menus menu);
void  writeAboutBox(void);
void  displayPanels(void);
void  writeMenu(enum menus);
void  writeOptionsPanel(void);
void  saveOptions(void);
void  swapPanels(void);
void  togglePanels(void);
void  writeHelpPanel(void);
void  copyFiles(void);
void  renameFile(void);
void  makeDirectory(void);
void  deleteFiles(void);
void  writeD64(void);
void  createD64(void);
void  quit(void);

// Menu drawing
void  drawFileMenu(unsigned char);
void  drawDriveMenu(unsigned char);
void  drawCommandMenu(unsigned char);
void  drawOptionsMenu(unsigned char);
void  drawMenu(unsigned char, unsigned char, unsigned char, char*[], unsigned char[]);
void  drawMenuLine(unsigned, unsigned char, char*, unsigned char, unsigned char);
void writeMenuBar(void);
#endif