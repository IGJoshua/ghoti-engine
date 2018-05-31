local engine = {};

local ffi = require("ffi")

if ffi.os == "Windows" then
  local game = ffi.load("build/monochrome.dll")
else
  local game = ffi.load("build/monochrome.so")
end

ffi.cdef[[]]

return engine
