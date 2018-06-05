local system = {}

system.init = nil
system.shutdown = nil

system.components = {}
system.components[1] = "orbit"
system.components[2] = "transform"

local time = {}

function system.run(scene, uuid, dt)
  local name = ffi.string(uuid.string)
  if time[name] == nil then
	time[name] = 0
  end

  time[name] = time[name] + dt

  local transform = scene:getComponent("transform", uuid)
  local orbit = scene:getComponent("orbit", uuid)

  transform.position.x = math.sin(time[name] * orbit.speed) * orbit.radius + orbit.origin.x
  transform.position.y = math.cos(time[name] * orbit.speed) * orbit.radius + orbit.origin.y
  transform.position.z = orbit.origin.z
end

return system
