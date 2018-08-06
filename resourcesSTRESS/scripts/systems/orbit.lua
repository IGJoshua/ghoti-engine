io.write("Loading the Orbit system\n")

local system = {}

system.init = nil
system.shutdown = nil

system.components = {}
system.components[1] = "orbit"
system.components[2] = "transform"

function system.run(scene, uuid, dt)
  local transform = scene:getComponent("transform", uuid)
  local orbit = scene:getComponent("orbit", uuid)

  orbit.time = orbit.time + dt

  transform.position.x = math.sin(orbit.time * orbit.speed) * orbit.radius + orbit.origin.x
  transform.position.y = math.cos(orbit.time * orbit.speed) * orbit.radius + orbit.origin.y
  transform.position.z = orbit.origin.z

  transform:markDirty(scene, uuid)
end

io.write("Finished loading the Orbit system\n")

return system
