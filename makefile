PROJ = monochrome

IDIRS = include
SRCDIR = src
BUILDDIR = build
OBJDIR = build/obj
LIBDIRS = -Llib

CC = clang
CFLAGS = $(foreach DIR,$(IDIRS),-I$(DIR))

_LIBS = glfw GL m
LIBS = $(foreach LIB,$(_LIBS),-l$(LIB))

CORE_DEPS = defines.h core/window.h
RENDERER_DEPS = renderer/renderer_types.h renderer/mesh.h
ASSET_MANAGEMENT_DEPS = asset_management/asset_management_types.h
DEPS = $(patsubst %,$(IDIRS)/%,$(CORE_DEPS)) $(patsubst %,$(IDIRS)/%,$(RENDERER_DEPS)) $(patsubst %,$(IDIRS)/%,$(ASSET_MANAGEMENT_DEPS))

_OBJ = core/main core/window renderer/mesh
OBJ = $(patsubst %,$(OBJDIR)/%.o,$(_OBJ))

$(OBJDIR)/%.o : $(SRCDIR)/%.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

build : $(OBJ)
	$(CC) $(CFLAGS) $(LIBDIRS) -o $(BUILDDIR)/$(PROJ) $^ $(LIBS)

.PHONY: clean

clean:
	rm -f $(OBJDIR)/*
	rm -f $(OBJDIR)/core/*
	rm -f $(BUILDDIR)/$(PROJ)*

.PHONY: run

run : build
	$(BUILDDIR)/$(PROJ)

.PHONY: rebuild

rebuild : clean build

WINCC = x86_64-w64-mingw32-clang
WINFLAGS = -DGLFW_DLL
_WINLIBS = glfw3dll opengl32
WINLIBS = $(foreach LIB,$(_WINLIBS),-l$(LIB))

_WINOBJ = $(foreach O,$(_OBJ),$(O).obj)
WINOBJ = $(patsubst %,$(OBJDIR)/%,$(_WINOBJ))

$(OBJDIR)/%.obj : $(SRCDIR)/%.c $(DEPS)
	$(WINCC) $(CFLAGS) -c -o $@ $<

.PHONY: windows

windows : $(WINOBJ)
	$(WINCC) $(CFLAGS) $(WINFLAGS) $(LIBDIRS) -o $(BUILDDIR)/$(PROJ).exe $^ $(WINLIBS)

.PHONY: wine

wine : windows
	wine $(BUILDDIR)/$(PROJ).exe
