local system = {}

local input = engine.input
local keyboard = engine.keyboard

system.components = {}
system.components[1] = "opponent"
system.components[2] = "transform"

local transform
local opponent
local ballTransform
local value
local direction

function system.run(scene, uuid, dt)
  transform = scene:getComponent("transform", uuid)
  opponent = scene:getComponent("opponent", uuid)
  ballTransform = scene:getComponent("transform", opponent.target)

  if ballTransform.position.y > transform.position.y then
	direction = 1
  elseif ballTransform.position.y < transform.position.y then
	direction = -1
  else
	direction = 0
  end

  value = opponent.speed * direction * dt

  if (transform.position.y < ballTransform.position.y
		and transform.position.y + value > ballTransform.position.y)
	or (transform.position.y > ballTransform.position.y
		and transform.position.y + value < ballTransform.position.y)then
	transform.position.y = ballTransform.position.y
  else
	transform.position.y = transform.position.y + value
  end
  transform:markDirty(scene)
end

return system
