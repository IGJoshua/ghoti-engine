ffi.cdef[[
typedef struct animator_component_t
{
	char currentAnimation[64];
	real64 time;
	real64 duration;
	bool loop;
	real32 speed;
	bool backwards;
	bool paused;
} AnimatorComponent;
]]

local component = engine.components:register("animator", "AnimatorComponent")

local C = engine.C

function component:play(scene, uuid, name, loop, speed, backwards)
  loop = loop or false
  speed = speed or 1.0
  backwards = backwards or false

  C.playAnimation(scene:getComponent("model", uuid), self, name, loop, speed, backwards)
end

function component:pause()
  self.paused = true
end

function component:unpause()
  self.paused = false
end

function component:restart()
  if self.backwards then
    self.time = self.duration
  else
    self.time = 0.0
  end
end

function component:stop()
  C.stopAnimation(self)
end