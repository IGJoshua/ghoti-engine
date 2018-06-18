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
  input:register("scene", input.BUTTON(keyboard.ENTER))
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

  if input.scene.keydown and input.scene.updated then
    C.changeScene = 1

    local oldScene = ffi.cast("Scene **", C.activeScenes.front.data)
    C.listPushFront(C.unloadedScenes, oldScene);
    C.listPopFront(C.activeScenes)

    local newScene = ffi.new("Scene *", ffi.cast("Scene *", 0))
    local newSceneOut = ffi.new("Scene *[1]", newScene)
    C.luaLoadScene("scene_1", newSceneOut)
    C.listPushFront(C.activeScenes, newSceneOut)
    newScene = newSceneOut[0]
  end
end

return system
