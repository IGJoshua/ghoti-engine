ffi.cdef[[
typedef struct spawner_component_t
{
	uint32 numToSpawn;
	uint32 numSpawned;
	real32 spawnPerSecond;
	real32 timeElapsed;
} SpawnerComponent;
]]

local component = engine.components:register("spawner", "SpawnerComponent")
