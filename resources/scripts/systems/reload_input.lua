local system = {}

local input = engine.input
local keyboard = engine.keyboard
local C = engine.C

function system.init(scene)
  input:register("reload", input.BUTTON(keyboard.R))
end

function system.begin(scene, dt)
  if input.reload.updated and input.reload.keydown then
	C.reloadAllScenes()
  end
end

return system
