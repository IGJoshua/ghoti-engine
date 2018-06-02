local system = {}

system.init = nil
system.shutdown = nil

system.components = {}
system.components[1] = "orbit"
system.components[2] = "transform"
system.numComponents = 2

local time = {}

function system.run(scene, uuid, dt)
  io.write(string.format("Running orbit on entity %s\n", ffi.string(uuid.string)))
--[[
  if time[uuid] == nil then
	time[uuid] = 0
  end

  time[uuid] = time[uuid] + dt

  local transform = scene:getComponent("transform", uuid)
  local orbit = scene:getComponent("orbit", uuid)

  transform.x = math.sin(time[uuid] * orbit.speed) * orbit.radius + orbit.origin.x
  transform.y = math.cos(time[uuid] * orbit.speed) * orbit.radius + orbit.origin.y
  transform.z = orbit.origin.z
--]]
end

return system
