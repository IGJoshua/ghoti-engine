local system = {}

local C = engine.C
local kazmath = engine.kazmath

system.run = nil
system.shutdown = nil

-- function system.init(scene)
--   io.write("Main scene init\n")

--   local model = ffi.new("ModelComponent")
--   model.visible = true
--   local orbit = ffi.new("OrbitComponent")
--   local transform = ffi.new("TransformComponent")
--   local oscillator = ffi.new("OscillatorComponent", {0, 0, 0}, {0.707, 0.707, 0}, 0, 1, 2)
--   local cameraComp = ffi.new("CameraComponent")

--   local teapot = C.sceneCreateEntity(scene.ptr)
--   io.write(string.format("Teapot ID: %s\n", ffi.string(teapot.string)))

--   ffi.copy(model.name, ffi.new("char[32]", "teapot"), 32)
--   C.sceneAddComponentToEntity(scene.ptr, teapot, C.idFromName("model"), model)
--   kazmath.kmVec3Zero(orbit.origin)
--   orbit.speed = 3
--   orbit.radius = 2
--   C.sceneAddComponentToEntity(scene.ptr, teapot, C.idFromName("orbit"), orbit)
--   kazmath.kmVec3Zero(transform.position)
--   kazmath.kmQuaternionRotationPitchYawRoll(transform.rotation, kazmath.kmDegreesToRadians(90), 0, 0)
--   transform.scale.x = 0.01
--   transform.scale.y = 0.01
--   transform.scale.z = 0.01
--   C.sceneAddComponentToEntity(scene.ptr, teapot, C.idFromName("transform"), transform)

--   local test = C.sceneCreateEntity(scene.ptr)
--   io.write(string.format("Test ID: %s\n", ffi.string(test.string)))

--   ffi.copy(model.name, ffi.new("char[32]", "test"), 32)
--   C.sceneAddComponentToEntity(scene.ptr, test, C.idFromName("model"), model)
--   C.sceneAddComponentToEntity(scene.ptr, test, C.idFromName("oscillator"), oscillator)
--   transform.scale.x = 1
--   transform.scale.y = 1
--   transform.scale.z = 1
--   C.sceneAddComponentToEntity(scene.ptr, test, C.idFromName("transform"), transform)

--   local camera = C.sceneCreateEntity(scene.ptr)
--   io.write(string.format("Camera ID: %s\n", ffi.string(camera.string)))

--   cameraComp.aspectRatio = 4.0 / 3.0
--   cameraComp.fov = 80
--   cameraComp.nearPlane = 0.1
--   cameraComp.farPlane = 1000
--   cameraComp.projectionType = ffi.new("CameraProjectionType", 0)
--   scene.ptr.mainCamera = camera
--   C.sceneAddComponentToEntity(scene.ptr, camera, C.idFromName("camera"), cameraComp)
--   kazmath.kmVec3Fill(transform.position, 0, 0, 2)
--   kazmath.kmQuaternionIdentity(transform.rotation)
--   C.sceneAddComponentToEntity(scene.ptr, camera, C.idFromName("transform"), transform)
-- end

return system
