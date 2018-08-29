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

local value
local transform
local player

function system.run(scene, uuid, dt)
  transform = scene:getComponent("transform", uuid)
  player = scene:getComponent("player", uuid)

  value = 0
  if input.vertical.value ~= 0 then
	value = input.vertical.value
  elseif input.verticalarrows.value ~= 0 then
	value = input.verticalarrows.value
  end

  if value ~= 0 then
	transform.position.y = transform.position.y + value * dt * player.speed
	transform:markDirty(scene)
  end
end

return system
