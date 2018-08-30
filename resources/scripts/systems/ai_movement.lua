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

  -- Acquire new target
  -- Search for balls heading in our direction
  local possibleTargets = {}
  for ball, uuid in scene:getComponentIterator("ball") do
	if ball.velocity.x > 0 then
	  possibleTargets[#possibleTargets + 1] = uuid
	end
  end

  -- Track the closest ball in our direction
  local closestBall = 1000
  ballTransform = nil
  for i, uuid in ipairs(possibleTargets) do
	local trans = scene:getComponent("transform", uuid)
	if ffi.cast("uint64", trans) ~= 0 then
	  local dist = math.abs(trans.position.x - transform.position.x)
	  if dist < closestBall then
		closestBall = dist
		ballTransform = trans
	  end
	end
  end

  -- if there is no ball in our direction, then track the one furthest
  -- away from us, which is most likely to bounce towards us next
  if not ballTransform then
	local furthestBall = 0
	for ball, uuid in scene:getComponentIterator("ball") do
	  local trans = scene:getComponent("transform", uuid)
	  if ffi.cast("uint64", trans) ~= 0 then
		local dist = math.abs(trans.position.x - transform.position.x)
		if dist > furthestBall then
		  furthestBall = dist
		  ballTransform = trans
		end
	  end
	end
  end

  if ballTransform then
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
end

return system
