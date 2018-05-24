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
DBFLAGS = -g -D_DEBUG -O0

_LIBS = GLEW glfw GL m assimp kazmath GLU IL ILU
LIBS = $(foreach LIB,$(_LIBS),-l$(LIB))

CORE_DEPS = defines.h core/window.h
DATA_DEPS = data/data_types.h data/list.h data/hash_map.h
RENDERER_DEPS = renderer/renderer_types.h renderer/shader.h
ASSET_MANAGEMENT_DEPS = asset_management/asset_manager_types.h asset_management/material.h asset_management/mesh.h asset_management/model.h asset_management/scene.h asset_management/texture.h
THREADING_DEPS = threading/threading_types.h threading/promise.h
ECS_DEPS = ECS/ecs_types.h ECS/component.h ECS/scene.h ECS/system.h systems.h
DEPS = $(patsubst %,$(IDIRS)/%,$(CORE_DEPS)) $(patsubst %,$(IDIRS)/%,$(RENDERER_DEPS)) $(patsubst %,$(IDIRS)/%,$(ASSET_MANAGEMENT_DEPS)) $(patsubst %,$(IDIRS)/%,$(THREADING_DEPS)) $(patsubst %,$(IDIRS)/%,$(DATA_DEPS)) $(patsubst %,$(IDIRS)/%,$(ECS_DEPS))

_OBJ = resources core/main core/window renderer/shader asset_management/material asset_management/mesh asset_management/model asset_management/scene asset_management/texture threading/promise data/list data/hash_map ECS/component ECS/scene ECS/system systems/renderer
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
	mkdir {$(BUILDDIR),$(OBJDIR)/{,core,data,renderer,ECS,threading,asset_management,systems}}

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
