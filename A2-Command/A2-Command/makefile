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

LIBS	=$(T).lib

# Default target - Builds for the Apple 2 Enhanced
all: $(PROGRAM)

A2Debug: T:=apple2enh
A2Debug: A2Disk
	$(EMULATOR)

A2Disk: T:=apple2enh
A2Disk: all
	cp $(MASTER_DISK) $(DISK)
#	cp cffa_main.po cffa.po
#	$(JAVA) -jar $(AC) -p $(DISK) $(TITLE).system sys 0 < loader.system
#	$(JAVA) -jar $(AC) -p cffa.po $(TITLE).system sys 0 < loader.system
#	$(JAVA) -jar $(AC) -cc65 $(DISK) $(TITLE) bin       < $(T)/$(PROGRAM)
#	$(JAVA) -jar $(AC) -cc65 cffa.po $(TITLE) bin	    < $(T)/$(PROGRAM)
	$(JAVA) -jar $(AC) -p $(DISK) $(TITLE).system sys 0 < $(T)/$(PROGRAM)
	$(JAVA) -jar $(AC) -p $(DISK) $(TITLE).TV bin 0 < $(T)/$(PROGRAM).TV
	$(JAVA) -jar $(AC) -p $(DISK) $(TITLE).DC bin 0 < $(T)/$(PROGRAM).DC
	$(JAVA) -jar $(AC) -p $(DISK) $(TITLE).DI bin 0 < $(T)/$(PROGRAM).DI
#	$(JAVA) -jar $(AC) -p cffa.po $(TITLE).system sys 0 < $(T)/$(PROGRAM)

$(T):
	mkdir $(T)

# Build our program
$(PROGRAM): $(SOURCES) $(ASMFILES) | $(T)
#	$(CL) -O -t $(T) -o $(T)/$(PROGRAM) $(SOURCES) $(ASMFILES) $(LIBS)
#	$(CL) -O -t $(T) -C apple2enh-loader.cfg -m $(T)/$(TITLE).map -o $(T)/$(PROGRAM) $(SOURCES) $(ASMFILES) $(LIBS)
	$(CL) -O -t $(T) -C apple2enh-system.cfg -m $(T)/$(TITLE).map -o $(T)/$(PROGRAM) $(SOURCES) $(ASMFILES) $(LIBS)

# Our phony targets for managing the project
#.PHONY: clean
clean: 
	$(RM) $(SOURCES:.c=.o) $(SOURCES:.c=.s)
	$(RM) $(T)/$(PROGRAM)
	$(RM) $(DISK)
