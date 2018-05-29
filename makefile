PROJ = monochrome

IDIRS = include
SRCDIR = src
BUILDDIR = build
RELDIR = build/release
OBJDIR = build/obj
RELOBJDIR = build/release/obj
_LIBDIRS = lib
LIBDIRS = $(foreach LIBDIR,$(_LIBDIRS),-L$(LIBDIR))
_WINLIBDIRS = winlib
WINLIBDIRS = $(foreach LIBDIR,$(_WINLIBDIRS),-L$(LIBDIR))
DIRS = $(foreach DIR,$(shell find ./src -type d -printf '%d\t%P\n' | sort -r -nk1 | cut -f2-),mkdir $(OBJDIR)/$(DIR) &&) :

CC = clang
CCDB = lldb
CFLAGS = $(foreach DIR,$(IDIRS),-I$(DIR))
DBFLAGS = -g -D_DEBUG -O0

_LIBS = GLEW glfw GL m assimp kazmath GLU IL ILU
LIBS = $(foreach LIB,$(_LIBS),-l$(LIB))

DEPS = $(shell find $(IDIRS) -name *.h)

OBJ = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(shell find $(SRCDIR) -name *.c))

RELOBJ = $(patsubst $(SRCDIR)/%.c,$(RELOBJDIR)/%.o,$(OBJ))

$(OBJDIR)/%.o : $(SRCDIR)/%.c $(DEPS)
	$(CC) $(CFLAGS) $(DBFLAGS) -c -o $@ $<

.PHONY: build

build : $(OBJ)
	$(CC) $(CFLAGS) $(DBFLAGS) $(LIBDIRS) -o $(BUILDDIR)/$(PROJ) $^ $(LIBS)

$(RELOBJDIR)/%.o : $(SRCDIR)/%.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: release

release : $(RELOBJ)
	$(CC) $(CFLAGS) $(LIBDIRS) -o $(RELDIR)/$(PROJ) $^ $(LIBS)

.PHONY: clean

clean:
	rm -f -r {$(RELOBJDIR),$(RELDIR),$(BUILDDIR),$(OBJDIR)}
	mkdir {$(BUILDDIR),$(OBJDIR),$(RELDIR),$(RELOBJDIR)}
	$(DIRS)

.PHONY: run

run : build
	$(BUILDDIR)/$(PROJ)

SUPPRESSIONS = monochrome.supp

.PHONY: leakcheck

leakcheck : build
	valgrind --leak-check=full --suppressions=$(SUPPRESSIONS) $(BUILDDIR)/$(PROJ)

.PHONY: relrun

relrun : release
	$(RELDIR)/$(PROJ)

.PHONY: debug

debug : build
	$(CCDB) $(BUILDDIR)/$(PROJ)

.PHONY: rebuild

rebuild : clean build

WINCC = x86_64-w64-mingw32-clang
WINFLAGS = -DGLFW_DLL -I./vendor -I/usr/local/include -Wl,-subsystem,windows
_WINLIBS = glew32 glfw3dll opengl32 assimp kazmath glu32 DevIL ILU pthread
WINLIBS = $(foreach LIB,$(_WINLIBS),-l$(LIB))

WINOBJ = $(patsubst %.o,%.obj,$(OBJ))

$(OBJDIR)/%.obj : $(SRCDIR)/%.c $(DEPS)
	$(WINCC) $(CFLAGS) $(WINFLAGS) -c -o $@ $<

.PHONY: windows

windows : $(WINOBJ)
	$(WINCC) $(CFLAGS) $(WINFLAGS) $(WINLIBDIRS) -o $(BUILDDIR)/$(PROJ).exe $^ $(WINLIBS)
	cp winlib/* $(BUILDDIR)/
	cp $(BUILDDIR)/libassimp.dll $(BUILDDIR)/assimp.dll

.PHONY: wine

wine : windows
	wine $(BUILDDIR)/$(PROJ).exe
