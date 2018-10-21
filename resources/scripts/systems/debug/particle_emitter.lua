local system = {}

local C = engine.C

function system.init(scene)
  local particle_emitter_id = C.idFromName("j.joFaf+aZa#)wToedKPoGVFw'+uWQfAS')7/_nm9fl.AYp#*?oNr<Dv<7Lbr-y")
  local particle_emitter = scene:getComponent("particle_emitter", particle_emitter_id)
  particle_emitter:emit(particle_emitter_id, false, "pokemon", 28 * 18 - 11, 18, 28, false)
end

return system