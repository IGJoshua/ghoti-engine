ffi.cdef[[

void loadAudio(const char *name);
void loadCubemap(const char *name, bool swapFrontAndBack);
void loadFont(const char *name, uint32 size, bool autoScaling);
void loadImage(const char *name, bool textureFiltering);

void loadModel(const char *name);
void swapMeshMaterial(
	const char *modelName,
	const char *meshName,
	const char *materialName);

void loadParticle(
	const char *name,
	uint32 numSprites,
	uint32 rows,
	uint32 columns,
	bool textureFiltering);

uint32 getAssetThreadCount(void);

]]