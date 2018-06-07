local system = {}

local keyboard = engine.keyboard

function system.begin(scene, dt)
  if keyboard.A.updated then
	io.write("Key event on A!\n")
	if keyboard.A.keydown then
	  io.write("Keydown!\n")
	else
	  io.write("Keyup!\n")
	end
  end
end

return system
