io.write("Loading the Oscillator system\n")

local system = {}

local C = engine.C
local kazmath = engine.kazmath

function system.init(scene)
  local oscillator = ffi.new("OscillatorComponent", {0, 0, 0}, {0.707, 0.707, 0}, 0, 1, 2)
  C.sceneAddComponentToEntity(scene.ptr, C.idFromName("test"), C.idFromName("oscillator"), oscillator)
end

system.shutdown = nil

system.components = {}
system.components[1] = "oscillator"
system.components[2] = "transform"

function system.run(scene, uuid, dt)
  local transform = scene:getComponent("transform", uuid)
  local oscillator = scene:getComponent("oscillator", uuid)

  oscillator.time = oscillator.time + dt

  local pos = ffi.new("kmVec3[1]")

  kazmath.kmVec3Scale(pos,
					  oscillator.direction,
					  oscillator.distance
						* math.sin(oscillator.time))

  kazmath.kmVec3Add(transform.position,
					pos,
					oscillator.position)
end

io.write("Finished loading the Oscillator system\n")

return system
