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

function component:play(scene, uuid, name, loop, speed, backwards)
  loop = loop or false
  speed = speed or 1.0
  backwards = backwards or false

  C.playAnimation(scene:getComponent("model", uuid), self, name, loop, speed, backwards)
end

function component:stop()
  C.stopAnimation(self)
end

function component:removeWithSkeleton(scene, uuid)
  C.sceneRemoveEntity(scene, self.skeleton)
  C.sceneRemoveComponentFromEntity(scene, uuid, C.idFromName("animation"))
end