ffi.cdef[[
int32 loadModel(const char *name);
void freeModel(const char *name);

int32 loadFont(const char *name, uint32 size);
void* getFont(const char *name, uint32 size);
void freeFont(void *font);
]]