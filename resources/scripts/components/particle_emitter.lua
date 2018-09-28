ffi.cdef[[
typedef enum particle_animation_e
{
	PARTICLE_ANIMATION_FORWARD,
	PARTICLE_ANIMATION_BACKWARD,
	PARTICLE_ANIMATION_LOOP_FORWARD,
	PARTICLE_ANIMATION_LOOP_BACKWARD,
	PARTICLE_ANIMATION_BOUNCING_FORWARD,
	PARTICLE_ANIMATION_BOUNCING_BACKWARD
} ParticleAnimation;

typedef struct particle_emitter_component_t
{
	bool active;
	bool paused;
	bool stopping;
	char currentParticle[64];
	real64 particleCounter;
	real32 currentSpawnRate;
	real32 spawnRate[2];
	uint32 maxNumParticles;
	bool stopAtCapacity;
	real64 lifetime[2];
	real32 fadeTime;
	kmVec3 initialVelocity;
	kmVec3 minRandomVelocity;
	kmVec3 maxRandomVelocity;
	kmVec3 acceleration;
	kmVec2 minSize;
	kmVec2 maxSize;
	bool preserveAspectRatio;
	kmVec4 color;
	kmVec4 minRandomColor;
	kmVec4 maxRandomColor;
	int32 initialSprite;
	bool randomSprite;
	real32 animationFPS;
	ParticleAnimation animationMode;
	int32 finalSprite;
} ParticleEmitterComponent;
]]

local component = engine.components:register("particle_emitter", "ParticleEmitterComponent")

local C = engine.C

function component:emit(stopAtCapacity, particle, numSprites, rows, columns, textureFiltering, initialSprite, randomSprite, animationFPS, animationMode, finalSprite)
  stopAtCapacity = stopAtCapacity or false
  particle = particle or ""
  numSprites = numSprites or 1
  rows = rows or 1
  columns = columns or 1
  initialSprite = initialSprite or self.initialSprite
  randomSprite = randomSprite or self.randomSprite
  animationFPS = animationFPS or self.animationFPS
  animationMode = animationMode or self.animationMode
  finalSprite = finalSprite or self.finalSprite

  C.emitParticles(self, stopAtCapacity, particle, numSprites, rows, columns, textureFiltering, initialSprite, randomSprite, animationFPS, animationMode, finalSprite)
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