ffi.cdef[[
typedef struct audio_source_component_t
{
	uint32 id;
	real32 pitch;
	real32 gain;
	bool looping;
} AudioSourceComponent;
]]

local component = engine.components:register("audio_source", "AudioSourceComponent")

local C = engine.C

function component:play(name)
  if not self:isActive() then
    C.playSoundAtSource(self, name)
  end
end

function component:isActive()
  return C.isSourceActive(self)
end

function component:pause()
  C.pauseSoundAtSource(self)
end

function component:resume()
  C.resumeSoundAtSource(self)
end

function component:stop()
  C.stopSoundAtSource(self)
end

function component:restart()
  C.resumeSoundAtSource(self)
end