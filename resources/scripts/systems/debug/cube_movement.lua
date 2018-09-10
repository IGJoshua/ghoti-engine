local system = {}

local C = engine.C
local input = engine.input
local keyboard = engine.keyboard

local cubeTransform = nil
local speed = 10.0

function system.init(scene)
  input:register("forward", input.BUTTON(keyboard.W))
  input:register("left", input.BUTTON(keyboard.A))
  input:register("backward", input.BUTTON(keyboard.S))
  input:register("right", input.BUTTON(keyboard.D))

  cubeTransform = scene:getComponent("transform", C.idFromName("cube"))
end

function system.begin(scene, dt)
  local displacement = speed * dt

  if input.forward.keydown then
	cubeTransform.position.z = cubeTransform.position.z - displacement
  end

  if input.left.keydown then
	cubeTransform.position.x = cubeTransform.position.x - displacement
  end

  if input.backward.keydown then
	cubeTransform.position.z = cubeTransform.position.z + displacement
  end

  if input.right.keydown then
	cubeTransform.position.x = cubeTransform.position.x + displacement
  end

  cubeTransform:markDirty(scene)
end

return system