local system = {}

local C = engine.C

system.components = {}
system.components[1] = "game_manager"

function system.run(scene, uuid, dt)
  local game_manager = scene:getComponent("game_manager", uuid)

  local num_balls = 0
  local to_remove = {}

  -- Check to see if the ball has gone out of bounds
  for ball, uuid in scene:getComponentIterator("ball") do
	local ballTransform = scene:getComponent("transform", uuid)
	if ballTransform.position.x < ball.bounds.x then
	  -- Ball is off left side of screen
	  game_manager.opponentScore = game_manager.opponentScore + 1
	  to_remove[#to_remove + 1] = uuid
	elseif ballTransform.position.x > ball.bounds.z then
	  -- Ball is off right side of screen
	  game_manager.playerScore = game_manager.playerScore + 1
	  to_remove[#to_remove + 1] = uuid
	end
  end

  for i, uuid in ipairs(to_remove) do
	io.write(string.format("%s\n", ffi.string(uuid.string)))
	-- C.sceneRemoveEntity(scene.ptr, uuid)
  end

  -- Update the scoreboards for the two players
  local playerText = scene:getComponent("text", game_manager.playerScoreText)
  local opponentText = scene:getComponent("text", game_manager.opponentScoreText)
  playerText.text = tostring(game_manager.playerScore)
  opponentText.text = tostring(game_manager.opponentScore)

  -- Spawn a ball if needed
end

return system
