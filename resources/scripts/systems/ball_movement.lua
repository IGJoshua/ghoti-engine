local system = {}

local C = engine.C
local kazmath = engine.kazmath

system.components = {}
system.components[1] = "ball"
system.components[2] = "transform"

local cdtItr = ffi.new("ComponentDataTableIterator")
local cdtItrOut = ffi.new("ComponentDataTableIterator[1]")

local ballComponentID = ffi.new("UUID[1]", C.idFromName("ball"))
local paddleComponentID = ffi.new("UUID[1]", C.idFromName("paddle"))
local ball

function system.init(scene)
  cdtItr = C.cdtGetIterator(
	ffi.cast(
	  "ComponentDataTable **",
	  C.hashMapGetData(
		scene.ptr.componentTypes,
		ballComponentID))[0])
  while C.cdtIteratorAtEnd(cdtItr) == 0 do
	ball = ffi.cast("BallComponent *", C.cdtIteratorGetData(cdtItr))
	ball.velocity.x = math.random(-200, 200) / 10.0
	ball.velocity.y = math.random(-200, 200) / 10.0

	if ball.velocity.x == 0 then
	  ball.velocity.x = 10
	end

	if math.abs(ball.velocity.x) < 5 then
	  ball.velocity.x = ball.velocity.x * 10
	end

	cdtItrOut[0] = cdtItr
	C.cdtMoveIterator(cdtItrOut)
	cdtItr = cdtItrOut[0]
  end
end

local transform
local paddle
local paddleTransform
local paddleUUID

function system.run(scene, uuid, dt)
  transform = scene:getComponent("transform", uuid)
  ball = scene:getComponent("ball", uuid)

  if ball.delay > 0 then
	ball.delay = ball.delay - dt
	return
  end

  -- Check if out of bounds

  if (transform.position.x < ball.bounds.x
		and ball.velocity.x < 0)
	or (transform.position.x > ball.bounds.z
		and ball.velocity.x > 0) then
	  -- Out of bounds
	  ball.delay = 1

	  transform.position.x = 0
	  transform.position.y = 0
	  transform:markDirty(scene)

	  ball.velocity.x = math.random(-200, 200) / 10.0
	  ball.velocity.y = math.random(-200, 200) / 10.0

	  if ball.velocity.x == 0 then
		ball.velocity.x = 10
	  end

	  if math.abs(ball.velocity.x) < 5 then
		ball.velocity.x = ball.velocity.x * 10
	  end

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
  cdtItr = C.cdtGetIterator(
	ffi.cast(
	  "ComponentDataTable **",
	  C.hashMapGetData(
		scene.ptr.componentTypes,
		paddleComponentID))[0])
  while C.cdtIteratorAtEnd(cdtItr) == 0 do
	paddleUUID = C.cdtIteratorGetUUID(cdtItr)
	paddle = scene:getComponent("paddle", paddleUUID)
	paddleTransform = scene:getComponent("transform", paddleUUID)

	-- If it hits the paddle, then go to the other side
	if (transform.position.x < paddleTransform.position.x + paddle.bounds.x)
	  and (transform.position.x > paddleTransform.position.x - paddle.bounds.x)
	  and (transform.position.y < paddleTransform.position.y + paddle.bounds.y)
	  and (transform.position.y > paddleTransform.position.y - paddle.bounds.y) then
	  -- The paddle has hit the ball
	  ball.velocity.x = -ball.velocity.x + math.random(-300, 300) / 100.0
	  ball.velocity.y = ball.velocity.y + math.random(-300, 300) / 100.0
	end

	cdtItrOut[0] = cdtItr
	C.cdtMoveIterator(cdtItrOut)
	cdtItr = cdtItrOut[0]
  end

  transform.position.x = transform.position.x + ball.velocity.x * dt
  transform.position.y = transform.position.y + ball.velocity.y * dt

  transform:markDirty(scene)
end

return system
