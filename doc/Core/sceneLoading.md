### [Table of Contents](../main.md) -> [Core Engine](core.md) -> Scene Loading

# Scene Loading

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

	"active_camera": "none"
}
```

### Systems

There are two types of systems: systems that update entities and systems that draw entities. Update systems would be systems like the physics engine, camera controls, or player controls. Drawing systems would consist of systems like the rendering system, animations, and debug rendering.  

Within these two types of systems, there are two more types of systems: internal and external systems. Internal systems are ones that are defined within the Ghoti engine already. External systems are user defined systems that also need to be implemented into the scene.

### Component Limits

The component limits are the total amount of that component type allowed in the scene. If more than the limit is found in the scene, no more of that component will be added. This is also a list of all the components that will be found in the scene. If a component is not in this list, _that component will not be added to the scene_. Make sure that the scene has the components your entities contain in the limits section!

### Active Camera

This is the UUID of the active camera. If you don't have an active camera in the scene, use "none" like in the example.

## Loading a Scenes
