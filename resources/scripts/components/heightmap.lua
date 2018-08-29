ffi.cdef[[
typedef struct heightmap_component_t
{
	dGeomID heightfieldGeom;
	char heightmapName[1024];
	char materialName[64];
	uint32 sizeX;
	uint32 sizeZ;
	uint32 maxHeight;
	real32 unitsPerTile;
	real32 uvScaleX;
	real32 uvScaleZ;
} HeightmapComponent;
]]

local component = engine.components:register("heightmap", "HeightmapComponent")