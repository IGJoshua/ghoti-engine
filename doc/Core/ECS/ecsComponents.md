### [Table of Contents](../../main.md) -> [Core Engine](../coreIndex.md) -> [Entity Component System](ecsMain.md) -> List of Components

# List of Component Types

[//]: <> (<style>               )
[//]: <> (    th#t01 {          )
[//]: <> (        width: 15%;   )
[//]: <> (    }                 )
[//]: <> (    th#t02 {          )
[//]: <> (        width: 55%;   )
[//]: <> (    }                 )
[//]: <> (    th#t03 {          )
[//]: <> (        width: 30%;   )
[//]: <> (    }                 )
[//]: <> (</style>              )


<table>
    <tr>
        <th id="t01">Component
        <th id="t02">Description
        <th id="t03">Members
    </tr>
    <tr>
        <td>transform
        <td>Contains the position, rotation, and scale of the entity
        <td>
            <ul>
                <li>kmVec3 position
                <li>kmQuaternion rotation
                <li>kmVec3 scale
            </ul>
    </tr>
    <tr>
        <td>model
        <td>Contains a file path to the model and a boolean for whether or not the model is actually visible
        <td>
            <ul>
                <li>char name[1025]
                <li>bool visible
            </ul>
    </tr>
    <tr>
        <td>camera
        <td>
            <p>Contains all necessary members for a functional camera. Currently supported Camera Projection Types are:
            <ul>
                <li>CAMERA_PROJECTION_TYPE_PERSPECTIVE
                <li>CAMERA_PROJECTION_TYPE_ORTHOGRAPHIC
            </ul>
            </p>
        <td>
            <ul>
                <li>float nearPlane
                <li>float farPlane
                <li>float aspectRatio
                <li>float fov
                <li>CameraProjectionType projectionType
            </ul>
    </tr>
    <tr>
        <td>orbit
        <td>Manipulates the transform component to spin around an origin
        <td>
            <ul>
                <li>kmVec3 origin
                <li>float speed
                <li>float radius
                <li>float time
            </ul>
    </tr>
    <tr>
        <td>oscillator
        <td>Manipulates the transform component to move the entity based on a sine wave in a given direction
        <td>
            <ul>
                <li>kmVec3 position
                <li>kmVec3 direction
                <li>float time
                <li>float speed
                <li>float distance
            </ul>
    </tr>

</table>
