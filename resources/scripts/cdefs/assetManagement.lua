ffi.cdef[[

void loadModel(const char *name);
void freeModel(const char *name);
void swapMeshMaterial(
	const char *modelName,
	const char *meshName,
	const char *materialName);

void loadFont(const char *name, uint32 size);

void loadImage(const char *name);
void freeImage(const char *name);

void loadAudio(const char *name);

void loadParticle(
	const char *name,
	uint32 numSprites,
	uint32 rows,
	uint32 columns);

]]