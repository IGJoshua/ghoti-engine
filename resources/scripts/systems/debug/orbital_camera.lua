local system = {}

local C = engine.C
local kazmath = engine.kazmath
local input = engine.input
local mouse = engine.mouse

local previousMouse = ffi.new("kmVec2")
local pitch = 0.0
local yaw = 0.0

local mouseSensitivity = -20.0
local distanceFromOrigin = 6

local cameraPosition = ffi.new("kmVec3[1]")
local target = ffi.new("kmVec3[1]")
local up = ffi.new("kmVec3[1]")

local cameraTransform = nil
local cubeTransform = nil

function system.init(scene)
  input:register("click", input.BUTTON(mouse.buttons[1]))

  kazmath.kmVec3Fill(cameraPosition, 0.0, 0.0, distanceFromOrigin)
  kazmath.kmVec3Zero(target)
  kazmath.kmVec3Fill(up, 0.0, 1.0, 0.0)

  cameraTransform = scene:getComponent("transform", scene.ptr.mainCamera)
end

function system.begin(scene, dt)
  local deltaMouse = ffi.new("kmVec2")
  deltaMouse.x = mouse.x - previousMouse.x
  deltaMouse.y = mouse.y - previousMouse.y

  previousMouse.x = mouse.x
  previousMouse.y = mouse.y

  if input.click.keydown then
    yaw = yaw + kazmath.kmDegreesToRadians(deltaMouse.x) * mouseSensitivity * dt
    pitch = pitch + kazmath.kmDegreesToRadians(deltaMouse.y) *  mouseSensitivity * dt

    if yaw < 0.0 then
      yaw = 2 * 3.14
    elseif yaw > 2 * 3.14 then
      yaw = 0.0
    end

    if pitch < -0.49 * 3.14 then
     pitch = -0.49 * 3.14
    elseif pitch > 0.49 * 3.14 then
      pitch = 0.49 * 3.14
    end

    kazmath.kmQuaternionRotationPitchYawRoll(cameraTransform.rotation, pitch, yaw, 0.0)
  end

  kazmath.kmQuaternionMultiplyVec3(cameraTransform.position, cameraTransform.rotation, cameraPosition)
  kazmath.kmVec3Add(cameraTransform.position, target, cameraTransform.position)

  cameraTransform:markDirty(scene)
end

return system