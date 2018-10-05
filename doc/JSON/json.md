### [Table of Contents](../main.md) -> JSON Utilities

# JSON Utilities

A set of utilities to manage the JSON entity, asset, and scene files that are used in the Ghoti Engine.

## Entity Generator

### Description

This utility generates a blank JSON entity file that can be used as a template for new entities. The blank file contains a randomly generated UUID and an empty list of components.

### Example

#### new_entity.json:

``` json
{
	"uuid": "-p]h3S).cZh(k-IH;8UbrOgo|K;;{I9):upLHwY-SA3>L[cfp8Jeg1VcZn!W79_",

	"components":
	{

	}
}
```

### Usage

``` shell
entity-generator
```

The utility will output to a JSON file in the current directory named `new_entity.json`.

``` shell
entity-generator player
```

or

``` shell
entity-generator player.json
```

The utility will output to a JSON file in the current directory named `player.json`.

``` shell
entity-generator --help
```

Example usage for the utility is shown.

## Entity Exporter

### Description

This utility exports JSON entity files to binary entity files that can be used in the engine at run time. The name of a component and the names and data types of its members correspond to the component's definition in its respective Lua file. The following data types can be used either singularly or as an array of values for any given member of a component.

#### Available Data Types:

| Type    | Size        | Range                                                                                       | Description          |
|:-------:|:-----------:|:-------------------------------------------------------------------------------------------:|:--------------------:|
| uint8   | 1 byte      | 0 to 255                                                                                    | non-negative integer |
| uint16  | 2 bytes     | 0 to 65,535                                                                                 | non-negative integer |
| uint32  | 4 bytes     | 0 to 4,294,967,295                                                                          | non-negative integer |
| uint64  | 8 bytes     | 0 to 18,446,744,073,709,551,615                                                             | non-negative integer |
| int8    | 1 byte      | -128 to 127                                                                                 | integer              |
| int16   | 2 bytes     | -32,768 to 32,767                                                                           | integer              |
| int32   | 4 bytes     | -2,147,483,648 to 2,147,483,647                                                             | integer              |
| int64   | 8 bytes     | -9,223,372,036,854,775,808 to 9,223,372,036,854,775,807                                     | integer              |
| float32 | 4 bytes     | -3.4 $\times$ 10<sup>38</sup> to 3.4 $\times$ 10<sup>38</sup>                               | real number          |
| float64 | 8 bytes     | -1.7 $\times$ 10<sup>308</sup> to 1.7 $\times$ 10<sup>308</sup>                             | real number          |
| bool    | 1 byte      | true or false                                                                               | boolean              |
| char    | 1 byte      | 1 ascii character                                                                           | character            |
| char(n) | n bytes     | maximum of n - 1 ascii characters                                                           | string               |
| uuid    | 64 bytes    | ascii characters between `#` and `z` (inclusive) excluding the characters `\`, `[`, and `]` | uuid                 |
| enum    | 4 bytes     | -2,147,483,648 to 2,147,483,647                                                             | enumerator           |
| ptr     | 8 bytes     | 0 to 18,446,744,073,709,551,615                                                             | pointer              |

### Example

#### entity_file.json:

``` json
{
	"uuid": "r<1F6bC@dn:B0Zte6)1'hA6N5C^?HyS8;YSG9td#`%CmT=P_;_dyuwM08)M(urA",

	"components":
	{
		"transform":
		[
			{
				"name": "position",
				"float32": [192.18, -527.35, 391.69]
			},

			{
				"name": "rotation",
				"float32": [0.0, 0.0, 0.0, 1.0]
			},

			{
				"name": "scale",
				"float32": [0.63, 0.54, 0.07]
			},

			{
                "name": "parent",
                "uuid": ""
			},

            {
                "name": "first child",
                "uuid": ""
			},

            {
                "name": "next sibling",
                "uuid": ""
			},

            {
                "name": "dirty",
                "bool": true
			},

            {
                "name": "global position",
                "float32": [0.0, 0.0, 0.0]
			},

            {
                "name": "global rotation",
                "float32": [0.0, 0.0, 0.0, 1.0]
			},

            {
                "name": "global scale",
                "float32": [1.0, 1.0, 1.0]
			},

            {
                "name": "last global position",
                "float32": [0.0, 0.0, 0.0]
			},

            {
                "name": "last global rotation",
                "float32": [0.0, 0.0, 0.0, 1.0]
			},

            {
                "name": "last global scale",
                "float32": [1.0, 1.0, 1.0]
            }
		],

		"model":
		[
			{
				"name": "name",
				"char(64)": "player"
			},

			{
				"name": "visible",
				"bool": true
			}
		],

		"player":
		[
			{
				"name": "hearts",
				"uint8": 16
			},

			{
				"name": "coins",
				"uint32": 576
			},

			{
				"name": "weapon",
				"uuid": "&v3Bh+vIyd.YO#ua@C5sV$2+,Pq%(>f#2w;w(7>vpJMFJI%_a/X=0g=:6,43Gx,"
			}
		]
	}
}
```

### Usage

``` shell
entity-exporter entity_file
```

or

``` shell
entity-exporter entity_file.json
```

The utility will output to a binary file in the current directory named `entity_file.entity`.

``` shell
entity-exporter --help
```

Example usage for the utility is shown.

## Asset Exporter

### Description

This utility exports JSON asset files to binary asset files and JSON entity files that can be used in the engine at run time. Each mesh is associated with a material and its values as well as a material mask and an opacity mask. There is also physics information about the model's center of mass as well as the boxes, spheres, and capsules associated with the model. If the asset has animations, a list of animation names are present along with the UUID of the root node of the skeleton.

### Example

#### asset_file.json:

``` json
{
	"meshes":
	{
		"door":
		{
			"material":
			{
				"name": "wood",
				"double-sided": false,

				"uv":
				{
					"map": 3,
					"swizzle": true
				},

				"values":
				{
					"ambient_occlusion": [1.0, 1.0, 1.0],
					"base_color": [1.0, 1.0, 1.0],
					"emissive": [1.0, 1.0, 1.0],
					"height": [1.0, 1.0, 1.0],
					"metallic": [1.0, 1.0, 1.0],
					"normal": [1.0, 1.0, 1.0],
					"roughness": [1.0, 1.0, 1.0]
				}
			},

			"masks":
			{
				"uv":
				{
					"map": 1,
					"swizzle": true
				},

				"material":
				{
					"collection":
					{
						"name": "moss",
						"double-sided": false,

						"values":
						{
							"ambient_occlusion": [1.0, 1.0, 1.0],
							"base_color": [1.0, 1.0, 1.0],
							"emissive": [1.0, 1.0, 1.0],
							"height": [1.0, 1.0, 1.0],
							"metallic": [1.0, 1.0, 1.0],
							"normal": [1.0, 1.0, 1.0],
							"roughness": [1.0, 1.0, 1.0]
						}
					},

					"grunge":
					{
						"name": "grass",
						"double-sided": false,

						"values":
						{
							"ambient_occlusion": [1.0, 1.0, 1.0],
							"base_color": [1.0, 1.0, 1.0],
							"emissive": [1.0, 1.0, 1.0],
							"height": [1.0, 1.0, 1.0],
							"metallic": [1.0, 1.0, 1.0],
							"normal": [1.0, 1.0, 1.0],
							"roughness": [1.0, 1.0, 1.0]
						}
					},

					"wear":
					{
						"name": "metal",
						"double-sided": false,

						"values":
						{
							"ambient_occlusion": [1.0, 1.0, 1.0],
							"base_color": [1.0, 1.0, 1.0],
							"emissive": [1.0, 1.0, 1.0],
							"height": [1.0, 1.0, 1.0],
							"metallic": [1.0, 1.0, 1.0],
							"normal": [1.0, 1.0, 1.0],
							"roughness": [1.0, 1.0, 1.0]
						}
					}
				},

				"opacity":
				{
					"value": 1.0
				}
			}
		},

		"window":
		{
			"material":
			{
				"name": "glass",
				"double-sided": true,

				"uv":
				{
					"map": 5,
					"swizzle": true
				},

				"values":
				{
					"ambient_occlusion": [1.0, 1.0, 1.0],
					"base_color": [1.0, 1.0, 1.0],
					"emissive": [1.0, 1.0, 1.0],
					"height": [1.0, 1.0, 1.0],
					"metallic": [1.0, 1.0, 1.0],
					"normal": [1.0, 1.0, 1.0],
					"roughness": [1.0, 1.0, 1.0]
				}
			},

			"masks":
			{
				"uv":
				{
					"map": 2,
					"swizzle": true
				},

				"material":
				{
					"collection":
					{
						"name": "moss",
						"double-sided": true,

						"values":
						{
							"ambient_occlusion": [1.0, 1.0, 1.0],
							"base_color": [1.0, 1.0, 1.0],
							"emissive": [1.0, 1.0, 1.0],
							"height": [1.0, 1.0, 1.0],
							"metallic": [1.0, 1.0, 1.0],
							"normal": [1.0, 1.0, 1.0],
							"roughness": [1.0, 1.0, 1.0]
						}
					},

					"grunge":
					{
						"name": "grass",
						"double-sided": true,

						"values":
						{
							"ambient_occlusion": [1.0, 1.0, 1.0],
							"base_color": [1.0, 1.0, 1.0],
							"emissive": [1.0, 1.0, 1.0],
							"height": [1.0, 1.0, 1.0],
							"metallic": [1.0, 1.0, 1.0],
							"normal": [1.0, 1.0, 1.0],
							"roughness": [1.0, 1.0, 1.0]
						}
					},

					"wear":
					{
						"name": "",
						"double-sided": true,

						"values":
						{
							"ambient_occlusion": [1.0, 1.0, 1.0],
							"base_color": [1.0, 1.0, 1.0],
							"emissive": [1.0, 1.0, 1.0],
							"height": [1.0, 1.0, 1.0],
							"metallic": [1.0, 1.0, 1.0],
							"normal": [1.0, 1.0, 1.0],
							"roughness": [1.0, 1.0, 1.0]
						}
					}
				},

				"opacity":
				{
					"value": 1.0
				}
			}
		}
	},

	"physics":
	{
		"center_of_mass": [3.0, 2.0, -5.0],

		"collision_primitives":
		{
			"boxes":
			{
				"wall_1":
				{
					"entity": "XOWqBtdco3>g)yX;K-1^@BW(unQ#h4Y5hHLb25WBI=AHj#`;%fwC&L@x@np'wO1",
					"position": [0.0, 0.0, 0.0],
					"rotation": [0.0, 0.0, 0.0, 1.0],
					"scale": [2.3, 5.0, 5.0]
				},

				"wall_2":
				{
					"entity": "Yw`1:pp<0&.;5LJ07*s2/ANLqLNr$q$Rna`.OO?TJBlk5i(<Z/AxSjo%7`u.aFq",
					"position": [2.0, -3.0, 0.0],
					"rotation": [0.0, 0.0, 0.0, 1.0],
					"scale": [1.0, 7.0, 3.0]
				}
			},

			"spheres":
			{
				"ground":
				{
					"entity": "L&xdE`,q23jSv@t&yQvx@Zhb:#H%+9N./0Id9@tB(E>E?ADmy8c8pQw(IEyQSMW",
					"position": [-2.7, 0.0, 5.0],
					"scale": 2.0
				}
			},

			"capsules":
			{
				"player":
				{
					"entity": "CP=6=n2rJHTAxw`V@)X38dcNMx=eUPZn&t*@i101OaOMW-)t3V-HAelcc'N7T'+",
					"position": [0.0, 0.0, 2.0],
					"rotation": [0.707, 0.0, 0.0, 0.707],
					"scale": [3.0, 4.0, 3.0]
				}
			}
		}
	},

	"animation":
	{
		"skeleton": "/gbr$uH^kuj50UJ;;tNi)E9j@e4qQWa^DANE5t)&gh8uC_.RYBXtYH:<Q)b._FG",
		"entity": "3B%k)#WG;M99(SRB4veHpi;W=b+WA;vC%u<Q:Lstb2q<Y2ENtj<cLT8^<@;Q0sK",

		"names":
		{
			"Idle": 20.0,
			"Walk": 4.0,
			"Run": 3.0,
			"Jump": 2.5,
			"Attack": 2.0,
			"Block": 1.5
		}
	}
}
```

#### asset_file_entity.json:

``` json
{
	"uuid": "3B%k)#WG;M99(SRB4veHpi;W=b+WA;vC%u<Q:Lstb2q<Y2ENtj<cLT8^<@;Q0sK",

	"components":
	{
		"transform":
		[
			{
				"name": "position",
				"float32": [0, 0, 0]
			},

			{
				"name": "rotation",
				"float32": [0, 0, 0, 1]
			},

			{
				"name": "scale",
				"float32": [1, 1, 1]
			},

			{
				"name": "parent",
				"uuid": ""
			},

			{
				"name": "first child",
				"uuid": "XOWqBtdco3>g)yX;K-1^@BW(unQ#h4Y5hHLb25WBI=AHj#`;%fwC&L@x@np'wO1"
			},

			{
				"name": "next sibling",
				"uuid": ""
			},

			{
				"name": "dirty",
				"bool": true
			},

			{
				"name": "global position",
				"float32": [0, 0, 0]
			},

			{
				"name": "global rotation",
				"float32": [0, 0, 0, 1]
			},

			{
				"name": "global scale",
				"float32": [1, 1, 1]
			},

			{
				"name": "last global position",
				"float32": [0, 0, 0]
			},

			{
				"name": "last global rotation",
				"float32": [0, 0, 0, 1]
			},

			{
				"name": "last global scale",
				"float32": [1, 1, 1]
			}
		],

		"model":
		[
			{
				"name": "name",
				"char(64)": "example_asset"
			},

			{
				"name": "visible",
				"bool": true
			}
		],

		"rigid_body":
		[
			{
				"name": "body ID",
				"ptr": 0
			},

			{
				"name": "space ID",
				"ptr": 0
			},

			{
				"name": "enabled",
				"bool": true
			},

			{
				"name": "dynamic",
				"bool": false
			},

			{
				"name": "gravity",
				"bool": false
			},

			{
				"name": "mass",
				"float32": 1
			},

			{
				"name": "center of mass",
				"float32": [0, 0, 0]
			},

			{
				"name": "velocity",
				"float32": [0, 0, 0]
			},

			{
				"name": "angular velocity",
				"float32": [0, 0, 0]
			},

			{
				"name": "default damping",
				"bool": true
			},

			{
				"name": "linear damping",
				"float32": 0.1
			},

			{
				"name": "angular damping",
				"float32": 0.1
			},

			{
				"name": "linear damping threshold",
				"float32": 0.1
			},

			{
				"name": "angular damping threshold",
				"float32": 0.1
			},

			{
				"name": "max angular speed",
				"float32": 0
			},

			{
				"name": "moment of inertia type",
				"enum": 2
			},

			{
				"name": "moment of inertia",
				"float32": [1, 1, 1, 0, 0, 0]
			}
		],

		"collision":
		[
			{
				"name": "collision tree",
				"uuid": "XOWqBtdco3>g)yX;K-1^@BW(unQ#h4Y5hHLb25WBI=AHj#`;%fwC&L@x@np'wO1"
			},

			{
				"name": "hit list",
				"uuid": ""
			},

			{
				"name": "last hit list",
				"uuid": ""
			}
		],

		"animation":
		[
			{
				"name": "skeleton",
				"uuid": "/gbr$uH^kuj50UJ;;tNi)E9j@e4qQWa^DANE5t)&gh8uC_.RYBXtYH:<Q)b._FG"
			},

			{
				"name": "idle animation",
				"char(64)": "Idle"
			},

			{
				"name": "speed",
				"float32": 1
			},

			{
				"name": "transition duration",
				"float64": 0
			}
		],

		"animator":
		[
			{
				"name": "current animation",
				"char(64)": ""
			},

			{
				"name": "time",
				"float64": 0
			},

			{
				"name": "duration",
				"float64": 0
			},

			{
				"name": "loop count",
				"int32": 0
			},

			{
				"name": "speed",
				"float32": 1
			},

			{
				"name": "paused",
				"bool": false
			},

			{
				"name": "previous animation",
				"char(64)": ""
			},

			{
				"name": "previous animation time",
				"float64": 0
			},

			{
				"name": "transition time",
				"float64": 0
			},

			{
				"name": "transition duration",
				"float64": 0
			}
		]
	}
}
```

### Usage

``` shell
asset-exporter asset_file
```

or

``` shell
asset-exporter asset_file.json
```

The utility will output to a binary file in the current directory named `asset_file.asset`. The utility will also output an entity to a JSON file named `asset_file_entity.json` and collision primitives, if present, as JSON files into a folder named `collision_primitives`.

``` shell
asset-exporter --help
```

Example usage for the utility is shown.

## Scene Generator

### Description

This utility generates a template for a scene by creating a folder containing files to represent a default scene. The scene file contains the default *internal* systems and empty lists for the *external* systems that are called during *update* and *draw*. This file also contains default limits on the number of instances for each of the components used in the default systems, a reference to the active camera in the scene, and a value for gravity. The `entities` folder contains the referenced camera entity as well as a folder containing the default prototype entities.

### Example

#### new_scene/new_scene.json:

``` json
{
	"systems":
	{
		"update":
		{
			"external": ["basic_input"],
			"internal":
			[
				"clean_hit_information",
				"clean_hit_list",
				"animation",
				"clean_global_transforms",
				"apply_parent_transforms",
				"simulate_rigid_bodies",
				"joint_information",
				"particle_simulator",
				"gui",
				"audio"
			]
		},

		"draw":
		{
			"external":
			[

			],

			"internal":
			[
				"cubemap_renderer",
				"render_heightmap",
				"renderer",
				"wireframe_renderer",
				"debug_renderer",
				"collision_primitive_renderer",
				"particle_renderer",
				"gui_renderer"
			]
		}
	},

	"component_limits":
	{
		"animation": 2048,
		"animator": 2048,
		"audio_source": 1024,
		"ball_socket_joint": 256,
		"ball_socket2_joint": 256,
		"box": 1024,
		"button": 1024,
		"camera": 8,
		"capsule": 1024,
		"collision_tree_node": 4096,
		"collision": 1024,
		"cubemap": 1,
		"debug_collision_primitive": 2048,
		"debug_line": 2048,
		"debug_point": 4096,
		"debug_primitive": 4096,
		"debug_transform": 1024,
		"font": 1024,
		"gui_transform": 2048,
		"heightmap": 256,
		"hinge_joint": 256,
		"hit_information": 4096,
		"hit_list": 8192,
		"image": 1024,
		"joint_information": 256,
		"joint_list": 512,
		"joint": 8192,
		"model": 2048,
		"next_animation": 2048,
		"panel": 512,
		"particle_emitter": 128,
		"progress_bar": 1024,
		"rigid_body": 1024,
		"slider_joint": 256,
		"slider": 1024,
		"sphere": 1024,
		"surface_information": 512,
		"text_field": 1024,
		"text": 1024,
		"transform": 16384,
		"widget": 1024,
		"wireframe": 2048
	},

	"active_camera": "uDVo733UsOcF4S@ZB/g@fCcdf&8%b+FCV3OgC*J4O3PXm8#r%9^=y@xtU#V>t2N",
	"gravity": -9.81
}
```

#### new_scene/entities/camera.json:

``` json
{
	"uuid": "uDVo733UsOcF4S@ZB/g@fCcdf&8%b+FCV3OgC*J4O3PXm8#r%9^=y@xtU#V>t2N",

	"components":
	{
		"transform":
		[
			{
				"name": "position",
				"float32": [0, 0, 5]
			},

			{
				"name": "rotation",
				"float32": [0, 0, 0, 1]
			},

			{
				"name": "scale",
				"float32": [1, 1, 1]
			},

			{
				"name": "parent",
				"uuid": ""
			},

			{
				"name": "first child",
				"uuid": ""
			},

			{
				"name": "next sibling",
				"uuid": ""
			},

			{
				"name": "dirty",
				"bool": true
			},

			{
				"name": "global position",
				"float32": [0, 0, 0]
			},

			{
				"name": "global rotation",
				"float32": [0, 0, 0, 1]
			},

			{
				"name": "global scale",
				"float32": [1, 1, 1]
			},

			{
				"name": "last global position",
				"float32": [0, 0, 0]
			},

			{
				"name": "last global rotation",
				"float32": [0, 0, 0, 1]
			},

			{
				"name": "last global scale",
				"float32": [1, 1, 1]
			}
		],

		"camera":
		[
			{
				"name": "near plane",
				"float32": 0.01
			},

			{
				"name": "far plane",
				"float32": 1000
			},

			{
				"name": "aspect ratio",
				"float32": 1
			},

			{
				"name": "field of view",
				"float32": 80
			},

			{
				"name": "projection type",
				"enum": 0
			}
		]
	}
}
```

### Usage

``` shell
scene-generator
```

The utility will output JSON files into a folder in the current directory named `new_scene`.

``` shell
scene-generator scene_1
```

The utility will output JSON files into a folder in the current directory named `scene_1`.

``` shell
scene-generator --help
```

Example usage for the utility is shown.

## Scene Exporter

### Description

This utility exports JSON scene files to binary scene files that can be used in the engine at run time. The order of the systems in the *internal* and *external* lists for both *update* and *draw* define the order that the systems are called at run time. *Internal* systems are called after *external* systems. The *update* systems run at a constant frame rate, while the *draw* systems run at a variable frame rate. The limits on the number of instances of a given component in the scene are pairs of component names and integers. The active camera is either the UUID of an entity with the *camera* component or an empty string if there is no active camera in the scene. The constant defined for gravity is used as the acceleration due to gravity in the scene.

### Example

#### scene_file.json:

``` json
{
	"systems":
	{
		"update":
		{
			"external":
			[
				"basic_input",
				"player_movement",
				"enemy_movement"
			],

			"internal":
			[
				"clean_hit_information",
				"clean_hit_list",
				"animation",
				"clean_global_transforms",
				"apply_parent_transforms",
				"simulate_rigid_bodies",
				"joint_information",
				"particle_simulator",
				"gui",
				"audio"
			]
		},

		"draw":
		{
			"external":
			[

			],

			"internal":
			[
				"cubemap_renderer",
				"render_heightmap",
				"renderer",
				"wireframe_renderer",
				"debug_renderer",
				"collision_primitive_renderer",
				"particle_renderer",
				"gui_renderer"
			]
		}
	},

	"component_limits":
	{
		"animation": 2048,
		"animator": 2048,
		"audio_source": 1024,
		"ball_socket_joint": 256,
		"ball_socket2_joint": 256,
		"box": 1024,
		"button": 1024,
		"camera": 8,
		"capsule": 1024,
		"collision_tree_node": 4096,
		"collision": 1024,
		"cubemap": 1,
		"debug_collision_primitive": 2048,
		"debug_line": 2048,
		"debug_point": 4096,
		"debug_primitive": 4096,
		"debug_transform": 1024,
		"font": 1024,
		"gui_transform": 2048,
		"heightmap": 256,
		"hinge_joint": 256,
		"hit_information": 4096,
		"hit_list": 8192,
		"image": 1024,
		"joint_information": 256,
		"joint_list": 512,
		"joint": 8192,
		"model": 2048,
		"next_animation": 2048,
		"panel": 512,
		"particle_emitter": 128,
		"progress_bar": 1024,
		"rigid_body": 1024,
		"slider_joint": 256,
		"slider": 1024,
		"sphere": 1024,
		"surface_information": 512,
		"text_field": 1024,
		"text": 1024,
		"transform": 16384,
		"widget": 1024,
		"wireframe": 2048,
		"enemy": 128,
		"player": 1
	},

	"active_camera": "xTd99Vp<Q/;s=Q(94^F85Oi8$k7&Zh_h(4^9bjQEdk(3h6DLQQ^'9lw*N--/rbt",
	"gravity": -9.81
}
```

### Usage

``` shell
scene-exporter scene_file
```

or

``` shell
scene-exporter scene_file.json
```

The utility will output to a binary file in the current directory named `scene_file.scene`.

``` shell
scene-exporter --help
```

Example usage for the utility is shown.