# Program declarations
CC := cc65
CA := ca65
LD := ld65
CL := cl65
T := apple2enh
JAVA := java
AC := AppleCommander-1.3.5.jar
DISK := a2command.po
MASTER_DISK := prodos.po
EMULATOR := C:/AppleWin/Applewin.exe -d1 $(CURDIR)/$(DISK)
JACE := java -jar /Applications/Apple\ II/jace.jar -computer.s6card disk -s6.d1 $(CURDIR)/$(DISK)

SOURCES	= \
	A2-Command.c \
	A2-disks.c \
	DriveMenu.c \
	drives.c \
	FileMenu.c \
	globals.c \
	input.c \
	menus.c \
	screen.c  \
	globalInput.c \
	Viewer.c \
    DiskImages.c \
    DiskCopy.c \
    Dispatcher.c

PROGRAM	=a2cmd

TITLE	=$(PROGRAM)

LIBS	:=$(T).lib

# Default target - Builds for the Apple 2 Enhanced
all: $(PROGRAM)

A2Plus: T := apple2
A2Plus: LIBS :=$(T).lib
A2Plus: $(PROGRAM)

A2Debug: A2Disk
	$(EMULATOR)

A2JACE: A2Disk
	$(JACE)

A2Disk: all
	cp $(MASTER_DISK) $(DISK)
	$(JAVA) -jar $(AC) -p $(DISK) $(TITLE).system sys 0 < $(T)/$(PROGRAM)
	$(JAVA) -jar $(AC) -p $(DISK) $(TITLE).TV bin 0 < $(T)/$(PROGRAM).TV
	$(JAVA) -jar $(AC) -p $(DISK) $(TITLE).DC bin 0 < $(T)/$(PROGRAM).DC
	$(JAVA) -jar $(AC) -p $(DISK) $(TITLE).DI bin 0 < $(T)/$(PROGRAM).DI

A2PlusDisk: T := apple2
A2PlusDisk: LIBS :=$(T).lib
A2PlusDisk: MASTER_DISK := prodos111.po
A2PlusDisk: A2Plus
	cp $(MASTER_DISK) $(DISK)
	$(JAVA) -jar $(AC) -p $(DISK) $(TITLE).system sys 0 < $(T)/$(PROGRAM)
	$(JAVA) -jar $(AC) -p $(DISK) $(TITLE).TV bin 0 < $(T)/$(PROGRAM).TV
	$(JAVA) -jar $(AC) -p $(DISK) $(TITLE).DC bin 0 < $(T)/$(PROGRAM).DC
	$(JAVA) -jar $(AC) -p $(DISK) $(TITLE).DI bin 0 < $(T)/$(PROGRAM).DI

$(T):
	mkdir $(T)

# Build our program
$(PROGRAM): $(SOURCES) $(ASMFILES) | $(T)
	$(CL) -O -t $(T) -C apple2enh-system.cfg -m $(T)/$(TITLE).map -o $(T)/$(PROGRAM) $(SOURCES) $(ASMFILES) $(LIBS)

# Our phony targets for managing the project
#.PHONY: clean
clean: 
	$(RM) $(SOURCES:.c=.o) $(SOURCES:.c=.s)
	$(RM) $(T)/$(PROGRAM)
	$(RM) $(DISK)
