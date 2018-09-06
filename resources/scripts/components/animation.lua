ffi.cdef[[
typedef struct animation_component_t
{
	char name[64];
	UUID skeleton;
	real64 time;
	real64 duration;
	bool loop;
	real32 speed;
	bool backwards;
	bool paused;
} AnimationComponent;
]]

local component = engine.components:register("animation", "AnimationComponent")

local C = engine.C

function component:play(name, loop, speed, backwards)
  self.name = name
  self.loop = loop or false
  self.speed = speed or 1.0
  self.backwards = backwards or false
  self.paused = false
end

function component:stop()
  self.name = ""
  self.time = 0.0
  self.duration = 0.0
  self.paused = false
end

function component:removeWithSkeleton(scene, uuid)
  C.sceneRemoveEntity(scene, self.skeleton)
  C.sceneRemoveComponentFromEntity(scene, uuid, C.idFromName("animation"))
end