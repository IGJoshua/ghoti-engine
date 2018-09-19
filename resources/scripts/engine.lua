ffi = require("ffi")

engine = {}

io.write("Loaded cFFI\n")

engine.C = ffi.load(
  ffi.os == "Windows"
    and "./ghoti.dll"
    or "./ghoti.so")

local C = engine.C

io.write("Loaded ghoti library\n")

engine.file = ffi.load(
  ffi.os == "Windows"
    and "./file-utilities.dll"
	or "./lib/libfile-utilities.so")

io.write("Loaded file utilities library\n")

engine.json = ffi.load(
  ffi.os == "Windows"
    and "./json-utilities.dll"
	or "./lib/libjson-utilities.so")

io.write("Loaded json utilities library\n")

engine.kazmath = ffi.load(
  ffi.os == "Windows"
    and "./kazmath.dll"
    or "./lib/libkazmath.so")

io.write("Loaded kazmath library\n")

require("resources/scripts/cdefs")

io.write("Created cffi definitions\n")

engine.input = require("resources/scripts/input")

engine.keyboard = require("resources/scripts/keyboard")
engine.mouse = require("resources/scripts/mouse")
engine.gamepad = require("resources/scripts/gamepad")

local Scene = require("resources/scripts/scene")

io.write("Required scene\n")

engine.components = require("resources/scripts/components")

-- iterate over every file in resources/scripts/components/ and require them
local componentFiles = io.popen(
  ffi.os == "Windows"
	and 'dir resources\\scripts\\components /b /a-d'
	or 'find resources/scripts/components -name "*.lua"')
for line in componentFiles:lines() do
  if ffi.os == "Windows" then
	line = 'resources/scripts/components/'..line
  end
  io.write("Loading component "..string.sub(line, 0, -5).."\n")
  local componentName = string.sub(line, 0, -5)
  require(componentName)
end

require("resources/scripts/componentUtils")

engine.scenes = {}
engine.systems = {}

function engine.initScene(pScene)
  local scene = Scene:new(pScene)

  local itr
  local physics = true

  for _=1,2 do
    if physics then
      itr = C.listGetIterator(scene.ptr.luaPhysicsFrameSystemNames)
      physics = false
    else
      itr = C.listGetIterator(scene.ptr.luaRenderFrameSystemNames)
    end

    while C.listIteratorAtEnd(itr) == 0 do
	  local systemName =
		ffi.string(
		  ffi.cast("UUID *", itr.curr.data).string)
	  package.loaded["resources/scripts/systems/"..systemName] = nil
	  local system = require("resources/scripts/systems/"..systemName)
      engine.systems[systemName] = system

      if not system then
        error(string.format(
				"Unable to load system %s, panic",
				ffi.string(ffi.cast("UUID *", itr.curr.data).string)))
      end

      if system.init then
        local err, message = pcall(system.init, scene)
        if err == false then
          io.write(string.format(
                     "Error while initializing a system\n%s\n",
                     message))
        end
      end

      local itrRef = ffi.new("ListIterator[1]", itr)
      C.listMoveIterator(itrRef)
      itr = itrRef[0]
    end
  end

  engine.scenes[pScene] = scene
end

function engine.runSystems(pScene, dt, physics)
  local scene = engine.scenes[pScene]

  local null = ffi.new("int64", 0)
  local emptyUUID = ffi.new("UUID")

  local itr
  local itrRef = ffi.new("ListIterator[1]")

  if physics then
    itr = C.listGetIterator(scene.ptr.luaPhysicsFrameSystemNames)
  else
    itr = C.listGetIterator(scene.ptr.luaRenderFrameSystemNames)
  end

  while C.listIteratorAtEnd(itr) == 0 do
	local systemName = ffi.string(ffi.cast("UUID *", itr.curr.data).string)
    local system = engine.systems[systemName]

    if not system then
      error(string.format("Unable to load system %s, panic", systemName))
    end

    if system.begin then
      local err, message = pcall(system.begin, scene, dt)
      if err == false then
        io.write(string.format("Error while beginning a system\n%s\n", message))
      end
    end

    if system.run then
	  for component, uuid in scene:getComponentIterator(system.components[1]) do
		local valid = true

		for k = 2,#system.components do
		  if scene:getComponent(system.components[k], uuid) == nil then
			valid = false
		  end
		end

		if valid then
		  local err, message = pcall(
			system.run,
			scene,
			uuid,
			dt)
		  if err == false then
			io.write(string.format(
					   "Error raised while running physics system\n%s\n",
					   message))
		  end
		end
	  end

      if system.clean then
        local err, message = pcall(system.clean, scene, dt)
        if err == false then
          io.write(string.format(
                     "Error raised while calling the clean system\n%s\n",
                     message))
        end
      end
    end

	itrRef[0] = itr
    C.listMoveIterator(itrRef)
    itr = itrRef[0]
  end
end

function engine.runPhysicsSystems(pScene, dt)
  engine.runSystems(pScene, dt, true)
end

function engine.runRenderSystems(pScene, dt)
  engine.runSystems(pScene, dt, nil)
end

function engine.shutdownScene(pScene)
  local scene = engine.scenes[pScene]
  engine.scenes[pScene] = nil

  local itr = C.listGetIterator(scene.ptr.luaPhysicsFrameSystemNames)
  while C.listIteratorAtEnd(itr) == 0 do
    local physicsSystem = engine.systems[
      ffi.string(ffi.cast("UUID *", itr.curr.data).string)]
    if physicsSystem.shutdown then
      local err, message = pcall(physicsSystem.shutdown, scene)
      if err == false then
        io.write(string.format("Error raised during physics shutdown\n%s\n",
                               message))
      end
    end

    local itrRef = ffi.new("ListIterator[1]", itr)
    C.listMoveIterator(itrRef)
    itr = itrRef[0]
  end

  itr = C.listGetIterator(scene.ptr.luaRenderFrameSystemNames)
  while C.listIteratorAtEnd(itr) == 0 do
    local renderSystem = engine.systems[
      ffi.string(ffi.cast("UUID *", itr.curr.data).string)]

    if renderSystem.shutdown then
      local err, message = pcall(renderSystem.shutdown, scene)
      if err == false then
        io.write(string.format("Error raised during render shutdown\n%s\n",
                               message))
      end
    end

    local itrRef = ffi.new("ListIterator[1]", itr)
    C.listMoveIterator(itrRef)
    itr = itrRef[0]
  end
end

function engine.cleanInput()
  for key, value in pairs(engine.keyboard) do
    if type(value) == "table" then
      value.updated = false
    end
  end
  for key, value in pairs(engine.mouse.buttons) do
    if type(value) == "table" then
      value.updated = false
    end
  end

  engine.mouse.scroll.x = 0
  engine.mouse.scroll.y = 0

  for name, button in pairs(engine.gamepad.buttons) do
    button.updated = false
  end

  for name, dpad in pairs(engine.gamepad.dpad) do
    dpad.updated = false
  end
end

io.write("Running init script\n")

require("resources/scripts/init")

io.write("Finished init script\n")
