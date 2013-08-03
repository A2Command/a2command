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

#ifndef _A2_DISKS_H
#define _A2_DISKS_H

#ifndef __fastcall
#define __fastcall __fastcall__
#endif

#include "drives.h"
#define MAX_PATH_LENGTH 256

extern unsigned char _driveCount;
extern unsigned char _devices[14];
extern unsigned char _fileTypes[256][4];

void __fastcall__ selectDrive(struct panel_drive *panel);
unsigned long __fastcall__ getDriveSize(unsigned char driveNumber);
void __fastcall__ writeDiskImage(void);
void __fastcall__ createDiskImage(void);
void __fastcall__ copyDisk(void);
void setupFileTypes(void);

#endif