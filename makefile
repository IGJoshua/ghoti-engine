PROJ = monochrome
LIBNAME = monochrome

IDIRS = include/arch include/game vendor

SRCDIR = src
ARCHDIR = $(SRCDIR)/arch
GAMEDIR = $(SRCDIR)/game

BUILDDIR = build
OBJDIR = $(BUILDDIR)/obj
ARCHOBJDIR = $(OBJDIR)/arch
GAMEOBJDIR = $(OBJDIR)/game

_LIBDIRS = lib
LIBDIRS = $(foreach LIBDIR,$(_LIBDIRS),-L$(LIBDIR))

LUALIBDIR = lualib

_WINLIBDIRS = winlib
WINLIBDIRS = $(foreach LIBDIR,$(_WINLIBDIRS),-L$(LIBDIR))

ARCHDIRS = $(foreach DIR,$(shell find $(ARCHDIR) -type d -printf '%d\t%P\n' | sort -r -nk1 | cut -f2-),mkdir $(ARCHOBJDIR)/$(DIR) &&) :
GAMEDIRS = $(foreach DIR,$(shell find $(GAMEDIR) -type d -printf '%d\t%P\n' | sort -r -nk1 | cut -f2-),mkdir $(GAMEOBJDIR)/$(DIR) &&) :

CC = clang
CCDB = lldb
CFLAGS = $(foreach DIR,$(IDIRS),-I$(DIR))
DBFLAGS = -g -D_DEBUG -O0 -Wall
RELFLAGS = -O3
SHAREDFLAGS = -shared

_LIBS = GLEW glfw GL m assimp kazmath GLU IL ILU luajit-5.1 SDL2 cjson frozen json-utilities
LIBS = $(foreach LIB,$(_LIBS),-l$(LIB))

VENDORDEPS = $(shell find vendor -name *.h)
ARCHDEPS = $(shell find include/arch -name *.h)
GAMEDEPS = $(shell find include/game -name *.h)

ARCHOBJ = $(patsubst $(ARCHDIR)/%.c,$(ARCHOBJDIR)/%.o,$(shell find $(ARCHDIR) -name *.c))
GAMEOBJ = $(patsubst $(GAMEDIR)/%.c,$(GAMEOBJDIR)/%.o,$(shell find $(GAMEDIR) -name *.c))

$(ARCHOBJDIR)/%.o : $(ARCHDIR)/%.c $(ARCHDEPS) $(VENDORDEPS)
	$(CC) $(CFLAGS) $(if $(RELEASE),$(RELFLAGS),$(DBFLAGS)) -c -o $@ $<

$(GAMEOBJDIR)/%.o : $(GAMEDIR)/%.c $(GAMEDEPS) $(ARCHDEPS) $(VENDORDEPS)
	$(CC) $(CFLAGS) $(if $(RELEASE),$(RELFLAGS),$(DBFLAGS)) -c -o $@ $<

.PHONY: build

build : $(GAMEOBJ) $(LIBNAME).so
	$(CC) $(CFLAGS) $(if $(RELEASE),$(RELFLAGS),$(DBFLAGS)) $(LIBDIRS) -o $(BUILDDIR)/$(PROJ) $^ $(LIBS)

$(BUILDDIR)/$(LIBNAME).so : $(ARCHOBJ)
	$(CC) $(CFLAGS) $(if $(RELEASE),$(RELFLAGS),$(DBFLAGS)) $(LIBDIRS) $(SHAREDFLAGS) -o $@ $^ $(LIBS)

$(LIBNAME).so : $(BUILDDIR)/$(LIBNAME).so
	ln -sf $(BUILDDIR)/$(LIBNAME).so $(LIBNAME).so

.PHONY: arch

arch : $(LIBNAME).so

.PHONY: clean

