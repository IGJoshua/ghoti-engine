ffi.cdef[[

typedef enum asset_type_e
{
	ASSET_TYPE_MODEL,
	ASSET_TYPE_TEXTURE
} AssetType;

void loadAssetAsync(AssetType type, const char *name, const char *filename);

void freeModel(const char *name);
void swapMeshMaterial(
	const char *modelName,
	const char *meshName,
	const char *materialName);

int32 loadFont(const char *name, uint32 size);
void* getFont(const char *name, uint32 size);
void freeFont(void *font);

int32 loadAudio(const char *name);

void activateAssetsChangedFlag(void);

]]