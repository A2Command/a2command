# Program declarations
CC := cc65.exe
CA := ca65.exe
LD := ld65.exe
CL := cl65.exe
T := apple2enh
JAVA := java
AC := AppleCommander-1.3.5.jar
DISK := a2command.po
MASTER_DISK := prodos.po
EMULATOR := C:/AppleWin/Applewin.exe -d1 $(CURDIR)/$(DISK)

SOURCES	= \
	AssemblerMethods.c \
	A2-Command.c \
	A2-disks.c \
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
	globalInput.c \
	Viewer.c

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
	$(JAVA) -jar $(AC) -p $(DISK) $(TITLE).system sys 0 < loader.system
#	$(JAVA) -jar $(AC) -p cffa.po $(TITLE).system sys 0 < loader.system
	$(JAVA) -jar $(AC) -cc65 $(DISK) $(TITLE) bin       < $(T)/$(PROGRAM)
#	$(JAVA) -jar $(AC) -cc65 cffa.po $(TITLE) bin		< $(T)/$(PROGRAM)

$(T):
	mkdir $(T)

# Build our program
$(PROGRAM): $(SOURCES) $(ASMFILES) | $(T)
	$(CL) -O -t $(T) -o $(T)/$(PROGRAM) $(SOURCES) $(ASMFILES) $(LIBS)

# Our phony targets for managing the project
#.PHONY: clean
clean: 
	$(RM) $(SOURCES:.c=.o) $(SOURCES:.c=.s)
	$(RM) $(T)/$(PROGRAM)
	$(RM) $(DISK)
