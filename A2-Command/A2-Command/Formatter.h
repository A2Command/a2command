#ifndef __FORMATTER_H
#define __FORMATTER_H

void formatDisk(struct panel_drive *panel);
extern void __fastcall__ FORMATTER(unsigned char DRV, unsigned char * VOLNAME, unsigned char SLOT);

#endif