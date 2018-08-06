local keyboard = {}

function keyboard.key(key)
  return tonumber(ffi.cast("int32", ffi.new("GLFW_KEY", "GLFW_KEY_"..key)))
end

local mt = {}
setmetatable(keyboard, mt)

function mt.__index(t, key)
  if type(key) == "string" then
    key = "GLFW_KEY_"..key
    local valid, enum = pcall(ffi.new, "GLFW_KEY", key)
    if valid then
      local index = tonumber(ffi.cast("int32", enum))
      local val = rawget(t, index)
      if val then
        return val
      else
        local ret = {}

        ret.updated = false
        ret.keydown = false

        t[index] = ret

        return ret
      end
    else
      error("Indexing into keyboard failed, key was not valid")
    end
  elseif type(key) == "number" then
    return rawget(t, key)
  else
    error("Indexing into keyboard failed")
  end
end

function mt.__newindex(t, key, value)
  if type(key) == "number" then
    rawset(t, key, value)
  else
    error("Trying to change a value in keyboard table")
  end
end

return keyboard
