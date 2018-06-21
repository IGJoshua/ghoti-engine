io.write("Loading the Oscillator system\n")

local system = {}

local C = engine.C
local kazmath = engine.kazmath

system.init = nil
system.shutdown = nil

system.components = {}
system.components[1] = "oscillator"
system.components[2] = "transform"

local pos
local transform
local oscillator

function system.run(scene, uuid, dt)
  transform = scene:getComponent("transform", uuid)
  oscillator = scene:getComponent("oscillator", uuid)

  oscillator.time = oscillator.time + dt

  pos = ffi.new("kmVec3[1]")

  kazmath.kmVec3Scale(pos,
					  oscillator.direction,
					  oscillator.distance
						* math.sin(oscillator.time * oscillator.speed))

  kazmath.kmVec3Add(transform.position,
					pos,
					oscillator.position)

  transform:markDirty(scene)
end

io.write("Finished loading the Oscillator system\n")

return system
