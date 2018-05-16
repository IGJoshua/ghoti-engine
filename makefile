PROJ = monochrome

IDIRS = include
SRCDIR = src
BUILDDIR = build
RELDIR = build/release
OBJDIR = build/obj
RELOBJDIR = build/release/obj
LIBDIRS = -Llib
WINLIBDIRS = -Lwinlib

CC = clang
CCDB = lldb
CFLAGS = $(foreach DIR,$(IDIRS),-I$(DIR))
DBFLAGS = -g -D_DEBUG

_LIBS = GLEW glfw GL m assimp kazmath GLU IL ILU
LIBS = $(foreach LIB,$(_LIBS),-l$(LIB))

CORE_DEPS = defines.h core/window.h
RENDERER_DEPS = renderer/renderer_types.h renderer/renderer.h renderer/shader.h
ASSET_MANAGEMENT_DEPS = asset_management/asset_manager.h
DEPS = $(patsubst %,$(IDIRS)/%,$(CORE_DEPS)) $(patsubst %,$(IDIRS)/%,$(RENDERER_DEPS)) $(patsubst %,$(IDIRS)/%,$(ASSET_MANAGEMENT_DEPS))

_OBJ = core/main core/window renderer/shader renderer/renderer asset_management/asset_manager
OBJ = $(patsubst %,$(OBJDIR)/%.o,$(_OBJ))

RELOBJ = $(patsubst %,$(RELOBJDIR)/%.o,$(_OBJ))

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
	rm -f -r $(BUILDDIR)
	rm -f -r $(RELDIR)
	mkdir $(BUILDDIR)
	mkdir $(OBJDIR)
	mkdir $(OBJDIR)/core
	mkdir $(OBJDIR)/renderer
	mkdir $(OBJDIR)/asset_management
	mkdir $(RELDIR)
	mkdir $(RELOBJDIR)
	mkdir $(RELOBJDIR)/core
	mkdir $(RELOBJDIR)/renderer
	mkdir $(RELOBJDIR)/asset_management

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
_WINLIBS = glew32 glfw3dll opengl32 assimp kazmath glu32 DevIL ILU
WINLIBS = $(foreach LIB,$(_WINLIBS),-l$(LIB))

_WINOBJ = $(foreach O,$(_OBJ),$(O).obj)
WINOBJ = $(patsubst %,$(OBJDIR)/%,$(_WINOBJ))

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
