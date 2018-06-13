local system = {}

local keyboard = engine.keyboard
local mouse = engine.mouse
local gamepad = engine.gamepad

function system.begin(scene, dt)
  if keyboard.A.updated then
	io.write("Key event on A!\n")
	if keyboard.A.keydown then
	  io.write("Keydown!\n")
	else
	  io.write("Keyup!\n")
	end
  end

  if keyboard.ESCAPE.keydown then
	engine.C.closeWindow()
  end

  if mouse.buttons[1].updated and mouse.buttons[1].keydown then
	io.write("Mouse click!\n")
  end

  if mouse.scroll.y ~= 0 then
	io.write(string.format("Mouse scrolled! %d\n", mouse.scroll.y))
  end

  if gamepad.lefttrigger.value ~= 0 then
	io.write(string.format("Left trigger value: %f\n", gamepad.lefttrigger.value))
  end

  if gamepad.leftstick.x ~= 0 then
	io.write(string.format("Left stick x value: %f\n", gamepad.leftstick.x))
  end

  if gamepad.rightstick.x ~= 0 then
	io.write(string.format("Right stick x value: %f\n", gamepad.rightstick.x))
  end

  if gamepad.buttons.guide.keydown then
	engine.C.closeWindow()
  end
end

return system
