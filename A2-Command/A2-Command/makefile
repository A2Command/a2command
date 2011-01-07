# Program declarations
CC := cc65.exe
CA := ca65.exe
LD := ld65.exe
CL := cl65.exe
T := apple2enh
JAVA := java
AC := AppleCommander-1.3.5.jar
DISK := a2command.dsk
MASTER_DISK := PRODOS203.DSK
EMULATOR := C:/AppleWin/Applewin.exe -d1 $(CURDIR)/$(DISK)

CONFIG_SOURCES = \
	ConfigurationManager.c \
	Configuration.c \
	globals.c \
	screen.c \
	globalInput.c

SOURCES	= \
	A2-Command.c \
	A2-disks.c \
	Configuration.c \
	DriveMenu.c \
	drives.c \
	FileMenu.c \
	globals.c \
	input.c \
	menus.c \
	screen.c  \
	globalInput.c \
	Viewer.c

ASMFILES= \
#	res/common/driveRoutines.s \
#	res/$(T)/PlatformSpecific.s

PROGRAM	=a2cmd.060803
CONFIG_PROGRAM =a2cmdcfg.060803

TITLE	=$(PROGRAM:.060803=)
CONFIG_TITLE =$(CONFIG_PROGRAM:.060803=)

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
	$(JAVA) -jar $(AC) -p $(DISK) $(TITLE).system sys 0 < loader.system
	$(JAVA) -jar $(AC) -cc65 $(DISK) $(TITLE) bin       < $(T)/$(PROGRAM)
#	$(JAVA) -jar $(AC) -cc65 cffa.po $(TITLE) bin       < $(T)/$(PROGRAM)

$(T):
	mkdir $(T)

# Build our program
$(PROGRAM): $(SOURCES) $(ASMFILES) | $(T)
	$(CL) -O -t $(T) -C apple2enh-loader.cfg -o $(T)/$(PROGRAM) -m $(T)/$(TITLE).map $(SOURCES) $(ASMFILES) $(LIBS)
	$(CL) -O -t $(T) -o $(T)/$(CONFIG_PROGRAM) -m $(T)/$(CONFIG_TITLE).map $(CONFIG_SOURCES) $(ASMFILES) $(LIBS)

# Our phony targets for managing the project
#.PHONY: clean
clean: 
	$(RM) $(SOURCES:.c=.o) $(SOURCES:.c=.s)
	$(RM) $(CONFIG_SOURCES:.c=.o) $(CONFIG_SOURCES:.c=.s)
	$(RM) $(ASMFILES:.s=.o)
	$(RM) $(T)/$(PROGRAM)
	$(RM) $(T)/$(CONFIG_PROGRAM)
	$(RM) $(DISK)
