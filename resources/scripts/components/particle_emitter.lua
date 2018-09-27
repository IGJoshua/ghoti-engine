ffi.cdef[[
typedef struct particle_emitter_component_t
{
	bool active;
	bool paused;
	bool stopping;
	char currentParticle[64];
	real64 particleCounter;
	uint32 spawnRate;
	uint32 maxNumParticles;
	bool stopAtCapacity;
	real64 lifetime;
	real64 fadeTime;
	kmVec3 initialVelocity;
	kmVec3 minRandomVelocity;
	kmVec3 maxRandomVelocity;
	kmVec3 acceleration;
	kmVec2 size;
	kmVec3 color;
	kmVec3 minRandomColor;
	kmVec3 maxRandomColor;
	real32 alpha;
	int32 initialSprite;
	bool randomSprite;
	real32 animationFPS;
	bool loop;
} ParticleEmitterComponent;
]]

local component = engine.components:register("particle_emitter", "ParticleEmitterComponent")

local C = engine.C

function component:emit(stopAtCapacity, particle, numSprites, spriteWidth, spriteHeight, initialSprite, randomSprite, animationFPS, loop)
  stopAtCapacity = stopAtCapacity or false
  particle = particle or ""
  numSprites = numSprites or 1
  spriteWidth = spriteWidth or -1
  spriteHeight = spriteHeight or -1
  initialSprite = initialSprite or 0
  randomSprite = randomSprite or false
  animationFPS = animationFPS or 24
  loop = loop or false

  C.emitParticles(self, stopAtCapacity, particle, numSprites, spriteWidth, spriteHeight, initialSprite, randomSprite, animationFPS, loop)
end

function component:pause()
  self.paused = true
end

function component:resume()
  self.paused = false
end

function component:stop()
  C.stopParticleEmitter(self, true)
end