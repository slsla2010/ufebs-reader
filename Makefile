CPP      = g++.exe
CC       = gcc.exe
WINDRES  = windres.exe
RES      = Menu.obj 
RESR     = Menu-rus.res
OBJ      = Main.o Lib.o Action.o DelKZ.o $(RES)
OBJR     = Main.o $(RESR)
LINKOBJ  = Main.o Lib.o Action.o DelKZ.o $(RES)
LINKOBJR = Main.o $(RESR)
LIBS     = -L"C:/temp/MinGW32/mingw32/lib32;c:/temp/mingw32/libexec;." libxml2.lib -lregex  -static-libgcc -m32 -s -mwindows 
INCS     = -I"C:/temp/MinGW32/include"
CXXINCS  = -I"C:/TEMP/MinGW32/x86_64-w64-mingw32/include"
BIN      = ur.exe
BINR     = EdExtractorR.exe
CXXFLAGS = $(CXXINCS)  -march=native -mtune=native -msse -O1 -std=c99 -m32 -mwindows
CFLAGS   = $(INCS)  -march=native -mtune=native -msse -std=c99 -O1 -m32 -mwindows -fno-use-linker-plugin
RM       = rm -f

.PHONY: all all-before all-after clean clean-custom

all: all-before $(BIN) all-after


clean: clean-custom
	${RM} $(OBJ) $(BIN)

$(BIN): $(OBJ)
	$(CC) $(LINKOBJ) -o $(BIN) $(LIBS) -fno-use-linker-plugin
	strip $(BIN)

$(BINR): $(OBJR)
	$(CC) $(LINKOBJR) -o $(BINR) $(LIBS)
	strip $(BINR)

#Ctl3d.o:	ctl3d.c
#	$(CC) -c $< -o $@ $(CFLAGS)

Lib.o:	Lib.c Main.h
	$(CC) -c $< -o $@ $(CFLAGS)

Action.o:	Action.c Main.h
	$(CC) -c $< -o $@ $(CFLAGS)

DelKZ.o:	DelKZ.c Main.h  rules.h
	$(CC) -c $< -o $@ $(CFLAGS)

Main.o: Main.c
	$(CC) -c Main.c -o Main.o $(CFLAGS)

buildtime.rc: Main.c
	 buildtime

Menu.obj:	Menu.rc buildtime.rc datdlg.rc
	GoRC /o Menu.rc 

#Menu.res:  Menu.rc buildtime.rc  datdlg.rc 
#	$(WINDRES) -i Menu.rc -F pe-i386  --input-format=rc -o Menu.res -O coff  

$(RESR):   Menu-rus.rc
	$(WINDRES) -i Menu-rus.rc -F pe-i386 --input-format=rc -o Menu-rus.res -O coff -fno-use-linker-plugin 

arc:	Main.c Action.c DelKZ.c Lib.c main.h menu.rc buildtime.rc
	rar u -r -p ur_src *.c *.h *.rc Makefile.win make.cmd *.lib bmp\icons50x48.bmp 