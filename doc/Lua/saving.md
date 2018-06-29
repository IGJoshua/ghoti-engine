### [Table of Contents](../main.md) -> [Lua](Lua.md) -> Saving

# Saving

More information on saving can be found [here](../Core/saving.md).

## bool getSaveSlotAvailability
Returns `true` if a save slot is available, else returns `false`
```c
bool getSaveSlotAvailability(uint32 slot);
```
## int32 exportSave
Exports save data that is contained in the scene to the specific save slot
```c
int32 exportSave(void *data, uint32 size, const Scene *scene, uint32 slot);
```
## int32 loadSave
Loads save data from a specific save slot
```c
int32 loadSave(uint32 slot, Scene **scene);
```
## int32 deleteSave
Deletes save data at a specific save slot
```c
int32 deleteSave(uint32 slot);
```
