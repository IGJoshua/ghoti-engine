local system = {}

local input = engine.input
local keyboard = engine.keyboard
local C = engine.C

local gold_rigid_body
local brick_rigid_body

local gold_force_magnitude = 1000
local brick_force_magnitude = 1000

function system.init(scene)
  input:register("gold_forward_force", input.BUTTON(keyboard.W))
  input:register("gold_backward_force", input.BUTTON(keyboard.S))
  input:register("gold_left_force", input.BUTTON(keyboard.A))
  input:register("gold_right_force", input.BUTTON(keyboard.D))
  input:register("gold_up_force", input.BUTTON(keyboard.E))
  input:register("gold_down_force", input.BUTTON(keyboard.Q))

  input:register("brick_forward_force", input.BUTTON(keyboard.UP))
  input:register("brick_backward_force", input.BUTTON(keyboard.DOWN))
  input:register("brick_left_force", input.BUTTON(keyboard.LEFT))
  input:register("brick_right_force", input.BUTTON(keyboard.RIGHT))
  input:register("brick_up_force", input.BUTTON(keyboard.ENTER))
  input:register("brick_down_force", input.BUTTON(keyboard.RIGHT_SHIFT))

  gold_rigid_body = scene:getComponent("rigid_body", C.idFromName("wD3^l'h*e8`+&I/;l(Q3Z8_KsE2q2FD/_TcQPIQ3^7;YU?qGEAXtN5F?WU7_xXk"))
  brick_rigid_body = scene:getComponent("rigid_body", C.idFromName("r#FJ_H.ti1w@lUvx>J3Nw7dD`tclVSHWng4<r/#)$@rV<hjI#'?0h`mBX7juk_b"))
end

function system.begin(scene, dt)
  if input.gold_forward_force.updated and input.gold_forward_force.keydown then
	local force = ffi.new("kmVec3")
	force.z = -gold_force_magnitude
	gold_rigid_body:addForce(force)
  end

  if input.gold_backward_force.updated and input.gold_backward_force.keydown then
	local force = ffi.new("kmVec3")
	force.z = gold_force_magnitude
	gold_rigid_body:addForce(force)
  end

  if input.gold_left_force.updated and input.gold_left_force.keydown then
	local force = ffi.new("kmVec3")
	force.x = -gold_force_magnitude
	gold_rigid_body:addForce(force)
  end

  if input.gold_right_force.updated and input.gold_right_force.keydown then
	local force = ffi.new("kmVec3")
	force.x = gold_force_magnitude
	gold_rigid_body:addForce(force)
  end

  if input.gold_up_force.updated and input.gold_up_force.keydown then
	local force = ffi.new("kmVec3")
	force.y = gold_force_magnitude
	gold_rigid_body:addForce(force)
  end

  if input.gold_down_force.updated and input.gold_down_force.keydown then
	local force = ffi.new("kmVec3")
	force.y = -gold_force_magnitude
	gold_rigid_body:addForce(force)
  end

  if input.brick_forward_force.updated and input.brick_forward_force.keydown then
	local force = ffi.new("kmVec3")
	force.z = -brick_force_magnitude
	brick_rigid_body:addForce(force)
  end

  if input.brick_backward_force.updated and input.brick_backward_force.keydown then
	local force = ffi.new("kmVec3")
	force.z = brick_force_magnitude
	brick_rigid_body:addForce(force)
  end

  if input.brick_left_force.updated and input.brick_left_force.keydown then
	local force = ffi.new("kmVec3")
	force.x = -brick_force_magnitude
	brick_rigid_body:addForce(force)
  end

  if input.brick_right_force.updated and input.brick_right_force.keydown then
	local force = ffi.new("kmVec3")
	force.x = brick_force_magnitude
	brick_rigid_body:addForce(force)
  end

  if input.brick_up_force.updated and input.brick_up_force.keydown then
	local force = ffi.new("kmVec3")
	force.y = brick_force_magnitude
	brick_rigid_body:addForce(force)
  end

  if input.brick_down_force.updated and input.brick_down_force.keydown then
	local force = ffi.new("kmVec3")
	force.y = -brick_force_magnitude
	brick_rigid_body:addForce(force)
  end
end

return system
