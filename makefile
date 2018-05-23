PROJ = monochrome

IDIRS = include
SRCDIR = src
BUILDDIR = build
OBJDIR = build/obj
LIBDIRS = -Llib

CC = clang
CCDB = lldb
CFLAGS = $(foreach DIR,$(IDIRS),-I$(DIR)) -g -D_DEBUG

_LIBS = GLEW glfw GL m
LIBS = $(foreach LIB,$(_LIBS),-l$(LIB))

CORE_DEPS = defines.h core/window.h
DATA_DEPS = data/data_types.h data/list.h data/hash_map.h
ASSET_MANAGEMENT_DEPS = asset_management/asset_management_types.h
THREADING_DEPS = threading/threading_types.h threading/promise.h
ECS_DEPS = ECS/ecs_types.h ECS/component.h ECS/scene.h ECS/system.h
DEPS = $(patsubst %,$(IDIRS)/%,$(CORE_DEPS)) $(patsubst %,$(IDIRS)/%,$(ASSET_MANAGEMENT_DEPS)) $(patsubst %,$(IDIRS)/%,$(THREADING_DEPS)) $(patsubst %,$(IDIRS)/%,$(DATA_DEPS)) $(patsubst %,$(IDIRS)/%,$(ECS_DEPS))

_OBJ = core/main core/window threading/promise data/list data/hash_map ECS/component ECS/scene ECS/system
OBJ = $(patsubst %,$(OBJDIR)/%.o,$(_OBJ))

$(OBJDIR)/%.o : $(SRCDIR)/%.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

build : $(OBJ)
	$(CC) $(CFLAGS) $(LIBDIRS) -o $(BUILDDIR)/$(PROJ) $^ $(LIBS)

.PHONY: clean

clean:
	rm -f -r $(BUILDDIR)
	mkdir {$(BUILDDIR),$(OBJDIR)/{,core,data,renderer,ECS,threading,asset_management}}

.PHONY: run

run : build
	$(BUILDDIR)/$(PROJ)

.PHONY: debug

debug : build
	$(CCDB) $(BUILDDIR)/$(PROJ)

.PHONY: rebuild

rebuild : clean build

WINCC = x86_64-w64-mingw32-clang
WINFLAGS = -DGLFW_DLL
_WINLIBS = glew32 glfw3dll opengl32 pthread
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
