ffi.cdef[[
typedef struct audio_manager_t
{
	uint32 buffers;
	uint32 sources;
} AudioManagerComonent;
]]

local component = engine.components:register("audio_manager", "AudioManagerComonent")
