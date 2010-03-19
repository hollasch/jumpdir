
srhlib = ..\srhlib
includes = -I$(srhlib)

DEBUG=1

!if !defined(DEBUG)
DEBUG=0
!elseif ($(DEBUG) == 0)
DEBUG=0
!else
DEBUG=1
!endif

!if ($(DEBUG))
cflags = -nologo -EHsc -W4 -WX -Zi $(includes) -DDEBUG=1
!else
cflags = -nologo -EHsc -W4 -WX -Ox -GF -GS $(includes)
!endif

all: jumpdir

#-----------------------------------------------------------------------------

jumpdir: jumpdir.exe

jumpdir.exe: jumpdir.cpp pathmatcher.obj wildcomp.obj
    cl -Fejumpdir.exe $(cflags) jumpdir.cpp pathmatcher.obj wildcomp.obj

wildcomp.obj: $(srhlib)\wildcomp.h $(srhlib)\wildcomp.cpp
    cl -c $(cflags) $(srhlib)\wildcomp.cpp

pathmatcher.obj: $(srhlib)\pathmatcher.h $(srhlib)\pathmatcher.cpp
    cl -c $(cflags) $(srhlib)\pathmatcher.cpp

#-----------------------------------------------------------------------------

clean:
    -del >nul 2>&1 /q *.obj

clobber: clean
    -del >nul 2>&1 /q *.exe

fresh: clobber all

#-----------------------------------------------------------------------------

install: $(BINDIR)\jumpdir.exe $(BINDIR)\j.cmd

$(BINDIR)\jumpdir.exe: jumpdir.exe
    if defined BINDIR copy /y $? %%BINDIR%%

$(BINDIR)\j.cmd: j.cmd
    if defined BINDIR copy /y $? %%BINDIR%%
