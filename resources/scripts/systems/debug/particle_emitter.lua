local system = {}

local C = engine.C

function system.init(scene)
  local particle_emitter = scene:getComponent("particle_emitter", C.idFromName("j.joFaf+aZa#)wToedKPoGVFw'+uWQfAS')7/_nm9fl.AYp#*?oNr<Dv<7Lbr-y"))
  particle_emitter:emit(false, "pokemon", 28 * 18 - 11, 18, 28, false)
end

return system