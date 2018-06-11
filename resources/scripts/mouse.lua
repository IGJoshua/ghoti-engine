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

return mouse
