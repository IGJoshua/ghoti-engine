local system = {}

local C = engine.C

system.components = {}
system.components[1] = "button"

local button

function system.run(scene, uuid, dt)
  button = scene:getComponent("button", uuid)

  if button.pressed then
	C.loadScene("pong_example")
	C.unloadScene("pong_example_main_menu")
  end
end

return system
