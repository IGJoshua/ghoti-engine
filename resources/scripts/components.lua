local components = {}

function components:register(n, t)
  local prototype = {}
  self[n] = prototype

  t = string.format("%s *", t)

  function prototype:new(cdata)
	local component = {}
	local mt = {}
	setmetatable(component, mt)
	mt.__index = ffi.cast(t, cdata)
	return component
  end

  return prototype
end

return components
