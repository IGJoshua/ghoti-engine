### [Table of Contents](../../main.md) -> [Core Engine](../coreIndex.md) -> Entity Component System

# Entity Component System

The Entity Component System allows us to have more flexibility with Lua than some other design patterns would. In the Entity Component System (a.k.a. ECS), there are three important parts, the entity, the component, and the system.

Entities are in-game objects.  
Components describe the entity they are attached to.  
Systems update any entity that has it's associated component type.  

A list of already available components can be found [here](ecsComponents.md)  

Making an entity is fairly easy. All of the data for an entity is contained in a JSON file. We even made a JSON file generator.

Here is the example of an entity, like the ones in the [JSON Utilities Guide](../../JSON/json.md):

``` json
{
	"uuid": "-p]h3S).cZh(k-IH;8UbrOgo|K;;{I9):upLHwY-SA3>L[cfp8Jeg1VcZn!W79_",

	"components":
	{
		"transform":
		[
			{
				"name": "position",
				"float32": [0.0, 0.0, 0.0]
			},

			{
				"name": "rotation",
				"float32": [0.0, 0.0, 0.0, 1.0]
			},

			{
				"name": "scale",
				"float32": [0.63, 0.54, 0.07]
			}
		],

		"orbit":
		[
			{
                "name": "origin",
				"float32": [0.0, 5.0, 0.0]
			},

			{
				"name": "speed",
				"float32": 1.5
			},

            {
				"name": "radius",
				"float32": 5.0
			},

            {
				"name": "time",
				"float32": 1.5
			}
		]
	}
}
```
Those UUIDs might look scary, but they are randomly generated strings used for unique identification.  

The "name" refers to which member variable you are trying to define. The second field refers to the type.

Now that we have an entity, and some components defined, we need to make sure that the components are defined in LuaJIT's cdefs:

```Lua
--resources/scripts/components/orbit.lua
ffi.cdef[[
typedef struct orbit_component_t
{
  kmVec3 origin;
  float speed;
  float radius;
  float time;
} OrbitComponent;
]]
```
This tells LuaJIT that there is a C side struct that has these members that we need to reference. The LuaJIT then makes a table that allows us to use the component in Lua.

This is only two thirds of the ECS though. The entity and components are cool, but are nothing without having a system.

```Lua
--resources/scripts/system/orbit.lua

io.write("Loading the Orbit system\n")

local system = {}

system.init = nil
system.shutdown = nil

system.components = {}
system.components[1] = "orbit"
system.components[2] = "transform"

function system.run(scene, uuid, dt)
    local transform = scene:getComponent("transform", uuid)
    local orbit = scene:getComponent("orbit", uuid)

    orbit.time = orbit.time + dt

    transform.position.x = math.sin(orbit.time * orbit.speed) * orbit.radius + orbit.origin.x
    transform.position.y = math.cos(orbit.time * orbit.speed) * orbit.radius + orbit.origin.y
    transform.position.z = orbit.origin.z
end

io.write("Finished loading the Orbit system\n")

return system

```

This is where the magic of ECS happens. Since our example entity has an `orbit` component attached to it, the `orbit` system will update that entity. All other entities that do not have that component will be ignored, as they aren't suppose to orbit anything.

The ECS is great, but you can't just have entities without something to hold them too. This is were scenes come in. You can read more about scenes and scene loading [here](../sceneLoading.md).
