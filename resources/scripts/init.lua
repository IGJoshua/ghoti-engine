-- NOTE(Joshua): This is where you load the main scene

math.randomseed(os.time())

local C = engine.C

C.loadScene("audio_test")

C.setListenerScene("audio_test")
