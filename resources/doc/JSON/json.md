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
				"char(1024)": "player"
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

This utility exports JSON asset files to binary asset files that can be used in the engine at run time. Each mesh is associated with a material and its values as well as a material mask and an opacity mask. There is also physics information about the model's center of mass as well as the boxes, spheres, and capsules associated with the model.

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
				"uv_map": 3,
				"double-sided": false,

				"values":
				{
					"base_color": [1.0, 1.0, 1.0],
					"emissive": [1.0, 1.0, 1.0],
					"metallic": 1.0,
					"normal": 1.0,
					"roughness": 1.0
				}
			},

			"masks":
			{
				"uv_map": 1,

				"material":
				{
					"collection":
					{
						"name": "moss",
						"double-sided": false,

						"values":
						{
							"base_color": [1.0, 1.0, 1.0],
							"emissive": [1.0, 1.0, 1.0],
							"metallic": 1.0,
							"normal": 1.0,
							"roughness": 1.0
						}
					},

					"grunge":
					{
						"name": "grass",
						"double-sided": false,

						"values":
						{
							"base_color": [1.0, 1.0, 1.0],
							"emissive": [1.0, 1.0, 1.0],
							"metallic": 1.0,
							"normal": 1.0,
							"roughness": 1.0
						}
					},

					"wear":
					{
						"name": "metal",
						"double-sided": false,

						"values":
						{
							"base_color": [1.0, 1.0, 1.0],
							"emissive": [1.0, 1.0, 1.0],
							"metallic": 1.0,
							"normal": 1.0,
							"roughness": 1.0
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
				"uv_map": 5,
				"double-sided": true,

				"values":
				{
					"base_color": [1.0, 1.0, 1.0],
					"emissive": [1.0, 1.0, 1.0],
					"metallic": 1.0,
					"normal": 1.0,
					"roughness": 1.0
				}
			},

			"masks":
			{
				"uv_map": 2,

				"material":
				{
					"collection":
					{
						"name": "moss",
						"double-sided": true,

						"values":
						{
							"base_color": [1.0, 1.0, 1.0],
							"emissive": [1.0, 1.0, 1.0],
							"metallic": 1.0,
							"normal": 1.0,
							"roughness": 1.0
						}
					},

					"grunge":
					{
						"name": "grass",
						"double-sided": true,

						"values":
						{
							"base_color": [1.0, 1.0, 1.0],
							"emissive": [1.0, 1.0, 1.0],
							"metallic": 1.0,
							"normal": 1.0,
							"roughness": 1.0
						}
					},

					"wear":
					{
						"name": "",
						"double-sided": true,

						"values":
						{
							"base_color": [1.0, 1.0, 1.0],
							"emissive": [1.0, 1.0, 1.0],
							"metallic": 1.0,
							"normal": 1.0,
							"roughness": 1.0
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
					"position": [0.0, 0.0, 0.0],
					"rotation": [0.0, 0.0, 0.0, 1.0],
					"scale": [2.3, 5.0, 5.0]
				},

				"wall_2":
				{
					"position": [2.0, -3.0, 0.0],
					"rotation": [0.0, 0.0, 0.0, 1.0],
					"scale": [1.0, 7.0, 3.0]
				}
			},

			"spheres":
			{
				"ground":
				{
					"position": [-2.7, 0.0, 5.0],
					"scale": 2.0
				}
			},

			"capsules":
			{
				"player":
				{
					"position": [0.0, 0.0, 2.0],
					"rotation": [0.707, 0.0, 0.0, 0.707],
					"scale": [3.0, 4.0, 3.0]
				}
			}
		}
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

The utility will output to a binary file in the current directory named `asset_file.asset`.

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
				"clean_global_transforms",
				"apply_parent_transforms",
				"simulate_rigid_bodies"
			]
		},

		"draw":
		{
			"external":
			[

			],

			"internal": ["renderer"]
		}
	},

	"component_limits":
	{
		"transform": 1024,
		"model": 2048,
		"camera": 16,
		"collision": 1024,
		"box": 1024,
		"sphere": 1024,
		"capsule": 1024,
		"hit_information": 4096,
		"hit_list": 8192,
		"rigid_body": 1024,
		"collision_tree_node": 4096,
		"surface_information": 4096
	},

	"active_camera": "Hf&<4`;5?4ojByWAYlld(kqg4($3BP/Va8<EHNYY;Ip3v7ahyKfjCSs@T$_5Rka",
	"gravity": -9.81
}
```

#### new_scene/entities/camera.json:

``` json
{
	"uuid": "Hf&<4`;5?4ojByWAYlld(kqg4($3BP/Va8<EHNYY;Ip3v7ahyKfjCSs@T$_5Rka",

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
				"int32": 0
			}
		]
	}
}
```

### Usage

``` shell
scene-generator
```

The utility will output to a JSON file in the current directory named `new_scene.json`.

``` shell
scene-generator scene_1
```

or

``` shell
scene-generator scene_1.json
```

The utility will output to a JSON file in the current directory named `scene_1.json`.

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
				"clean_global_transforms",
				"apply_parent_transforms",
				"simulate_rigid_bodies"
			]
		},

		"draw":
		{
			"external":
			[

			],

			"internal": ["renderer"]
		}
	},

	"component_limits":
	{
		"transform": 1024,
		"model": 2048,
		"camera": 16,
		"collision": 1024,
		"box": 1024,
		"sphere": 1024,
		"capsule": 1024,
		"hit_information": 4096,
		"hit_list": 8192,
		"rigid_body": 1024,
		"collision_tree_node": 4096,
		"surface_information": 4096,
		"player": 1,
		"enemy": 128
	},

	"active_camera": "Hf&<4`;5?4ojByWAYlld(kqg4($3BP/Va8<EHNYY;Ip3v7ahyKfjCSs@T$_5Rka",
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