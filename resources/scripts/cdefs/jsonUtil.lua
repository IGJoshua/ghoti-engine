ffi.cdef[[

typedef void (*Log)(const char *format, ...);

int32 generateEntity(const char *filename, Log log);
int32 exportEntity(const char *filename, Log log);
int32 generateScene(const char *filename, Log log);
int32 exportScene(const char *filename, Log log);

]]
