
#ifndef __HEX_EDITOR_H
#define __HEX_EDITOR_H

void processLine(char *readBuffer, off_t at, int bytesRead, char y);
void display(int bytesPerLine, char *filename, off_t startAt);

void hexEditCurrentFile(struct panel_drive *panel);

#endif