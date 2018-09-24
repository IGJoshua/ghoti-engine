ffi.cdef[[

int32 loadModel(const char *name);
void freeModel(const char *name);
void swapMeshMaterial(
	const char *modelName,
	const char *meshName,
	const char *materialName);

int32 loadFont(const char *name, uint32 size);
void* getFont(const char *name, uint32 size);

int32 loadAudio(const char *name);

]]