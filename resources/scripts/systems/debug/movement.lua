local system = {}

local input = engine.input
local kazmath = engine.kazmath
local keyboard = engine.keyboard
local C = engine.C

system.components = {}
system.components[1] = "movement"
system.components[2] = "collision"
system.components[3] = "rigid_body"
system.components[4] = "transform"

function system.init(scene)
  input:register("close", input.BUTTON(keyboard.ESCAPE))
  input:register("horizontal", input.AXIS(keyboard.A, keyboard.D))
  input:register("vertical", input.AXIS(keyboard.W, keyboard.S))
  input:register("jump", input.BUTTON(keyboard.SPACE))
end

function system.begin(scene, dt)
  if input.close.keydown then
	C.closeWindow()
  end
end

local outVec = ffi.new("kmVec3[1]")

local movement
local transform
local rigid_body
local collision

function system.run(scene, uuid, dt)
  movement = scene:getComponent("movement", uuid)
  transform = scene:getComponent("transform", uuid)
  rigid_body = scene:getComponent("rigid_body", uuid)
  collision = scene:getComponent("collision", uuid)

  if kazmath.kmVec3LengthSq(rigid_body.velocity) <= movement.maxSpeed then
	kazmath.kmVec3Fill(outVec, input.horizontal.value, 0, input.vertical.value)
	kazmath.kmVec3Normalize(outVec, outVec[0])
	kazmath.kmVec3Scale(outVec, outVec[0], movement.speed * dt / rigid_body.mass)
	kazmath.kmVec3Add(outVec, outVec[0], rigid_body.velocity)
	kazmath.kmVec3Assign(rigid_body.velocity, outVec[0])
  end

  if ffi.string(collision.hitList.string) ~= "" then
	if input.jump.keydown and input.jump.updated then
	  kazmath.kmVec3Fill(outVec, 0, movement.jumpHeight, 0)
	  kazmath.kmVec3Scale(outVec, outVec[0], 1 / rigid_body.mass)
	  kazmath.kmVec3Add(outVec, outVec[0], rigid_body.velocity)
	  kazmath.kmVec3Assign(rigid_body.velocity, outVec[0])
	end
  end
end

return system
