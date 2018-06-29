ffi.cdef[[

typedef struct component_value_definition_t
{
	char *name;
	DataType type;
	uint32 maxStringSize;
	uint32 count;
} ComponentValueDefinition;

typedef struct component_definition_t
{
	char *name;
	uint32 size;
	uint32 numValues;
	ComponentValueDefinition *values;
} ComponentDefinition;

typedef struct component_data_table_t
{
  uint32 numEntries;
  uint32 componentSize;
  HashMap idToIndex;
  uint8 data[];
} ComponentDataTable;

]]
