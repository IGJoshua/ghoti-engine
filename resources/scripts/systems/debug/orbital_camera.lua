local system = {}

local C = engine.C
local kazmath = engine.kazmath
local input = engine.input
local mouse = engine.mouse

local previousMouse = ffi.new("kmVec2[1]")
local pitch = 0.0
local yaw = 0.0

local mouseSensitivity = -10.0
local distanceFromOrigin = 6

local cameraPosition = ffi.new("kmVec3[1]")
local up = ffi.new("kmVec3[1]")

local cameraTransform = nil
local cubeTransform = nil

function system.init(scene)
  input:register("click", input.BUTTON(mouse.buttons[1]))

  kazmath.kmVec3Fill(cameraPosition, 0.0, 0.0, distanceFromOrigin)
  kazmath.kmVec3Fill(up, 0.0, 1.0, 0.0)

  cameraTransform = scene:getComponent("transform", scene.ptr.mainCamera)
  cubeTransform = scene:getComponent("transform", C.idFromName("cube"))
end

function system.begin(scene, dt)
  local deltaMouse = ffi.new("kmVec2[1]")
  deltaMouse[0].x = mouse.x - previousMouse[0].x
  deltaMouse[0].y = mouse.y - previousMouse[0].y

  previousMouse[0].x = mouse.x
  previousMouse[0].y = mouse.y

  if input.click.keydown then
    yaw = yaw + kazmath.kmDegreesToRadians(deltaMouse[0].x) * mouseSensitivity * dt
    pitch = pitch + kazmath.kmDegreesToRadians(deltaMouse[0].y) *  mouseSensitivity * dt

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
  kazmath.kmVec3Add(cameraTransform.position, cubeTransform.globalPosition, cameraTransform.position)

  cameraTransform:markDirty(scene)
end

return system