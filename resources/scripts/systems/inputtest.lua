local system = {}

local input = engine.input
local keyboard = engine.keyboard
local mouse = engine.mouse
local gamepad = engine.gamepad
local C = engine.C

function system.init(scene)
  input:register("close", input.BUTTON(keyboard.ESCAPE, gamepad.buttons.guide))
  input:register("mouseclick", input.BUTTON(mouse.buttons[1]))
  input:register("attack", input.BUTTON(nil, gamepad.buttons.x))
  input:register("scroll", input.AXIS(nil, nil, mouse.scroll.yaxis))
  input:register("trigger", input.AXIS(nil, nil, gamepad.lefttrigger))
  input:register("horizontal", input.AXIS(keyboard.A, keyboard.D, gamepad.leftstick.xaxis))
  input:register("horizontallook", input.AXIS(keyboard.LEFT, keyboard.RIGHT, gamepad.rightstick.xaxis))
  input:register("reload", input.BUTTON(keyboard.R))
  input:register("load_cool_thing", input.BUTTON(keyboard.L))
  input:register("unload_cool_thing", input.BUTTON(keyboard.U))
  input:register("switch_to_cool_thing", input.BUTTON(keyboard.C))
  input:register("save", input.BUTTON(keyboard.S))
end

function system.begin(scene, dt)
  if input.close.keydown then
	engine.C.closeWindow()
  end

  if input.mouseclick.updated and input.mouseclick.keydown then
	io.write("Mouse click!\n")
  end

  if input.scroll.value ~= 0 then
	io.write(string.format("Mouse scrolled! %s\n", input.scroll.value))
  end

  if input.trigger.value ~= 0 then
	io.write(string.format("Left trigger value: %f\n", input.trigger.value))
  end

  if input.horizontal.value ~= 0 then
	io.write(string.format("Left stick x value: %f\n", input.horizontal.value))
  end

  if input.horizontallook.value ~= 0 then
	io.write(string.format("Right stick x value: %f\n", input.horizontallook.value))
  end

  if input.attack.updated and input.attack.keydown then
	io.write("Pressed X\n")
  end

  if input.reload.updated and input.reload.keydown then
	C.reloadAllScenes()
  end

  if input.load_cool_thing.updated and input.load_cool_thing.keydown then
	C.loadScene("cool_thing")
  end

  if input.unload_cool_thing.updated and input.unload_cool_thing.keydown then
	C.unloadScene("cool_thing")
  end

  if input.switch_to_cool_thing.updated and input.switch_to_cool_thing.keydown then
	C.loadScene("cool_thing")
	C.unloadScene("cool_thing")
  end

  if input.save.updated and input.save.keydown then
	C.exportSave(nil, 0, 2)
  end
end

return system
