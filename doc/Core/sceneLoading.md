### [Table of Contents](../main.md) -> [Core Engine](coreIndex.md) -> Scenes

# Scenes

As with entities, scenes are written in JSON files. We have a generator for Scene files that is separate from the entity exporter, but works in basically the same way.  

Scenes are made up of three main things: systems, component limits, and an active camera.  

Here is an example from the [JSON Utilities Guide](../JSON/json.md):

``` json
{
	"systems":
	{
		"update":
		{
			"external":
			[

			],

			"internal":
			[
				"physics"
			]
		},

		"draw":
		{
			"external":
			[

			],

			"internal":
			[
				"renderer"
			]
		}
	},

	"component_limits":
	{
		"transform": 1024,
		"model": 1024,
		"camera": 16
	},

	"active_camera": ""
}
```

#### Systems

There are two types of systems: systems that update entities and systems that draw entities. Update systems would be systems like the physics engine, camera controls, or player controls. Drawing systems would consist of systems like the rendering system, animations, and debug rendering.  

Within these two types of systems, there are two more types of systems: internal and external systems. Internal systems are ones that are defined within the Ghoti engine already. External systems are user defined systems that also need to be implemented into the scene.

#### Component Limits

The component limits are the total amount of that component type allowed in the scene. If more than the limit is found in the scene, no more of that component will be added. This is also a list of all the components that will be found in the scene. If a component is not in this list, _that component will not be added to the scene_. Make sure that the scene has the components your entities contain in the limits section!

#### Active Camera

This is the UUID of the active camera. If you don't have an active camera in the scene, use an empty string like in the example.

## Scenes Management

To load a scene in Lua, call `C.loadScene("scene_name")`. It is that simple.

To unload a scene, call `C.unloadScene("scene_name")`. It really is that simple.

To reload a scene, call `C.reloadScene("scene_name")` Do you see a pattern here?

Behind the *scenes*, there is complex code running just so that we can manage scenes as easily as possible.

#### Component Limits in the Scene

Before adding entities, the component limits need to be set, as well as what components are in the scene. The component needs a corresponding UUID, which you can get by calling `idFromName` with the component's name. After that, you can now call `sceneAddComponentType` to add component limits to the scene. To remove components from a scene, you can call `sceneRemoveComponentType`.

#### Entities in a Scene

Now that we have component limits added to the scene, adding entities to a scene is just as easy. By calling the `sceneCreateEntity` function, it will return a UUID for a new entity. This allows you to start adding the components of an entity by using `sceneAddComponentToEntity`. You can remove components by calling `sceneRemoveComponentFromEntity` and remove entire entities by calling `sceneRemoveEntity`.  

If you want to create an entity with a specific UUID, `sceneRegisterEntity` will register an entity if you hand it a UUID created with the `idFromName` function.
