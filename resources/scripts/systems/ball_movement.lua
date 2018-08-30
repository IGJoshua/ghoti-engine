local system = {}

local C = engine.C
local kazmath = engine.kazmath

system.components = {}
system.components[1] = "ball"
system.components[2] = "transform"

local ballComponentID = ffi.new("UUID[1]", C.idFromName("ball"))
local paddleComponentID = ffi.new("UUID[1]", C.idFromName("paddle"))

local transform
local paddle
local paddleTransform
local paddleUUID
local ball

function system.run(scene, uuid, dt)
  ball = scene:getComponent("ball", uuid)
  transform = scene:getComponent("transform", uuid)

  if ball.delay > 0 then
	ball.delay = ball.delay - dt
	return
  end

  -- Ensure velocity is high enough
  if math.abs(ball.velocity.x) < 5 then
	ball.velocity.x = 5 * (math.random(1, 2) * 2 - 4)
  end
  local length = kazmath.kmVec2Length(ball.velocity)
  if length < 10 then
	if length < 0.01 then
	  ball.velocity.x = 1
	end
	kazmath.kmVec2Normalize(ball.velocity, ball.velocity)
	kazmath.kmVec2Scale(ball.velocity, ball.velocity, 10)
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
