-- NOTE(Joshua): This is where you load the main scene

local C = engine.C

local sceneLoaded = "scene_1"

local outScene = ffi.new("Scene *[1]")
C.luaLoadScene(sceneLoaded, outScene)
local scene = outScene[0]

C.listPushFront(C.activeScenes, outScene)
