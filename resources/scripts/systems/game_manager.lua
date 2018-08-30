local system = {}

local C = engine.C
local kazmath = engine.kazmath

system.components = {}
system.components[1] = "game_manager"

function system.run(scene, uuid, dt)
  local game_manager = scene:getComponent("game_manager", uuid)

  local num_balls = 0
  local to_remove = {}

  -- Check to see if the ball has gone out of bounds
  for ball, uuid in scene:getComponentIterator("ball") do
	local ballTransform = scene:getComponent("transform", uuid)
	if ffi.cast("uint64", ballTransform) ~= 0 then
	  num_balls = num_balls + 1

	  if ballTransform.position.x < ball.bounds.x then
		-- Ball is off left side of screen
		game_manager.opponentScore = game_manager.opponentScore + 1
		to_remove[#to_remove + 1] = uuid
		num_balls = num_balls - 1

		local opponentPaddle = scene:getComponent("opponent", game_manager.opponentPaddle)
		opponentPaddle.speed = opponentPaddle.speed - 1
	  elseif ballTransform.position.x > ball.bounds.z then
		-- Ball is off right side of screen
		game_manager.playerScore = game_manager.playerScore + 1
		to_remove[#to_remove + 1] = uuid
		num_balls = num_balls - 1

		local opponentPaddle = scene:getComponent("opponent", game_manager.opponentPaddle)
		opponentPaddle.speed = opponentPaddle.speed + 2
	  end
	end
  end

  for i, uuid in ipairs(to_remove) do
	C.sceneRemoveEntity(scene.ptr, uuid)
  end

  -- Update the scoreboards for the two players
  local playerText = scene:getComponent("text", game_manager.playerScoreText)
  local opponentText = scene:getComponent("text", game_manager.opponentScoreText)
  playerText.text = tostring(game_manager.playerScore)
  opponentText.text = tostring(game_manager.opponentScore)

  -- End the game if needed
  if game_manager.playerScore > 10 then
	-- Player has won
  elseif game_manager.opponentScore > 10 then
	-- Player has lost
  end

  -- Spawn a ball if needed
  if num_balls < game_manager.numBalls then
	-- Spawn a ball
	local ball = C.sceneCreateEntity(scene.ptr)

	local transform = ffi.new("TransformComponent")
	kazmath.kmVec3Fill(transform.position, 0, 0, 0)
	kazmath.kmVec3Fill(transform.scale, 1, 1, 1)
	kazmath.kmQuaternionIdentity(transform.rotation)
	kazmath.kmVec3Fill(transform.globalPosition, 0, 0, 0)
	kazmath.kmVec3Fill(transform.globalScale, 1, 1, 1)
	kazmath.kmQuaternionIdentity(transform.globalRotation)
	kazmath.kmVec3Fill(transform.lastGlobalPosition, 0, 0, 0)
	kazmath.kmVec3Fill(transform.lastGlobalScale, 1, 1, 1)
	kazmath.kmQuaternionIdentity(transform.lastGlobalRotation)
	transform.parent = C.idFromName("")
	transform.firstChild = C.idFromName("")
	transform.nextSibling = C.idFromName("")
	transform.dirty = true
	scene:addComponentToEntity("transform", ball, transform)

	local ballComponent = ffi.new("BallComponent")
	ballComponent.delay = 2
	ballComponent.velocity.x = math.random(-200, 200) / 10.0
	ballComponent.velocity.y = math.random(-200, 200) / 10.0
	kazmath.kmVec4Fill(ballComponent.bounds, -25, -17, 25, 17)
	scene:addComponentToEntity("ball", ball, ballComponent)

	local model = ffi.new("ModelComponent")
	model.name = "woodsphere"
	model.visible = true
	scene:addComponentToEntity("model", ball, model)
  end
end

return system
