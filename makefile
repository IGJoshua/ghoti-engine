PROJ = ghoti
LIBNAME = ghoti

CONFIG_FILE = $(PROJ).json
DEFAULT_FONT = default_font.ttf

IDIRS = include/arch include/game vendor

SRCDIR = src
ARCHDIR = $(SRCDIR)/arch
GAMEDIR = $(SRCDIR)/game

BUILDDIR = build
OBJDIR = $(BUILDDIR)/obj
ARCHOBJDIR = $(OBJDIR)/arch
GAMEOBJDIR = $(OBJDIR)/game

_LIBDIRS = lib
LIBDIRS = $(foreach LIBDIR,$(_LIBDIRS),-L$(LIBDIR) -Wl,-rpath-link,$(LIBDIR))

_WINLIBDIRS = winlib winlib/static
WINLIBDIRS = $(foreach LIBDIR,$(_WINLIBDIRS),-L$(LIBDIR))

ARCHDIRS = $(foreach DIR,$(shell find $(ARCHDIR) -type d -printf '%d\t%P\n' | sort -r -nk1 | cut -f2-),mkdir $(ARCHOBJDIR)/$(DIR) &&) :
GAMEDIRS = $(foreach DIR,$(shell find $(GAMEDIR) -type d -printf '%d\t%P\n' | sort -r -nk1 | cut -f2-),mkdir $(GAMEOBJDIR)/$(DIR) &&) :

CC = clang
CCDB = lldb
CFLAGS = $(foreach DIR,$(IDIRS),-I$(DIR)) -fPIC
DBFLAGS = -g -D_DEBUG -O0 -Wall
RELFLAGS = -O3
SHAREDFLAGS = -shared

RELEASE_RESOURCES_FOLDER = release/resources
RELEASE_SCRIPTS_FOLDER = $(RELEASE_RESOURCES_FOLDER)/scripts
RELEASE_INIT_FILE = $(RELEASE_SCRIPTS_FOLDER)/init.lua
RELEASE_DEBUG_FILES = $(RELEASE_SCRIPTS_FOLDER)/components/debug $(RELEASE_SCRIPTS_FOLDER)/systems/debug

_LIBS = json-utilities cjson frozen glfw GLEW GLU GL ILU IL luajit-5.1 kazmath m SDL2 ode
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

SUPPRESSIONS = $(PROJ).supp

.PHONY: clean

clean:
	rm -rf release
	rm -rf $(BUILDDIR)
	rm -f $(LIBNAME).so
	rm -f $(LIBNAME).dll
	mkdir -p $(ARCHOBJDIR)
	mkdir -p $(GAMEOBJDIR)
	$(ARCHDIRS)
	$(GAMEDIRS)
	touch local-$(SUPPRESSIONS)
	./clean_resources.sh

.PHONY: run

run : build
	LD_LIBRARY_PATH=.:./lib $(BUILDDIR)/$(PROJ)

.PHONY: leakcheck

leakcheck : build
	LD_LIBRARY_PATH=.:./lib valgrind --leak-check=full --track-origins=yes --suppressions=$(SUPPRESSIONS) --suppressions=local-$(SUPPRESSIONS) --gen-suppressions=$(if $(GENSUPPRESSIONS),$(GENSUPPRESSIONS),no) $(if $(VGDB),--vgdb=yes --vgdb-error=0,) $(BUILDDIR)/$(PROJ)

.PHONY: callgrind

callgrind : build
	LD_LIBRARY_PATH=.:./lib valgrind --tool=callgrind --branch-sim=yes $(BUILDDIR)/$(PROJ)

.PHONY: cachegrind

