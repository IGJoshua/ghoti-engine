local input = {}

function input:register(n, t)
  self[n] = t
end

function input.axis(neg, pos, gamepad)
  local axis = {}

  axis.value = 0
  axis.neg = neg
  axis.pos = pos

  return axis
end

return input
