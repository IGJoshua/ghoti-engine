local system = {}

local C = engine.C
local kazmath = engine.kazmath

system.components = {}
system.components[1] = "ball"
system.components[2] = "transform"

local ballComponentID = ffi.new("UUID[1]", C.idFromName("ball"))
local paddleComponentID = ffi.new("UUID[1]", C.idFromName("paddle"))

function system.init(scene)
  for ball in scene:getComponentIterator("ball") do
	ball.velocity.x = math.random(-200, 200) / 10.0
	ball.velocity.y = math.random(-200, 200) / 10.0

	if ball.velocity.x == 0 then
	  ball.velocity.x = 10
	end

	if math.abs(ball.velocity.x) < 5 then
	  ball.velocity.x = ball.velocity.x * 10
	end
  end
end

local transform
local paddle
local paddleTransform
local paddleUUID
local ball

function system.run(scene, uuid, dt)
  transform = scene:getComponent("transform", uuid)
  ball = scene:getComponent("ball", uuid)

  if ball.delay > 0 then
	ball.delay = ball.delay - dt
	return
  end

  -- Check if hit top or bottom

  if (transform.position.y < ball.bounds.y
		and ball.velocity.y < 0)
	or (transform.position.y > ball.bounds.w
		and ball.velocity.y > 0) then
	  ball.velocity.y = -ball.velocity.y
	  ball.velocity.x = ball.velocity.x + math.random(-200, 200) / 100.0
  end

  -- Check if it hit a paddle
  for paddle, puuid in scene:getComponentIterator("paddle") do
	paddleTransform = scene:getComponent("transform", puuid)

	-- If it hits the paddle, then reverse your x velocity
	if (transform.position.x < paddleTransform.position.x + paddle.bounds.x)
	  and (transform.position.x > paddleTransform.position.x - paddle.bounds.x)
	  and (transform.position.y < paddleTransform.position.y + paddle.bounds.y)
	and (transform.position.y > paddleTransform.position.y - paddle.bounds.y) then
	  transform.position.x =
		paddleTransform.position.x
		+ -(ball.velocity.x / math.abs(ball.velocity.x))
		* paddle.bounds.x * 1.1

	  -- The paddle has hit the ball
	  ball.velocity.x = -ball.velocity.x + math.random(-300, 300) / 100.0
	  ball.velocity.y = ball.velocity.y + math.random(-300, 300) / 100.0
	end
  end

  transform.position.x = transform.position.x + ball.velocity.x * dt
  transform.position.y = transform.position.y + ball.velocity.y * dt

  transform:markDirty(scene)
end

return system
