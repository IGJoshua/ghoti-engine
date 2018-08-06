local input = {}

input.__registry = {}

function input:register(n, t)
  self.__registry[n] = t
end

function input.AXIS(neg, pos, gp)
  local ret = {}
  local mt = {}
  setmetatable(ret, mt)

  function mt.__index(t, k)
    if k == "value" then
      if gp and gp.value ~= 0 then
        return gp.value
      elseif neg and pos and neg.keydown and pos.keydown then
        return 0
      elseif pos and pos.keydown then
        return 1
      elseif neg and neg.keydown then
        return -1
      else
        return 0
      end
    else
      error(string.format(
              "Indexing into axis with non-value index %s\n",
              k))
    end
  end

  return ret
end

function input.BUTTON(key, gp)
  local ret = {}
  local mt = {}
  setmetatable(ret, mt)

  function mt.__index(t, k)
    if k == "updated" or k == "keydown" then
      if key and gp then
        return key[k] or gp[k]
      elseif gp then
        return gp[k]
      elseif key then
        return key[k]
      else
        local ret = {}
        ret.updated = false
        ret.keydown = false
        return ret
      end
    else
      error(string.format(
              "Indexing into button with non-conforming value: %s\n",
              k))
    end
  end

  return ret
end

input.axismt = {}
function input.axismt.__index(t, k)
  if k == "value" then
    return rawget(rawget(t, "__container"), rawget(t, "__axisname"))
  end
end


local inputmt = {}
setmetatable(input, inputmt)

function inputmt.__index(t, k)
  local raw = rawget(t, k)
  if raw then
    return raw
  else
    return t.__registry[k]
  end
end

return input
