local system = {}

local input = engine.input
local keyboard = engine.keyboard

system.components = {}
system.components[1] = "player"
system.components[2] = "transform"

function system.init(scene)
  input:register("vertical", input.AXIS(keyboard.S, keyboard.W))
  input:register("verticalarrows", input.AXIS(keyboard.DOWN, keyboard.UP))
  input:register("close", input.BUTTON(keyboard.ESCAPE))
end

function system.begin(scene)
  if input.close.keydown then
	engine.C.closeWindow()
  end
end

local direction
local value
local transform
local player

function system.run(scene, uuid, dt)
  transform = scene:getComponent("transform", uuid)
  player = scene:getComponent("player", uuid)

  direction = 0
  if input.vertical.value ~= 0 then
	direction = input.vertical.value
  elseif input.verticalarrows.value ~= 0 then
	direction = input.verticalarrows.value
  end

  if direction ~= 0 then
	value = direction * dt * player.speed

	if transform.position.y + value > player.worldHeight / 2 then
	  transform.position.y = player.worldHeight / 2
	elseif transform.position.y + value < -player.worldHeight / 2 then
	  transform.position.y = -player.worldHeight / 2
	else
	  transform.position.y = transform.position.y + value
	end
	transform:markDirty(scene)
  end
end

return system