clean:
	rm -rf release
	rm -rf {$(ARCHOBJDIR),$(GAMEOBJDIR),$(OBJDIR),$(BUILDDIR)}
	rm -f $(LIBNAME).{so,dll}
	mkdir {$(BUILDDIR),$(OBJDIR),$(ARCHOBJDIR),$(GAMEOBJDIR)}
	$(ARCHDIRS)
	$(GAMEDIRS)

.PHONY: run

run : build
	LD_LIBRARY_PATH=.:./lib $(BUILDDIR)/$(PROJ)

SUPPRESSIONS = monochrome.supp

.PHONY: leakcheck

leakcheck : build
	LD_LIBRARY_PATH=.:./lib valgrind --leak-check=full --track-origins=yes --suppressions=$(SUPPRESSIONS) $(BUILDDIR)/$(PROJ)

.PHONY: debug

debug : build
	LD_LIBRARY_PATH=.:./lib $(CCDB) $(BUILDDIR)/$(PROJ)

.PHONY: rebuild

rebuild : clean build

.PHONY: release

release : clean
	@make RELEASE=yes$(if $(WINDOWS), windows,)
	mkdir release/
	find build/* -type f -not -path '*/obj/*' -exec cp {} release/ \;
	$(if $(WINDOWS),,mv release/$(PROJ) release/$(PROJ)-bin)
	cp -r resources/ release/
	cp -r lualib/ release/
	$(if $(WINDOWS),,cp -r lib/ release/)
	$(if $(WINDOWS),,echo '#!/bin/bash' > release/$(PROJ) && echo 'LD_LIBRARY_PATH=.:./lib ./$(PROJ)-bin' >> release/$(PROJ) && chmod +x release/$(PROJ))

WINCC = x86_64-w64-mingw32-clang
WINFLAGS = -DGLFW_DLL -I/usr/local/include -Wl,-subsystem,windows
_WINLIBS = glew32 glfw3dll opengl32 assimp kazmath glu32 DevIL ILU pthread luajit mingw32 SDL2main SDL2 cjson frozen json-utilities
WINLIBS = $(foreach LIB,$(_WINLIBS),-l$(LIB))

WINARCHOBJ = $(patsubst %.o,%.obj,$(ARCHOBJ))
WINGAMEOBJ = $(patsubst %.o,%.obj,$(GAMEOBJ))

$(ARCHOBJDIR)/%.obj : $(ARCHDIR)/%.c $(ARCHDEPS) $(VENDORDEPS)
	$(WINCC) $(CFLAGS) $(if $(RELEASE),$(RELFLAGS),$(DBFLAGS)) $(WINFLAGS) -c -o $@ $<

$(GAMEOBJDIR)/%.obj : $(GAMEDIR)/%.c $(GAMEDEPS) $(ARCHDEPS) $(VENDORDEPS)
	$(WINCC) $(CFLAGS) $(if $(RELEASE),$(RELFLAGS),$(DBFLAGS)) $(WINFLAGS) -c -o $@ $<

$(BUILDDIR)/$(LIBNAME).dll : $(WINARCHOBJ)
	$(WINCC) $(CFLAGS) $(if $(RELEASE),$(RELFLAGS),$(DBFLAGS)) $(WINFLAGS) $(WINLIBDIRS) $(SHAREDFLAGS) -o $@ $^ $(WINLIBS)

$(LIBNAME).dll : $(BUILDDIR)/$(LIBNAME).dll
	ln -s $(BUILDDIR)/$(LIBNAME).dll $(LIBNAME).dll

.PHONY: windows

windows : $(WINGAMEOBJ) $(LIBNAME).dll
	$(WINCC) $(CFLAGS) $(if $(RELEASE),$(RELFLAGS),$(DBFLAGS)) $(WINFLAGS) $(WINLIBDIRS) -o $(BUILDDIR)/$(PROJ).exe $^ $(WINLIBS)
	cp winlib/* $(BUILDDIR)/
	cp $(BUILDDIR)/libassimp.dll $(BUILDDIR)/assimp.dll

.PHONY: wine

wine : windows $(LIBNAME).dll
	wine $(BUILDDIR)/$(PROJ).exe
