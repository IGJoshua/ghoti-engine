-- NOTE(Joshua): This is where you load the main scene

math.randomseed(os.time())

local C = engine.C

C.loadScene("reloader")
C.loadScene("pong_example_main_menu")
