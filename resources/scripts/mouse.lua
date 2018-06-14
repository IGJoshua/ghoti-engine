local mouse = {}

mouse.x = 0
mouse.y = 0

mouse.buttons = {}

for i = 1,10 do
  mouse.buttons[i] = {}
end

mouse.scroll = {}
mouse.scroll.x = 0
mouse.scroll.y = 0

mouse.scroll.xaxis = {}
mouse.scroll.xaxis.__axisname = "x"
mouse.scroll.xaxis.__container = mouse.scroll
mouse.scroll.yaxis = {}
mouse.scroll.yaxis.__axisname = "y"
mouse.scroll.yaxis.__container = mouse.scroll

setmetatable(mouse.scroll.xaxis, engine.input.axismt)
setmetatable(mouse.scroll.yaxis, engine.input.axismt)

return mouse
