local system = {}

local input = engine.input
local keyboard = engine.keyboard
local mouse = engine.mouse
local gamepad = engine.gamepad
local C = engine.C

function system.init(scene)
  input:register("close", input.BUTTON(keyboard.ESCAPE, gamepad.buttons.guide))
  input:register("reload", input.BUTTON(keyboard.R))
  input:register("fullscreen", input.BUTTON(keyboard.F))
  input:register("save", input.BUTTON(keyboard.S))
  input:register("load_save", input.BUTTON(keyboard.L))
end

function system.begin(scene, dt)
  if input.close.keydown then
    engine.C.closeWindow()
  end

  if input.reload.updated and input.reload.keydown then
	C.reloadAllScenes()
  end

  if input.fullscreen.updated and input.fullscreen.keydown then
	C.switchFullscreenMode()
  end

  if input.save.updated and input.save.keydown then
    C.exportSave(nil, 0, 1)
  end

  if input.load_save.updated and input.load_save.keydown then
    C.loadSave(1, nil)
  end
end

return system
