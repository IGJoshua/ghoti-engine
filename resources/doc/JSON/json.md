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

| Type    | Size        | Range                                                           | Description          |
|:-------:|:-----------:|:---------------------------------------------------------------:|:--------------------:|
| uint8   | 1 byte      | 0 to 255                                                        | non-negative integer |
| uint16  | 2 bytes     | 0 to 65,535                                                     | non-negative integer |
| uint32  | 4 bytes     | 0 to 4,294,967,295                                              | non-negative integer |
| uint64  | 8 bytes     | 0 to 18,446,744,073,709,551,615                                 | non-negative integer |
| int8    | 1 byte      | -128 to 127                                                     | integer              |
| int16   | 2 bytes     | -32,768 to 32,767                                               | integer              |
| int32   | 4 bytes     | -2,147,483,648 to 2,147,483,647                                 | integer              |
| int64   | 8 bytes     | -9,223,372,036,854,775,808 to 9,223,372,036,854,775,807         | integer              |
| float32 | 4 bytes     | -3.4 $\times$ 10<sup>38</sup> to 3.4 $\times$ 10<sup>38</sup>   | real number          |
| float64 | 8 bytes     | -1.7 $\times$ 10<sup>308</sup> to 1.7 $\times$ 10<sup>308</sup> | real number          |
| bool    | 4 bytes     | true or false                                                   | boolean              |
| char    | 1 byte      | 1 ascii character                                               | character            |
| char(n) | n bytes     | maximum of n - 1 ascii characters                               | string               |
| uuid    | 64 bytes    | valid entity uuids                                              | uuid                 |

### Example

#### entity_file.json:

``` json
{
	"uuid": "-p]h3S).cZh(k-IH;8UbrOgo|K;;{I9):upLHwY-SA3>L[cfp8Jeg1VcZn!W79_",

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
			}
		],

		"model":
		[
			{
				"name": "filename",
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
				"uuid": "n_$fB#g:8Giiao#)F+/){[0ForcMRnqCOranMJ+cIL)b=_i;huA@-NczB#$rosm2"
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

This utility exports JSON asset files to binary asset files that can be used in the engine at run time. Each subset contains material data for the individual meshes in the model in the form of components that contain information pertaining to the given type of component.

### Example

#### asset_file.json:

``` json
{
	"player":
	{
		"diffuse":
		{
			"texture_name": "player_diffuse",
			"texture_uv_map": 0,
			"value": [0.1, 0.58, 0.93]
		},

		"normal":
		{
			"texture_name": "player_normal",
			"texture_uv_map": 1
		},

		"ambient":
		{
			"value": [0.75, 0.5, 0.2]
		}
	},

	"weapon":
	{
		"diffuse":
		{
			"texture_name": "weapon_diffuse",
			"texture_uv_map": 0,
			"value": [0.76, 0.33, 0.2]
		},

		"specular":
		{
			"texture_name": "weapon_specular",
			"texture_uv_map": 2,
			"value": [0.84, 0.89, 0.67],
			"power": 32.0
		},

		"emissive":
		{
			"texture_name": "weapon_emissive",
			"texture_uv_map": 1,
			"value": [0.25, 0.13, 0.41]
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

This utility generates a blank JSON scene file that can be used as a template for new scenes. The blank file contains the default *internal* systems and empty lists for the *external* systems that are called during *update* and *draw*. The file also contains default limits on the number of instances for each of the components used in the default systems and a placeholder for the active camera in the scene.

### Example

#### new_scene.json:

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

This utility exports JSON scene files to binary scene files that can be used in the engine at run time. The order of the systems in the *internal* and *external* lists for both *update* and *draw* define the order that the systems are called at run time. *Internal* systems are called after *external* systems. The *update* systems run at a constant frame rate, while the *draw* systems run at a variable frame rate. The limits on the number of instances of a given component in the scene are pairs of component names and integers. The active camera is either the UUID of an entity with the *camera* component or the string "none" if there is no active camera in the scene.

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
				"player_movement",
				"enemy_movement"
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
		"camera": 16,
		"player": 1,
		"enemy": 128
	},

	"active_camera": "m_]/a`m`-IR45mU6}Us7F5SEd?y<5?C\"}}.^`z>jCo}XUk*`rOsEqYamrv,8u)5"
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
