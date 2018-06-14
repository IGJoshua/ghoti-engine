local gamepad = {}

gamepad.buttons = {}
gamepad.buttons.x = {}
gamepad.buttons.y = {}
gamepad.buttons.a = {}
gamepad.buttons.b = {}
gamepad.buttons.leftbumper = {}
gamepad.buttons.rightbumper = {}
gamepad.buttons.leftstick = {}
gamepad.buttons.rightstick = {}
gamepad.buttons.start = {}
gamepad.buttons.back = {}
gamepad.buttons.guide = {}

for name, button in pairs(gamepad.buttons) do
  button.updated = false
  button.keydown = false
end

gamepad.dpad = {}
gamepad.dpad.left = {}
gamepad.dpad.right = {}
gamepad.dpad.up = {}
gamepad.dpad.down = {}

for name, dpad in pairs(gamepad.dpad) do
  dpad.updated = false
  dpad.keydown = false
end

gamepad.leftstick = {}
gamepad.leftstick.deadzone = {}
gamepad.leftstick.deadzone.value = 0.25
gamepad.leftstick.deadzone.type = "square"
gamepad.leftstick.x = 0
gamepad.leftstick.y = 0
gamepad.leftstick.rawx = 0
gamepad.leftstick.rawy = 0

gamepad.rightstick = {}
gamepad.rightstick.deadzone = {}
gamepad.rightstick.deadzone.value = 0.25
gamepad.rightstick.deadzone.type = "circular"
gamepad.rightstick.x = 0
gamepad.rightstick.y = 0
gamepad.rightstick.rawx = 0
gamepad.rightstick.rawy = 0

gamepad.leftstick.xaxis = {}
gamepad.leftstick.xaxis.__axisname = "x"
gamepad.leftstick.xaxis.__container = gamepad.leftstick
gamepad.leftstick.yaxis = {}
gamepad.leftstick.yaxis.__axisname = "y"
gamepad.leftstick.yaxis.__container = gamepad.leftstick

gamepad.rightstick.xaxis = {}
gamepad.rightstick.xaxis.__axisname = "x"
gamepad.rightstick.xaxis.__container = gamepad.rightstick
gamepad.rightstick.yaxis = {}
gamepad.rightstick.yaxis.__axisname = "y"
gamepad.rightstick.yaxis.__container = gamepad.rightstick

setmetatable(gamepad.leftstick.xaxis, engine.input.axismt)
setmetatable(gamepad.leftstick.yaxis, engine.input.axismt)
setmetatable(gamepad.rightstick.xaxis, engine.input.axismt)
setmetatable(gamepad.rightstick.yaxis, engine.input.axismt)

gamepad.lefttrigger = {}
gamepad.lefttrigger.deadzone = 0.1
gamepad.lefttrigger.value = 0

gamepad.righttrigger = {}
gamepad.righttrigger.deadzone = 0.1
gamepad.righttrigger.value = 0

return gamepad
