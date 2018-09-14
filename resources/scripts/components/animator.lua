ffi.cdef[[
typedef struct animator_component_t
{
	char currentAnimation[64];
	real64 time;
	real64 duration;
	int32 loopCount;
	real32 speed;
	bool paused;
	char previousAnimation[64];
	real64 previousAnimationTime;
	real64 transitionTime;
	real64 transitionDuration;
} AnimatorComponent;
]]

local component = engine.components:register("animator", "AnimatorComponent")

local C = engine.C

function component:play(scene, uuid, name, loopCount, speed, transitionDuration)
  loopCount = loopCount or 0
  speed = speed or 1.0
  transitionDuration = transitionDuration or 0.0

  C.playAnimation(scene:getComponent("model", uuid), self, name, loopCount, speed, transitionDuration, true)
end

function component:pause()
  self.paused = true
end

function component:unpause()
  self.paused = false
end

function component:reverse()
  self.speed = self.speed * -1.0
end

function component:restart(loopCount, transitionTime)
  if self.speed < 0.0 then
    self.time = self.duration
  else
    self.time = 0.0
  end

  self.loopCount = loopCount or 0
  self.paused = false
  self.transitionTime = transitionTime or 0.0
end

function component:stop()
  C.stopAnimation(self)
end