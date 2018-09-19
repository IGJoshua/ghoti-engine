ffi.cdef[[
typedef struct audio_manager_component_t
{
	uint32 buffers;
	uint32 sources;
} AudioManagerComponent;
]]

local component = engine.components:register("audio_manager", "AudioManagerComponent")