cachegrind : build
	LD_LIBRARY_PATH=.:./lib valgrind --tool=cachegrind $(BUILDDIR)/$(PROJ)

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
	find $(RELEASE_RESOURCES_FOLDER)/fonts/* -type f -not -path '*$(DEFAULT_FONT)' -exec rm {} \;
	cp -r doc/ release/
	cp $(CONFIG_FILE) release/
	rm -rf $(RELEASE_RESOURCES_FOLDER)/audio/*
	rm -rf $(RELEASE_RESOURCES_FOLDER)/heightmaps/*
	rm -rf $(RELEASE_RESOURCES_FOLDER)/images/*
	rm -rf $(RELEASE_RESOURCES_FOLDER)/models/*
	rm -rf $(RELEASE_RESOURCES_FOLDER)/scenes/*
	mkdir -p $(RELEASE_RESOURCES_FOLDER)/saves/
	rm -rf $(RELEASE_RESOURCES_FOLDER)/saves/*
	rm $(RELEASE_INIT_FILE)
	echo "-- NOTE(Joshua): This is where you load the main scene" > $(RELEASE_INIT_FILE) && echo "" >> $(RELEASE_INIT_FILE) && echo "math.randomseed(os.time())" >> $(RELEASE_INIT_FILE) && echo "" >> $(RELEASE_INIT_FILE) && echo "local C = engine.C" >> $(RELEASE_INIT_FILE)
	rm -rf $(RELEASE_DEBUG_FILES) && echo "" >> $(RELEASE_INIT_FILE)
	$(if $(WINDOWS),,cp -r lib/ release/)
	$(if $(WINDOWS),,echo '#!/bin/bash' > release/$(PROJ) && echo 'LD_LIBRARY_PATH=.:./lib ./$(PROJ)-bin' >> release/$(PROJ) && chmod +x release/$(PROJ))

WINCC = x86_64-w64-mingw32-clang
WINCFLAGS = $(foreach DIR,$(IDIRS),-I$(DIR))
WINFLAGS = -I/usr/local/include -Wl,-subsystem,windows
_WINLIBS = mingw32 SDL2main json-utilities cjson frozen glfw3 glew32 glu32 opengl32 ILU DevIL kazmath pthread luajit ode-6 SDL2

WINLIBS = $(foreach LIB,$(_WINLIBS),-l$(LIB))

WINARCHOBJ = $(patsubst %.o,%.obj,$(ARCHOBJ))
WINGAMEOBJ = $(patsubst %.o,%.obj,$(GAMEOBJ))

$(ARCHOBJDIR)/%.obj : $(ARCHDIR)/%.c $(ARCHDEPS) $(VENDORDEPS)
	$(WINCC) $(WINCFLAGS) $(if $(RELEASE),$(RELFLAGS),$(DBFLAGS)) $(WINFLAGS) -c -o $@ $<

$(GAMEOBJDIR)/%.obj : $(GAMEDIR)/%.c $(GAMEDEPS) $(ARCHDEPS) $(VENDORDEPS)
	$(WINCC) $(WINCFLAGS) $(if $(RELEASE),$(RELFLAGS),$(DBFLAGS)) $(WINFLAGS) -c -o $@ $<

$(BUILDDIR)/$(LIBNAME).dll : $(WINARCHOBJ)
	$(WINCC) $(WINCFLAGS) $(if $(RELEASE),$(RELFLAGS),$(DBFLAGS)) $(WINFLAGS) $(WINLIBDIRS) $(SHAREDFLAGS) -o $@ $^ $(WINLIBS)

$(LIBNAME).dll : $(BUILDDIR)/$(LIBNAME).dll
	ln -sf $(BUILDDIR)/$(LIBNAME).dll $(LIBNAME).dll

.PHONY: windows

windows : $(WINGAMEOBJ) $(LIBNAME).dll
	$(WINCC) $(WINCFLAGS) $(if $(RELEASE),$(RELFLAGS),$(DBFLAGS)) $(WINFLAGS) $(WINLIBDIRS) -o $(BUILDDIR)/$(PROJ).exe $^ $(WINLIBS)
	find winlib/* -not -path '*/static*' -exec cp {} $(BUILDDIR)/ \;

.PHONY: wine

wine : windows $(LIBNAME).dll
	wine $(BUILDDIR)/$(PROJ).exe
