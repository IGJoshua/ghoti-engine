### [Table of Contents](../main.md) -> [Core Engine](core.md) -> Saving

# Saving Game data

## Checking a Save Slot
When saving game data, make sure to find out if the save slot you are saving to has room in it by using `getSaveSlotAvailability`. This checks to see if there is data there already. You can either prompt the user to save over existing data, or tell them they cannot save there.

## Saving Data
To actually save the game, use the function `exportSave`. This function takes a void pointer for the data being saved, the size of bytes that data is, a pointer to the current scene, and a save slot in which to save to. Calling this on a save slot that already has data in it will overwrite the data, so make sure to ask before saving!


## Loading Save Data
Loading a save requires the save slot number and a double pointer to the current scene.  By calling the `loadSave` function, the save will be read, and the scene data will be loaded into the double pointer.


## Deleting Save Data
To delete a save, call `deleteSave` with the save slot's number.
