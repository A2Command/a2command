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
	AssemblerMethods.c \
	A2-Command.c \
	CommandMenu.c \
	Configuration.c \
	DriveMenu.c \
	drives.c \
	FileMenu.c \
	globals.c \
	input.c \
	menus.c \
	OptionMenu.c \
	screen.c  \
	globalInput.c

ASMFILES= \
#	res/common/driveRoutines.s \
#	res/$(T)/PlatformSpecific.s

PROGRAM	=a2command.060803
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
	$(JAVA) -jar $(AC) -cc65 $(DISK) $(TITLE) bin < $(T)/$(PROGRAM)

$(T):
	mkdir $(T)

# Build our program
$(PROGRAM): $(SOURCES) $(ASMFILES) | $(T)
	$(CL) -O -t $(T) -o $(T)/$(PROGRAM) $(SOURCES) $(ASMFILES) $(LIBS)
	$(CL) -O -t $(T) -o $(T)/$(CONFIG_PROGRAM) $(CONFIG_SOURCES) $(ASMFILES) $(LIBS)

# Our phony targets for managing the project
#.PHONY: clean
clean: 
	$(RM) $(SOURCES:.c=.o) $(SOURCES:.c=.s)
	$(RM) $(CONFIG_SOURCES:.c=.o) $(CONFIG_SOURCES:.c=.s)
	$(RM) $(ASMFILES:.s=.o)
	$(RM) $(T)/$(PROGRAM)
	$(RM) $(T)/$(CONFIG_PROGRAM)
	$(RM) $(DISK)
