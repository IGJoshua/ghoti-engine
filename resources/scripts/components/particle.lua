ffi.cdef[[
typedef struct particle_component_t
{
	UUID particleEmitter;
	real64 lifetime;
	real64 fadeTimer[2];
	real64 fadeTime[2];
	int32 sprite;
	real64 animationTime;
	int8 animationDirection;
	kmVec3 position;
	kmVec3 previousPosition;
	kmVec3 velocity;
	kmVec2 size;
	kmVec2 uv;
	kmVec4 color;
	real32 alpha;
} ParticleComponent;
]]

local component = engine.components:register("particle", "ParticleComponent")