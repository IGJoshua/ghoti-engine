local system = {}

local input = engine.input
local keyboard = engine.keyboard
local mouse = engine.mouse
local gamepad = engine.gamepad
local C = engine.C

function system.init(scene)
  input:register("close", input.BUTTON(keyboard.ESCAPE, gamepad.buttons.guide))
  input:register("reload", input.BUTTON(keyboard.R))
  input:register("save", input.BUTTON(keyboard.S))
  input:register("load_save", input.BUTTON(keyboard.Z))

  input:register("play", input.BUTTON(keyboard.H))
  input:register("queue", input.BUTTON(keyboard.Q))
  input:register("isActive", input.BUTTON(keyboard.W))
  input:register("pause", input.BUTTON(keyboard.J))
  input:register("resume", input.BUTTON(keyboard.K))
  input:register("stop", input.BUTTON(keyboard.L))

end

function system.begin(scene, dt)
  if input.close.keydown then
    engine.C.closeWindow()
  end

  if input.reload.updated and input.reload.keydown then
    C.reloadAllScenes()
  end

  if input.save.updated and input.save.keydown then
    C.exportSave(nil, 0, 1)
  end

  if input.load_save.updated and input.load_save.keydown then
    C.loadSave(1, nil)
  end

--[[

  if input.play.updated and input.play.keydown then
    C.playSoundAtSource(scene.ptr, "BMG/BattlePrep", scene.ptr.mainCamera)
  end

  if input.queue.updated and input.queue.keydown then
    C.queueSoundAtSource(scene.ptr, "BMG/Raid", scene.ptr.mainCamera)
  end

  if input.isActive.updated and input.isActive.keydown then
    local active
    active = C.isSourceActive(scene.ptr, scene.ptr.mainCamera)
    print(active)
  end

  if input.pause.updated and input.pause.keydown then
    C.pauseSoundAtSource(scene.ptr, scene.ptr.mainCamera)
  end

  if input.resume.updated and input.resume.keydown then
    C.resumeSoundAtSource(scene.ptr, scene.ptr.mainCamera)
  end

  if input.stop.updated and input.stop.keydown then
    C.stopSoundAtSource(scene.ptr, scene.ptr.mainCamera)
  end
end

--]]

return system
