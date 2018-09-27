ffi.cdef[[
typedef struct particle_emitter_component_t
{
	bool active;
	char currentParticle[64];
	real64 particleCounter;
	uint32 spawnRate;
	uint32 maxNumParticles;
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