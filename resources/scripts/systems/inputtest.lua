local system = {}

local keyboard = engine.keyboard
local mouse = engine.mouse

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

end

return system
