#include "drives.h"

#pragma code-name("INITDRVS")
#pragma rodata-name("INITDRVS")

void initializeDrives(void)
{
	static unsigned char i = 0;

	if(!areDrivesInitialized)
	{
		leftPanelDrive.drive = 0;
		leftPanelDrive.currentIndex = 0;
		leftPanelDrive.displayStartAt = 0;
		leftPanelDrive.position = left;
		
		for(i=0; i<SLIDING_WINDOW_SIZE; ++i)
		{
			leftPanelDrive.slidingWindow[i].size = 0u;
			leftPanelDrive.slidingWindow[i].type = 0;
		}

		rightPanelDrive.drive = 0;
		rightPanelDrive.currentIndex = 0;
		rightPanelDrive.displayStartAt = 0;
		rightPanelDrive.position = right;
		
		for(i=0; i<SLIDING_WINDOW_SIZE; ++i)
		{
			rightPanelDrive.slidingWindow[i].size = 0u;
			rightPanelDrive.slidingWindow[i].type = 0;
		}

		areDrivesInitialized = true;
		selectedPanel = &leftPanelDrive;
	}
}
