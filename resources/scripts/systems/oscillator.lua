io.write("Loading the Oscillator system\n")

local system = {}

local C = engine.C
local kazmath = engine.kazmath

system.init = nil
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

  transform:markDirty(scene)
end

io.write("Finished loading the Oscillator system\n")

return system
