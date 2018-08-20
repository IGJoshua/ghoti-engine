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
  UUID componentID;
  uint32 numEntries;
  uint32 componentSize;
  uint32 firstFree;
  HashMap idToIndex;
  uint8 data[];
} ComponentDataTable;

typedef struct
{
  uint32 index;
  ComponentDataTable *table;
} ComponentDataTableIterator;

ComponentDataTableIterator cdtGetIterator(ComponentDataTable *table);
void cdtMoveIterator(ComponentDataTableIterator *itr);
uint32 cdtIteratorAtEnd(ComponentDataTableIterator itr);
UUID cdtIteratorGetUUID(ComponentDataTableIterator itr) ;
void *cdtIteratorGetData(ComponentDataTableIterator itr);
]]
