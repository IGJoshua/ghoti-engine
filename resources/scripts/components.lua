local components = {}

function components:register(n, t)
  local prototype = {}
  prototype.__index = prototype
  prototype.numEntries = 256
  prototype.type = t
  self[n] = prototype

  ffi.metatype(t, prototype)

  t = string.format("%s *", t)

  function prototype:new(cdata)
	return ffi.cast(t, cdata)
  end

  return prototype
end

return components
